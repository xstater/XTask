#ifndef _XTASK_TASK_H_
#define _XTASK_TASK_H_

#include <future>
#include "XSignal.hpp"

namespace xtask{

    template <class ReturnType>
    class Task{
    public:
        template <class Function,class...ArgsType>
        Task(Function &&func,ArgsType&&...args)
            :m_function(std::bind(std::forward(func),std::forward(args)...)){}
        Task(const Task &) = delete;
        Task(Task &&) = default;
        ~Task() = default;

        Task &operator=(const Task &) = delete;
        Task &operator=(Task &&) = default;

        std::future<ReturnType> get_future(){ m_promise.get_future(); }

        void operator()()noexcept{
            try {
                m_promise.set_value(m_function());
            }catch(std::exception &e){
                m_promise.set_exception(e);
            }
        }

    protected:
    private:
        std::function<ReturnType()> m_function;
        std::promise<ReturnType> m_promise;
    };
}

#endif