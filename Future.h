#ifndef _XTASK_FUTURE_H_
#define _XTASK_FUTURE_H_

#include <exception>
#include <memory>
#include "return_type.h"
#include "ThreadPool.h"

namespace xtask{
    enum class Policy{
        pool,
        thread,
        synchronized
    };

    enum class Status{
        waiting,
        running,
        done
    };
    
    class FutureThenError:public std::exception{
    public:
        FutureThenError() = default;
        ~FutureThenError() = default;
        
        const char *what() const noexcept{
            return "Future: then() can only be invoked once!"
        }
    protected:
    private:
    };

    template <class Type>
    struct FutureBase{
        FutureBase()
            :m_data(),
             m_status(Status::waiting),
             m_exception(nullptr),
             m_then(nullptr),
             m_then_policy(Policy::pool){}
        ~FutureBase() = default;

        Type m_data;
        Status m_status;
        std::exception_ptr m_exception;
        std::function<void()> m_then;
        Policy m_then_policy;
    };

    template <>
    struct FutureBase<void>{
        FutureBase()
                :m_status(Status::waiting),
                 m_exception(nullptr),
                 m_then(nullptr),
                 m_then_policy(Policy::pool){}
        ~FutureBase() = default;

        Status m_status;
        std::exception_ptr m_exception;
        std::function<void()> m_then;
        Policy m_then_policy;
    };

    template <class Type>
    class Future{
    public:
        Future(std::shared_ptr<FutureBase<Type>> fut)
            :m_future(std::move(fut)){}
        Future(const Future &) = default;
        Future(Future &&) = default;
        ~Future() = default;

        Future &operator=(const Future &) = default;
        Future &operator=(Future &&) = default;

        Type get()const{
            wait();
            if(m_future->m_exception)std::rethrow_exception(m_future->m_exception);
            return m_future->m_data;
        }
        void wait()const noexcept{
            while(m_future->m_status != Status::done && !m_future->m_exception){}
        }

        template <class Function>
        auto then(Function &&function,Policy policy = Policy::pool) -> Future<return_type_t<Function>>{
            if(m_future->m_then)throw FutureThenError();
            auto ptr = std::make_shared<FutureBase<return_type_t<Function>>>();
            m_future->m_then_policy = policy;
            m_future->m_then = [ptr,function,this]()->void{
                try{
                    ptr->m_status = Status::running;
                    ptr->m_data = function(Future<Type>(*this));
                    ptr->m_status = Status::done;
                }catch(...){
                    ptr->m_exception = std::current_exception();
                }
                if(ptr->m_then){
                    switch(ptr->m_then_policy){
                        case Policy::pool:
                            ThreadPool::instance().addTask(ptr->m_then);
                            break;
                        case Policy::thread:
                            std::async(std::launch::async,ptr->m_then);
                            break;
                        case Policy::synchronized:
                            ptr->m_then();
                            break;
                    }
                }
            };
            return Future<return_type_t<Function>>(ptr);
        }
    protected:
    private:
        std::shared_ptr<FutureBase<Type>> m_future;
    };

    template <>
    class Future<void>{
    public:
        Future(std::shared_ptr<FutureBase<void>> fut)
                :m_future(std::move(fut)){}
        Future(const Future &) = default;
        Future(Future &&) = default;
        ~Future() = default;

        Future &operator=(const Future &) = default;
        Future &operator=(Future &&) = default;

        void get()const{
            wait();
            if(m_future->m_exception)std::rethrow_exception(m_future->m_exception);
        }
        void wait()const noexcept{
            while(m_future->m_status != Status::done && !m_future->m_exception){}
        }

        template <class Function>
        auto then(Function &&function,Policy policy = Policy::pool) -> Future<return_type_t<Function>>{
            if(m_future->m_then)throw FutureThenError();
            auto ptr = std::make_shared<FutureBase<return_type_t<Function>>>();
            m_future->m_then_policy = policy;
            m_future->m_then = [ptr,function,this]()->void{
                try{
                    ptr->m_status = Status::running;
                    function(Future<void>(*this));
                    ptr->m_status = Status::done;
                }catch(...){
                    ptr->m_exception = std::current_exception();
                }
                if(ptr->m_then){
                    switch(ptr->m_then_policy){
                        case Policy::pool:
                            ThreadPool::instance().addTask(ptr->m_then);
                            break;
                        case Policy::thread:
                            std::async(std::launch::async,ptr->m_then);
                            break;
                        case Policy::synchronized:
                            ptr->m_then();
                            break;
                    }
                }
            };
            return Future<return_type_t<Function>>(ptr);
        }
    protected:
    private:
        std::shared_ptr<FutureBase<void>> m_future;
    };
}

#endif //_XTASK_FUTURE_H_
