#ifndef _XTASK_CONTINUATION_H_
#define _XTASK_CONTINUATION_H_

#include <functional>
#include <memory>
#include <exception>
#include <type_traits>
#include <chrono>
#include <thread>
#include "return_type.h"
#include "ThreadPool.h"

namespace xtask{

    enum class Status {
        waiting,
        running,
        done
    };

    enum class Policy{
        pool,
        synchronized,
        thread
    };

    template <class Type>
    struct ContinuationBase{
        ContinuationBase()
            :data(),
             status(Status::waiting),
             then_exception(nullptr),
             then_policy(Policy::pool),
             then(nullptr){}
         ~ContinuationBase() = default;

        Type data;
        Status status;
        std::exception_ptr then_exception;
        Policy then_policy;
        std::function<void(std::exception_ptr)> then;
    };

    template <>
    struct ContinuationBase<void>{
        ContinuationBase()
                :status(Status::waiting),
                 then_exception(nullptr),
                 then_policy(Policy::pool),
                 then(nullptr){}
        ~ContinuationBase() = default;

        Status status;
        std::exception_ptr then_exception;
        Policy then_policy;
        std::function<void(std::exception_ptr)> then;
    };


    template <class Type>
    class Continuation{
    public:
        using value_type = Type;

        Continuation(std::shared_ptr<ContinuationBase<Type>> ptr)
                :m_ptr(std::move(ptr)){}
        Continuation(const Continuation &) = default;
        Continuation(Continuation &&) = default;

        Continuation &operator=(const Continuation &) = default;
        Continuation &operator=(Continuation &&) = default;

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
                        ThreadPool::instance().addTask(m_ptr->then,m_ptr->then_exception);
                        break;
                    case Policy::synchronized:
                        m_ptr->then(m_ptr->then_exception);
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
            };
            if(m_ptr->status == Status::done){
                switch(m_ptr->then_policy){
                    case Policy::pool:
                        ThreadPool::instance().addTask(m_ptr->then,m_ptr->then_exception);
                        break;
                    case Policy::synchronized:
                        m_ptr->then(m_ptr->then_exception);
                        break;
                    case Policy::thread:
                        std::thread t(ptr->then,ptr->then_exception);
                        t.detach();
                        break;
                }
            }
            return Continuation<return_type_t<TaskFunc>>(ptr);
        }
    protected:
    private:
        std::shared_ptr<ContinuationBase<Type>> m_ptr;
    };

    template <>
    class Continuation<void>{
    public:
        using value_type = void;

        Continuation(std::shared_ptr<ContinuationBase<void>> ptr)
                :m_ptr(std::move(ptr)){}
        Continuation(const Continuation &) = default;
        Continuation(Continuation &&) = default;

        Continuation &operator=(const Continuation &) = default;
        Continuation &operator=(Continuation &&) = default;

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
                        ThreadPool::instance().addTask(m_ptr->then,m_ptr->then_exception);
                        break;
                    case Policy::synchronized:
                        m_ptr->then(m_ptr->then_exception);
                        break;
                    case Policy::thread:
                        std::thread t(ptr->then,ptr->then_exception);
                        t.detach();;
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
            };
            if(m_ptr->status == Status::done){
                switch(m_ptr->then_policy){
                    case Policy::pool:
                        ThreadPool::instance().addTask(m_ptr->then,m_ptr->then_exception);
                        break;
                    case Policy::synchronized:
                        m_ptr->then(m_ptr->then_exception);
                        break;
                    case Policy::thread:
                        std::thread t(ptr->then,ptr->then_exception);
                        t.detach();
                        break;
                }
            }
            return Continuation<return_type_t<TaskFunc>>(ptr);
        }
    protected:
    private:
        std::shared_ptr<ContinuationBase<void>> m_ptr;
    };

}

#endif //_XTASK_CONTINUATION_H_