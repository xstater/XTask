#ifndef _XTASK_TASK_H_
#define _XTASK_TASK_H_

#include <functional>
#include <future>
#include "return_type.h"

namespace xtask{
    template <class Type>
    class Task{
    public:
        Task() = default;
        template <class Function>
        Task(Function &&function)
            :m_function(std::forward<Function>(function)),
             m_result(),
             m_is_done(false),
             m_exception(nullptr){}
        Task(const Task &) = delete;
        Task(Task &&) = default;
        ~Task() = default;

        Task &operator=(const Task &) = delete;
        Task &operator=(Task &&) = default;

        void operator()() {
            m_result = m_function();
        }

        void run();

        template <class Function>
        auto then(Function &&func) -> Task<return_type_t<Function>>{
            return Task<return_type_t<Function>>(std::bind(std::forward<Function>(func),m_result));
        }

        Type get(){
            while(!m_is_done && !m_exception){}
            return m_result;
        }

        void wait(){

        }

        template<>
        void wait_for(){

        }

        template<>
        void wait_until(){

        }


    protected:
    private:
        std::function<Type()> m_function;
        Type m_result;
        bool m_is_done;
        std::exception_ptr m_exception;
    };
}

#endif