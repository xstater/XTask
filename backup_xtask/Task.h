#ifndef _XTASK_TASK_H_
#define _XTASK_TASK_H_

#include "Continuation.h"

namespace xtask{
    template <class Type>
    struct TaskBase{
        TaskBase()
                :then_policy(Policy::pool),
                 then(nullptr),
                 task(nullptr),
                 data(),
                 exception(nullptr),
                 error(nullptr),
                 status(Status::waiting),
                 policy(Policy::pool){}
         ~TaskBase() = default;

        Policy then_policy;
        std::function<void(std::exception_ptr)> then;
        std::function<Type()> task;
        Type data;
        std::exception_ptr exception;
        std::function<void(std::exception_ptr)> error;
        Status status;
        Policy policy;
    };

    template <>
    struct TaskBase<void>{
        TaskBase()
                :then_policy(Policy::pool),
                 then(nullptr),
                 task(nullptr),
                 exception(nullptr),
                 error(nullptr),
                 status(Status::waiting),
                 policy(Policy::pool){}
        ~TaskBase() = default;

        Policy then_policy;
        std::function<void(std::exception_ptr)> then;
        std::function<void()> task;
        std::exception_ptr exception;
        std::function<void(std::exception_ptr)> error;
        Status status;
        Policy policy;
    };

    template <class Type>
    class Task{
    public:
        using value_type = Type;

        template <class TaskFunc>
        Task(TaskFunc &&task_func,bool auto_run = true,Policy policy = Policy::pool)
                :m_ptr(std::make_shared<TaskBase<Type>>()){
            m_ptr->task = std::forward<TaskFunc>(task_func);
            m_ptr->policy = policy;
            if(auto_run)run();
         }
        template <class TaskFunc,class ErrFunc>
        Task(TaskFunc &&task_func,ErrFunc &&err_func,bool auto_run = true,Policy policy = Policy::pool)
                :m_ptr(std::make_shared<TaskBase<Type>>()){
            m_ptr->task = std::forward<TaskFunc>(task_func);
            m_ptr->error = std::forward<ErrFunc>(err_func);
            m_ptr->policy = policy;
            if(auto_run)run();
        }
        Task(std::shared_ptr<TaskBase<void>> ptr)
                :m_ptr(std::move(ptr)){}
        Task(const Task &) = default;
        Task(Task &&) = default;
        ~Task() = default;

        Task &operator=(std::shared_ptr<TaskBase<void>> ptr){
            m_ptr = std::move(ptr);
            return *this;
        }
        Task &operator=(const Task &) = default;
        Task &operator=(Task &&) = default;

        void run()noexcept{
            switch(m_ptr->policy){
                case Policy::pool:
                    ThreadPool::instance().addTask(std::bind(&Task::m_run,this));
                    break;
                case Policy::synchronized:
                    m_run();
                    break;
                case Policy::thread:
                    std::thread t(std::bind(&Task::m_run,this));
                    t.detach();
                    break;
            }
        }

        Type get(){
            wait();
            if(m_ptr->exception){
                std::rethrow_exception(m_ptr->exception);
            }
            return std::move(m_ptr->data);
        }

        void wait()const noexcept{
            while(m_ptr->status != Status::done){}
        }

        template<class Rep,class Period>
        Status wait_for(const std::chrono::duration<Rep,Period> &timeout_duration)const{
            auto start = std::chrono::steady_clock::now();
            auto end = std::chrono::steady_clock::now();
            while(end - start < timeout_duration && m_ptr->status != Status::done){
                end = std::chrono::steady_clock::now();
            }
            return m_ptr->status;
        }

        template<class Clock,class Duration>
        Status wait_for(const std::chrono::time_point<Clock,Duration> &timeout_point)const{
            while(std::chrono::steady_clock::now() < timeout_point && m_ptr->status != Status::done){}
            return m_ptr->status;
        }

