#ifndef _XTASK_TASK_H_
#define _XTASK_TASK_H_

#include <functional>
#include <future>
#include "return_type.h"

namespace xtask{
    enum class Policy{
        pool,
        synchronized,
        thread,
        lazy
    };

    enum class TaskState{
        running,
        waiting,
        done
    };

    template <class Type>
    class Task{
    public:
        Task()
            :m_function(nullptr),
             m_policy(Policy::pool),
             m_result(),
             m_state(TaskState::waiting),
             m_exception(nullptr){}
        template <class Function>
        Task(Function &&function,Policy policy = Policy::pool)
            :m_function(std::forward<Function>(function)),
             m_policy(policy),
             m_result(),
             m_state(TaskState::waiting),
             m_exception(nullptr){}
        Task(const Task<Type> &task)
            :m_function(task.m_function),
             m_policy(task.m_policy),
             m_result(),
             m_state(TaskState::waiting),
             m_exception(nullptr){}
        Task(Task<Type> &&) = default;
        ~Task() = default;

        Task &operator=(const Task<Type> &task){
            m_function = task.m_function;
            m_policy = task.m_policy;
            m_result = Type();
            m_state = TaskState::waiting;
            m_exception = nullptr;
            return *this;
        }
        Task &operator=(Task<Type> &&) = default;

        void run(){
            if(m_state == TaskState::done || m_policy == Policy::lazy)
                return;
            m_state = TaskState::running;
            try {
                m_result = m_function();
                m_state = TaskState::done;
            }catch (...){
                m_state = TaskState::done;
                m_exception = std::current_exception();
            }
        }

        Type get(){
            if(m_policy == Policy::lazy){
                m_state = TaskState::running;
                m_result = m_function();
                m_state = TaskState::done;
                return m_result;
            }
            while(m_state != TaskState::done && !m_exception){}
            if(m_exception) std::rethrow_exception(m_exception);
            return m_result;
        }

        void wait()const{
            while(m_state != TaskState::done && !m_exception){}
        }

        template <class Rep, class Period>
        TaskState wait_for(const std::chrono::duration<Rep,Period>& timeout_duration)const{
            return m_state;
        }

        template <class Clock, class Duration>
        TaskState wait_until(const std::chrono::time_point<Clock,Duration>& timeout_time)const{
            return m_state;
        }

        /*template <class Function>
        auto then(Function &&func,Policy policy = Policy::pool) -> Task<return_type_t<Function>>&{
            return
        }*/

        TaskState getState()const noexcept{
            return m_state;
        }

        void setPolicy(Policy policy)noexcept{
            m_policy = policy;
        }

        Policy getPolicy()const noexcept{
            return m_policy;
        }

    protected:
    private:
        std::function<Type()> m_function;
        Policy m_policy;
        Type m_result;
        TaskState m_state;
        std::exception_ptr m_exception;
    };
}

#endif