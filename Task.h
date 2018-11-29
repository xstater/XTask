#ifndef _XTASK_TASK_H_
#define _XTASK_TASK_H_

#include <memory>
#include "Future.h"

namespace xtask{
    template <class Type>
    class Task{
    public:
        template <class Function>
        Task(Function &&function,Policy policy = Policy::pool,bool auto_run = false)
            :m_function(std::forward<Function>(function)),
             m_policy(policy),
             m_future(std::make_shared<FutureBase<Type>>()){
             if(auto_run) run();
         }
        Task(const Task &) = delete;
        Task(Task &&) = default;
        ~Task() = default;

        Task &operator=(const Task &) = delete;
        Task &operator=(Task &&) = default;

        void run(){
            switch(m_policy){
                case Policy::pool:
                    ThreadPool::instance().addTask([this]()->void{
                        try{
                            m_future->m_status = Status::running;
                            m_future->m_data = m_function();
                            m_future->m_status = Status::done;
                        }catch(...){
                            m_future->m_exception = std::current_exception();
                        }
                        if(m_future->m_then){
                            m_future->m_called = true;
                            switch(m_future->m_then_policy){
                                case Policy::pool:
                                    ThreadPool::instance().addTask(m_future->m_then);
                                    break;
                                case Policy::thread:
                                    std::async(std::launch::async,m_future->m_then);
                                    break;
                                case Policy::synchronized:
                                    m_future->m_then();
                                    break;
                            }
                        }
                    });
                    break;
                case Policy::thread:
                    std::async(std::launch::async,[this]()->void{
                        try{
                            m_future->m_status = Status::running;
                            m_future->m_data = m_function();
                            m_future->m_status = Status::done;
                        }catch(...){
                            m_future->m_exception = std::current_exception();
                        }
                        if(m_future->m_then){
                            m_future->m_called = true;
                            switch(m_future->m_then_policy){
                                case Policy::pool:
                                    ThreadPool::instance().addTask(m_future->m_then);
                                    break;
                                case Policy::thread:
                                    std::async(std::launch::async,m_future->m_then);
                                    break;
                                case Policy::synchronized:
                                    m_future->m_then();
                                    break;
                            }
                        }
                    });
                    break;
                case Policy::synchronized:
                    try{
                        m_future->m_status = Status::running;
                        m_future->m_data = m_function();
                        m_future->m_status = Status::done;
                    }catch(...){
                        m_future->m_exception = std::current_exception();
                    }
                    if(m_future->m_then){
                        m_future->m_called = true;
                        switch(m_future->m_then_policy){
                            case Policy::pool:
                                ThreadPool::instance().addTask(m_future->m_then);
                                break;
                            case Policy::thread:
                                std::async(std::launch::async,m_future->m_then);
                                break;
                            case Policy::synchronized:
                                m_future->m_then();
                                break;
                        }
                    }
                    break;
            }
        }

        Future<Type> get_future()const noexcept{
            return Future<Type>(m_future);
        }
    protected:
    private:
        std::function<Type()> m_function;
        Policy  m_policy;
        std::shared_ptr<FutureBase<Type>> m_future;
    };



    template <>
    class Task<void>{
    public:
        template <class Function>
        Task(Function &&function,Policy policy = Policy::pool,bool auto_run = false)
                :m_function(std::forward<Function>(function)),
                 m_policy(policy),
                 m_future(std::make_shared<FutureBase<void>>()){
            if(auto_run) run();
         }
        Task(const Task &) = delete;
        Task(Task &&) = default;
        ~Task() = default;

        Task &operator=(const Task &) = delete;
        Task &operator=(Task &&) = default;

        void run(){
            switch(m_policy){
                case Policy::pool:
                    ThreadPool::instance().addTask([this]()->void{
                        try{
                            m_future->m_status = Status::running;
                            m_function();
                            m_future->m_status = Status::done;
                        }catch(...){
                            m_future->m_exception = std::current_exception();
                        }
                        if(m_future->m_then){
                            m_future->m_called = true;
                            switch(m_future->m_then_policy){
                                case Policy::pool:
                                    ThreadPool::instance().addTask(m_future->m_then);
                                    break;
                                case Policy::thread:
                                    std::async(std::launch::async,m_future->m_then);
                                    break;
                                case Policy::synchronized:
                                    m_future->m_then();
                                    break;
                            }
                        }
                    });
                    break;
                case Policy::thread:
                    std::async(std::launch::async,[this]()->void{
                        try{
                            m_future->m_status = Status::running;
                            m_function();
                            m_future->m_status = Status::done;
                        }catch(...){
                            m_future->m_exception = std::current_exception();
                        }
                        if(m_future->m_then){
                            m_future->m_called = true;
                            switch(m_future->m_then_policy){
                                case Policy::pool:
                                    ThreadPool::instance().addTask(m_future->m_then);
                                    break;
                                case Policy::thread:
                                    std::async(std::launch::async,m_future->m_then);
                                    break;
                                case Policy::synchronized:
                                    m_future->m_then();
                                    break;
                            }
                        }
                    });
                    break;
                case Policy::synchronized:
                    try{
                        m_future->m_status = Status::running;
                        m_function();
                        m_future->m_status = Status::done;
                    }catch(...){
                        m_future->m_exception = std::current_exception();
                    }
                    if(m_future->m_then){
                        m_future->m_called = true;
                        switch(m_future->m_then_policy){
                            case Policy::pool:
                                ThreadPool::instance().addTask(m_future->m_then);
                                break;
                            case Policy::thread:
                                std::async(std::launch::async,m_future->m_then);
                                break;
                            case Policy::synchronized:
                                m_future->m_then();
                                break;
                        }
                    }
                    break;
            }
        }

        Future<void> get_future()const noexcept{
            return Future<void>(m_future);
        }
    protected:
    private:
        std::function<void()> m_function;
        Policy  m_policy;
        std::shared_ptr<FutureBase<void>> m_future;
    };
}

#endif