        template <class TaskFunc>
        auto then(TaskFunc &&task_func,Policy policy = Policy::pool) -> Continuation<return_type_t<TaskFunc>>{
            auto ptr = std::make_shared<ContinuationBase<return_type_t<TaskFunc>>>();
            m_ptr->then_policy = policy;
            m_ptr->then = [ptr,task_func,this_ptr = this->m_ptr](std::exception_ptr except)->void{
                ptr->status = Status::running;
                if(except){
                    ptr->then_exception = except;
                }else{
                    try{
                        if constexpr (std::is_void<return_type_t<TaskFunc>>::value){
                            task_func(this_ptr->data);
                        }else{
                            ptr->data = task_func(this_ptr->data);
                        }
                    }catch(...){
                        ptr->then_exception = std::current_exception();
                    }
                }
                if(ptr->then){
                    switch(ptr->then_policy){
                        case Policy::pool:
                            ThreadPool::instance().addTask(ptr->then,ptr->then_exception);
                            break;
                        case Policy::synchronized:
                            ptr->then(ptr->then_exception);
                            break;
                        case Policy::thread:
                            std::thread t(ptr->then,ptr->then_exception);
                            t.detach();
                            break;
                    }
                }
                ptr->status = Status::done;
            };
            if(m_ptr->status == Status::done){
                switch(m_ptr->then_policy){
                    case Policy::pool:
                        ThreadPool::instance().addTask(m_ptr->then,m_ptr->exception);
                        break;
                    case Policy::synchronized:
                        m_ptr->then(m_ptr->exception);
                        break;
                    case Policy::thread:
                        std::thread t(ptr->then,ptr->then_exception);
                        t.detach();
                        break;
                }
            }
            return Continuation<return_type_t<TaskFunc>>(ptr);
        }

        template <class TaskFunc,class ErrFunc>
        auto then(TaskFunc &&task_func,ErrFunc &&err_func,Policy policy = Policy::pool) -> Continuation<return_type_t<TaskFunc>>{
            auto ptr = std::make_shared<ContinuationBase<return_type_t<TaskFunc>>>();
            m_ptr->then_policy = policy;
            m_ptr->then = [ptr,task_func,err_func,this_ptr = this->m_ptr](std::exception_ptr except)->void{
                ptr->status = Status::running;
                if(except){
                    try{
                        err_func(except);
                    }catch(...){
                        ptr->then_exception = std::current_exception();
                    }
                }else{
                    try{
                        if constexpr (std::is_void<return_type_t<TaskFunc>>::value){
                            task_func(this_ptr->data);
                        }else{
                            ptr->data = task_func(this_ptr->data);
                        }
                    }catch(...){
                        try{
                            err_func(std::current_exception());
                        }catch(...){
                            ptr->then_exception = std::current_exception();
                        }
                    }
                }
                if(ptr->then){
                    switch(ptr->then_policy){
                        case Policy::pool:
                            ThreadPool::instance().addTask(ptr->then,ptr->then_exception);
                            break;
                        case Policy::synchronized:
                            ptr->then(ptr->then_exception);
                            break;
                        case Policy::thread:
                            std::thread t(ptr->then,ptr->then_exception);
                            t.detach();
                            break;
                    }
                }
                ptr->status = Status::done;
            };
            if(m_ptr->status == Status::done){
                switch(m_ptr->then_policy){
                    case Policy::pool:
                        ThreadPool::instance().addTask(m_ptr->then,m_ptr->exception);
                        break;
                    case Policy::synchronized:
                        m_ptr->then(m_ptr->exception);
                        break;
                    case Policy::thread:
                        std::thread t(m_ptr->then,m_ptr->exception);
                        t.detach();
                        break;
                }
            }
            return Continuation<return_type_t<TaskFunc>>(ptr);
        }

    protected:
    private:
        std::shared_ptr<TaskBase<Type>> m_ptr;

