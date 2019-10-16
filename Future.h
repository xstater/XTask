#ifndef XTASK_FUTURE_H
#define XTASK_FUTURE_H

#include <memory>
#include "FutureBase.h"
#include "Policy.h"
#include "ThreadPool.h"
#include "return_type.h"

namespace xtask{
    template <class Type>
    class Future{
    public:
        Future(const std::shared_ptr<detail::FutureBase<Type>> &future)
            :m_future(future){}
        Future(const Future &) = default;
        Future(Future &&) = default;
        ~Future() = default;

        Future &operator=(const Future &) = default;
        Future &operator=(Future &&) = default;

        Type get(){
            return m_future->get();
        }
        void wait()const{
            m_future->wait();
        }
        template <class Rep,class Period>
        bool wait_for(const std::chrono::duration<Rep,Period> &duration)const{
            return m_future->wait_for(duration);
        }
        template <class Clock,class Duration>
        bool wait_until(const std::chrono::time_point<Clock,Duration> &timePoint)const{
            return m_future->wait_until(timePoint);
        }


        ///non void continuation function
        template <class Function>
        auto then(Function &&f,Policy policy = Policy::pool)
        -> typename std::enable_if<!std::is_void<return_type_t<Function>>::value,Future<return_type_t<Function>>>::type{
            using ReturnType = return_type_t<Function>;

            auto future_base = std::make_shared<detail::FutureBase<ReturnType>>();
            auto func = [this,func = std::forward<Function>(f),fb = future_base]()->void{
                try{
                    fb->emplace(func(*this));
                }catch(...){
                    fb->set_exception(std::current_exception());
                }
            };

            switch(policy){
                case Policy::pool:
                    ThreadPool::instance().addTask(func);
                    break;
                case Policy::thread:
                    std::async(std::launch::async,func);
                    break;
            }

            return Future<ReturnType>(future_base);
        }

        ///void continuation function
        template <class Function>
        auto then(Function &&f,Policy policy = Policy::pool)
        -> typename std::enable_if<std::is_void<return_type_t<Function>>::value,Future<return_type_t<Function>>>::type{
            using ReturnType = return_type_t<Function>;

            auto future_base = std::make_shared<detail::FutureBase<ReturnType>>();
            auto func = [this,func = std::forward<Function>(f),fb = future_base]()->void{
                try{
                    func(*this);
                    fb->emplace();
                }catch(...){
                    fb->set_exception(std::current_exception());
                }
            };

            switch(policy){
                case Policy::pool:
                    ThreadPool::instance().addTask(func);
                    break;
                case Policy::thread:
                    std::async(std::launch::async,func);
                    break;
            }

            return Future<ReturnType>(future_base);
        }
    protected:
    private:
        std::shared_ptr<detail::FutureBase<Type>> m_future;
    };

    template <class Type>
    class Future<Type&>{
    public:
        Future(const std::shared_ptr<detail::FutureBase<Type&>> &future)
                :m_future(future){}
        Future(const Future &) = default;
        Future(Future &&) = default;
        ~Future() = default;

        Future &operator=(const Future &) = default;
        Future &operator=(Future &&) = default;

        Type &get(){
            return m_future->get();
        }
        void wait()const{
            m_future->wait();
        }
        template <class Rep,class Period>
        bool wait_for(const std::chrono::duration<Rep,Period> &duration)const{
            return m_future->wait_for(duration);
        }
        template <class Clock,class Duration>
        bool wait_until(const std::chrono::time_point<Clock,Duration> &timePoint)const{
            return m_future->wait_until(timePoint);
        }


