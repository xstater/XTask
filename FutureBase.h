#ifndef XTASK_FUTURE_BASE_H
#define XTASK_FUTURE_BASE_H

#include <mutex>
#include <condition_variable>
#include <chrono>
#include <type_traits>
#include <memory>
#include <utility>

namespace xtask{
namespace detail{
    template <class Type>
    class FutureBase{
    public:
        FutureBase()
            :m_mutex(),
             m_cond(),
             m_data(),
             m_exception(nullptr),
             m_done(false){}
        FutureBase(const FutureBase &) = delete;
        FutureBase(FutureBase &&) = delete;
        ~FutureBase() = default;

        FutureBase &operator=(const FutureBase &) = delete;
        FutureBase &operator=(FutureBase &&) = delete;

        template <class...Args>
        void emplace(Args&&...args){
            std::unique_lock<std::mutex> lock(m_mutex);
            try{
                m_data = std::make_unique<Type>(std::forward<Args>(args)...);
            }catch(...){
                m_exception = std::current_exception();
            }
            m_done = true;
            lock.unlock();
            m_cond.notify_all();
        }

        void set_exception(std::exception_ptr exception){
            std::unique_lock<std::mutex> lock(m_mutex);
            m_exception = std::move(exception);
            m_done = true;
            lock.unlock();
            m_cond.notify_all();
        }

        Type get(){
            std::unique_lock<std::mutex> lock(m_mutex);
            m_cond.wait(lock,[this]()->bool{ return m_done; });
            if(m_exception){
                std::rethrow_exception(m_exception);
            }
            return *m_data;
        }

        void wait()const{
            std::unique_lock<std::mutex> lock(m_mutex);
            m_cond.wait(lock,[this]()->bool{ return m_done; });
        }

        template <class Rep,class Period>
        bool wait_for(const std::chrono::duration<Rep,Period> &duration)const{
            std::unique_lock<std::mutex> lock(m_mutex);
            return m_cond.wait_for(lock,duration,[this]()->bool{ return m_done; });
        }

        template <class Clock,class Duration>
        bool wait_until(const std::chrono::time_point<Clock,Duration> &timePoint)const {
            std::unique_lock<std::mutex> lock(m_mutex);
            return m_cond.wait_until(lock,timePoint,[this]()->bool{ return m_done; });
        }
    protected:
    private:
        mutable std::mutex m_mutex;
        mutable std::condition_variable m_cond;
        std::unique_ptr<Type> m_data;
        std::exception_ptr m_exception;
        bool m_done;
    };

    template <class Type>
    class FutureBase<Type&>{
    public:
        FutureBase()
                :m_mutex(),
                 m_cond(),
                 m_data(nullptr),
                 m_exception(nullptr),
                 m_done(false){}
        FutureBase(const FutureBase &) = delete;
        FutureBase(FutureBase &&) = delete;
        ~FutureBase() = default;

        FutureBase &operator=(const FutureBase &) = delete;
        FutureBase &operator=(FutureBase &&) = delete;

        void emplace(Type &data){
            std::unique_lock<std::mutex> lock(m_mutex);
            m_data = &data;
            m_done = true;
            lock.unlock();
            m_cond.notify_all();
        }

        void set_exception(std::exception_ptr exception){
            std::unique_lock<std::mutex> lock(m_mutex);
            m_exception = std::move(exception);
            m_done = true;
            lock.unlock();
            m_cond.notify_all();
        }

        Type &get(){
            std::unique_lock<std::mutex> lock(m_mutex);
            m_cond.wait(lock,[this]()->bool{ return m_done; });
            if(m_exception){
                std::rethrow_exception(m_exception);
            }
            return *m_data;
        }

        void wait()const{
            std::unique_lock<std::mutex> lock(m_mutex);
            m_cond.wait(lock,[this]()->bool{ return m_done; });
        }

        template <class Rep,class Period>
        bool wait_for(const std::chrono::duration<Rep,Period> &duration)const{
            std::unique_lock<std::mutex> lock(m_mutex);
            return m_cond.wait_for(lock,duration,[this]()->bool{ return m_done; });
        }

        template <class Clock,class Duration>
        bool wait_until(const std::chrono::time_point<Clock,Duration> &timePoint)const {
            std::unique_lock<std::mutex> lock(m_mutex);
            return m_cond.wait_until(lock,timePoint,[this]()->bool{ return m_done; });
        }
    protected:
    private:
        mutable std::mutex m_mutex;
        mutable std::condition_variable m_cond;
        Type *m_data;
        std::exception_ptr m_exception;
        bool m_done;
    };

    template <>
    class FutureBase<void>{
    public:
        FutureBase()
                :m_mutex(),
                 m_cond(),
                 m_exception(nullptr),
                 m_done(false){}
        FutureBase(const FutureBase &) = delete;
        FutureBase(FutureBase &&) = delete;
        ~FutureBase() = default;

        FutureBase &operator=(const FutureBase &) = delete;
        FutureBase &operator=(FutureBase &&) = delete;

        void emplace(){
            std::unique_lock<std::mutex> lock(m_mutex);
            m_done = true;
            lock.unlock();
            m_cond.notify_all();
        }

        void set_exception(std::exception_ptr exception){
            std::unique_lock<std::mutex> lock(m_mutex);
            m_exception = std::move(exception);
            m_done = true;
            lock.unlock();
            m_cond.notify_all();
        }

        void get(){
            std::unique_lock<std::mutex> lock(m_mutex);
            m_cond.wait(lock,[this]()->bool{ return m_done; });
            if(m_exception){
                std::rethrow_exception(m_exception);
            }
        }

        void wait()const{
            std::unique_lock<std::mutex> lock(m_mutex);
            m_cond.wait(lock,[this]()->bool{ return m_done; });
        }

        template <class Rep,class Period>
        bool wait_for(const std::chrono::duration<Rep,Period> &duration)const{
            std::unique_lock<std::mutex> lock(m_mutex);
            return m_cond.wait_for(lock,duration,[this]()->bool{ return m_done; });
        }

        template <class Clock,class Duration>
        bool wait_until(const std::chrono::time_point<Clock,Duration> &timePoint)const {
            std::unique_lock<std::mutex> lock(m_mutex);
            return m_cond.wait_until(lock,timePoint,[this]()->bool{ return m_done; });
        }
    protected:
    private:
        mutable std::mutex m_mutex;
        mutable std::condition_variable m_cond;
        std::exception_ptr m_exception;
        bool m_done;
    };
}
}

#endif //XTASK_FUTURE_BASE_H