        void m_run()noexcept{
            m_ptr->status = Status::running;
            try{
                m_ptr->data =  m_ptr->task();
            }catch(...){
                if( m_ptr->error){
                    try{
                        m_ptr->error(std::current_exception());
                    }catch(...){
                        m_ptr->exception = std::current_exception();
                    }
                }else{
                    m_ptr->exception = std::current_exception();
                }
            }
            if( m_ptr->then){
                switch( m_ptr->then_policy){
                    case Policy::pool:
                        ThreadPool::instance().addTask( m_ptr->then, m_ptr->exception);
                        break;
                    case Policy::synchronized:
                        m_ptr->then( m_ptr->exception);
                        break;
                    case Policy::thread:
                        std::thread t(m_ptr->then,m_ptr->exception);
                        t.detach();
                        break;
                }
            }
            m_ptr-> status = Status::done;
        }
    };

    template <>
    class Task<void>{
    public:
        using value_type = void;

        template <class TaskFunc>
        Task(TaskFunc &&task_func,bool auto_run = true,Policy policy = Policy::pool)
                :m_ptr(std::make_shared<TaskBase<void>>()){
            m_ptr->task = std::forward<TaskFunc>(task_func);
            m_ptr->policy = policy;
            if(auto_run)run();
        }
        template <class TaskFunc,class ErrFunc>
        Task(TaskFunc &&task_func,ErrFunc &&err_func,bool auto_run = true,Policy policy = Policy::pool)
                :m_ptr(std::make_shared<TaskBase<void>>()){
            m_ptr->task = std::forward<TaskFunc>(task_func);
            m_ptr->error = std::forward<ErrFunc>(err_func);
            m_ptr->policy = policy;
            if(auto_run)run();
        }
        Task(std::shared_ptr<TaskBase<void>> ptr)
            :m_ptr(std::move(ptr)){}
        Task(const Task &) = default;
        Task(Task &&) = default;
        ~Task() = default;

        Task &operator=(std::shared_ptr<TaskBase<void>> ptr){
            m_ptr = std::move(ptr);
            return *this;
        }
        Task &operator=(const Task &) = default;
        Task &operator=(Task &&) = default;

        void run()noexcept{
            switch(m_ptr->policy){
                case Policy::pool:
                    ThreadPool::instance().addTask(std::bind(&Task::m_run,this));
                    break;
                case Policy::synchronized:
                    m_run();
                    break;
                case Policy::thread:
                    std::thread t(std::bind(&Task::m_run,this));
                    t.detach();
                    break;
            }
        }

        void get(){
            wait();
            if(m_ptr->exception){
                std::rethrow_exception(m_ptr->exception);
            }
        }

        void wait()const noexcept{
            while(m_ptr->status != Status::done){}
        }

        template<class Rep,class Period>
        Status wait_for(const std::chrono::duration<Rep,Period> &timeout_duration)const{
            auto start = std::chrono::steady_clock::now();
            auto end = std::chrono::steady_clock::now();
            while(end - start < timeout_duration && m_ptr->status != Status::done){
                end = std::chrono::steady_clock::now();
            }
            return m_ptr->status;
        }

        template<class Clock,class Duration>
        Status wait_for(const std::chrono::time_point<Clock,Duration> &timeout_point)const{
            while(std::chrono::steady_clock::now() < timeout_point && m_ptr->status != Status::done){}
            return m_ptr->status;
        }