        ///non void continuation function
        template <class Function>
        auto then(Function &&f,Policy policy = Policy::pool)
        -> typename std::enable_if<!std::is_void<return_type_t<Function>>::value,Future<return_type_t<Function>>>::type{
            using ReturnType = return_type_t<Function>;

            auto future_base = std::make_shared<detail::FutureBase<ReturnType>>();
            auto func = [this,func = std::forward<Function>(f),fb = future_base]()->void{
                try{
                    fb->emplace(func(*this));
                }catch(...){
                    fb->set_exception(std::current_exception());
                }
            };

            switch(policy){
                case Policy::pool:
                    ThreadPool::instance().addTask(func);
                    break;
                case Policy::thread:
                    std::async(std::launch::async,func);
                    break;
            }

            return Future<ReturnType>(future_base);
        }

        ///void continuation function
        template <class Function>
        auto then(Function &&f,Policy policy = Policy::pool)
        -> typename std::enable_if<std::is_void<return_type_t<Function>>::value,Future<return_type_t<Function>>>::type{
            using ReturnType = return_type_t<Function>;

            auto future_base = std::make_shared<detail::FutureBase<ReturnType>>();
            auto func = [this,func = std::forward<Function>(f),fb = future_base]()->void{
                try{
                    func(*this);
                    fb->emplace();
                }catch(...){
                    fb->set_exception(std::current_exception());
                }
            };

            switch(policy){
                case Policy::pool:
                    ThreadPool::instance().addTask(func);
                    break;
                case Policy::thread:
                    std::async(std::launch::async,func);
                    break;
            }

            return Future<ReturnType>(future_base);
        }
    protected:
    private:
        std::shared_ptr<detail::FutureBase<Type&>> m_future;
    };

    template <>
    class Future<void>{
    public:
        Future(const std::shared_ptr<detail::FutureBase<void>> &future)
                :m_future(future){}
        Future(const Future &) = default;
        Future(Future &&) = default;
        ~Future() = default;

        Future &operator=(const Future &) = default;
        Future &operator=(Future &&) = default;

        void get(){
            m_future->get();
        }
        void wait()const{
            m_future->wait();
        }
        template <class Rep,class Period>
        bool wait_for(const std::chrono::duration<Rep,Period> &duration)const{
            return m_future->wait_for(duration);
        }
        template <class Clock,class Duration>
        bool wait_until(const std::chrono::time_point<Clock,Duration> &timePoint)const{
            return m_future->wait_until(timePoint);
        }


        ///non void continuation function
        template <class Function>
        auto then(Function &&f,Policy policy = Policy::pool)
        -> typename std::enable_if<!std::is_void<return_type_t<Function>>::value,Future<return_type_t<Function>>>::type{
            using ReturnType = return_type_t<Function>;

            auto future_base = std::make_shared<detail::FutureBase<ReturnType>>();
            auto func = [this,func = std::forward<Function>(f),fb = future_base]()->void{
                try{
                    fb->emplace(func(*this));
                }catch(...){
                    fb->set_exception(std::current_exception());
                }
            };

            switch(policy){
                case Policy::pool:
                    ThreadPool::instance().addTask(func);
                    break;
                case Policy::thread:
                    std::async(std::launch::async,func);
                    break;
            }

            return Future<ReturnType>(future_base);
        }

        ///void continuation function
        template <class Function>
        auto then(Function &&f,Policy policy = Policy::pool)
        -> typename std::enable_if<std::is_void<return_type_t<Function>>::value,Future<return_type_t<Function>>>::type{
            using ReturnType = return_type_t<Function>;

            auto future_base = std::make_shared<detail::FutureBase<ReturnType>>();
            auto func = [this,func = std::forward<Function>(f),fb = future_base]()->void{
                try{
                    func(*this);
                    fb->emplace();
                }catch(...){
                    fb->set_exception(std::current_exception());
                }
            };

            switch(policy){
                case Policy::pool:
                    ThreadPool::instance().addTask(func);
                    break;
                case Policy::thread:
                    std::async(std::launch::async,func);
                    break;
            }

            return Future<ReturnType>(future_base);
        }
    protected:
    private:
        std::shared_ptr<detail::FutureBase<void>> m_future;
    };
}

#endif //XTASK_FUTURE_H
