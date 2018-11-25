#ifndef _XTASK_PROMISE_H_
#define _XTASK_PROMISE_H_

#include <utility>
#include <exception>
#include <memory>
#include "Future.h"

namespace xtask{
    template <class Type>
    class Promise{
    public:
        Promise()
            :m_future(std::make_shared<FutureBase<Type>>()){}
        Promise(const Promise &) = delete;
        Promise(Promise &&) = default;
        ~Promise() = default;

        Promise &operator=(const Promise &) = delete;
        Promise &operator=(Promise &&) = default;

        void set(const Type &data){
            m_future->m_data = data;
            m_future->m_status = Status::done;
            if(m_future->m_then){
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
        }
        void set(std::exception_ptr exception){
            m_future->m_exception = exception;
        }

        Future<Type> get_future()const{
            return Future<Type>(m_future);
        }
    protected:
    private:
        std::shared_ptr<FutureBase<Type>> m_future;
    };

    template <>
    class Promise<void>{
    public:
        Promise()
                :m_future(std::make_shared<FutureBase<void>>()){}
        Promise(const Promise &) = delete;
        Promise(Promise &&) = default;
        ~Promise() = default;

        Promise &operator=(const Promise &) = delete;
        Promise &operator=(Promise &&) = default;

        void set(){
            m_future->m_status = Status::done;
            if(m_future->m_then){
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
        }
        void set(std::exception_ptr exception){
            m_future->m_exception = exception;
        }

        Future<void> get_future()const{
            return Future<void>(m_future);
        }
    protected:
    private:
        std::shared_ptr<FutureBase<void>> m_future;
    };
}

#endif //_XTASK_PROMISE_H_