        template <class TaskFunc>
        auto then(TaskFunc &&task_func,Policy policy = Policy::pool) -> Continuation<return_type_t<TaskFunc>>{
            auto ptr = std::make_shared<ContinuationBase<return_type_t<TaskFunc>>>();
            m_ptr->then_policy = policy;
            m_ptr->then = [ptr,task_func](std::exception_ptr except)->void{
                ptr->status = Status::running;
                if(except){
                    ptr->then_exception = except;
                }else{
                    try{
                        if constexpr (std::is_void<return_type_t<TaskFunc>>::value){
                            task_func();
                        }else{
                            ptr->data = task_func();
                        }
                    }catch(...){
                        ptr->then_exception = std::current_exception();
                    }
                }
                if(ptr->then){
                    switch(ptr->then_policy){
                        case Policy::pool:
                            ThreadPool::instance().addTask(ptr->then,ptr->then_exception);
                            break;
                        case Policy::synchronized:
                            ptr->then(ptr->then_exception);
                            break;
                        case Policy::thread:
                            std::thread t(ptr->then,ptr->then_exception);
                            t.detach();
                            break;
                    }
                }
                ptr->status = Status::done;
            };
            if(m_ptr->status == Status::done){
                switch(m_ptr->then_policy){
                    case Policy::pool:
                        ThreadPool::instance().addTask(m_ptr->then,m_ptr->exception);
                        break;
                    case Policy::synchronized:
                        m_ptr->then(m_ptr->exception);
                        break;
                    case Policy::thread:
                        std::thread t(m_ptr->then,m_ptr->exception);
                        t.detach();
                        break;
                }
            }
            return Continuation<return_type_t<TaskFunc>>(ptr);
        }

        template <class TaskFunc,class ErrFunc>
        auto then(TaskFunc &&task_func,ErrFunc &&err_func,Policy policy = Policy::pool) -> Continuation<return_type_t<TaskFunc>>{
            auto ptr = std::make_shared<ContinuationBase<return_type_t<TaskFunc>>>();
            m_ptr->then_policy = policy;
            m_ptr->then = [ptr,task_func,err_func](std::exception_ptr except)->void{
                ptr->status = Status::running;
                if(except){
                    try{
                        err_func(except);
                    }catch(...){
                        ptr->then_exception = std::current_exception();
                    }
                }else{
                    try{
                        if constexpr (std::is_void<return_type_t<TaskFunc>>::value){
                            task_func();
                        }else{
                            ptr->data = task_func();
                        }
                    }catch(...){
                        try{
                            err_func(std::current_exception());
                        }catch(...){
                            ptr->then_exception = std::current_exception();
                        }
                    }
                }
                if(ptr->then){
                    switch(ptr->then_policy){
                        case Policy::pool:
                            ThreadPool::instance().addTask(ptr->then,ptr->then_exception);
                            break;
                        case Policy::synchronized:
                            ptr->then(ptr->then_exception);
                            break;
                        case Policy::thread:
                            std::thread t(ptr->then,ptr->then_exception);
                            t.detach();
                            break;
                    }
                }
                ptr->status = Status::done;
            };
            if(m_ptr->status == Status::done){
                switch(m_ptr->then_policy){
                    case Policy::pool:
                        ThreadPool::instance().addTask(m_ptr->then,m_ptr->exception);
                        break;
                    case Policy::synchronized:
                        m_ptr->then(m_ptr->exception);
                        break;
                    case Policy::thread:
                        std::thread t(m_ptr->then,m_ptr->exception);
                        t.detach();
                        break;
                }
            }
            return Continuation<return_type_t<TaskFunc>>(ptr);
        }

    protected:
    private:
        std::shared_ptr<TaskBase<void>> m_ptr;

        void m_run()noexcept{
            m_ptr->status = Status::running;
            try{
                m_ptr->task();
            }catch(...){
                if( m_ptr->error){
                    try{
                        m_ptr->error(std::current_exception());
                    }catch(...){
                        m_ptr->exception = std::current_exception();
                    }
                }else{
                    m_ptr->exception = std::current_exception();
                }
            }
            if(m_ptr->then){
                switch( m_ptr->then_policy){
                    case Policy::pool:
                        ThreadPool::instance().addTask( m_ptr->then, m_ptr->exception);
                        break;
                    case Policy::synchronized:
                        m_ptr->then( m_ptr->exception);
                        break;
                    case Policy::thread:
                        std::thread t(m_ptr->then,m_ptr->exception);
                        t.detach();
                        break;
                }
            }
            m_ptr-> status = Status::done;
        }
    };
}

#endif //_XTASK_TASK_H_