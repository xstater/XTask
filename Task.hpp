#ifndef _XTASK_TASK_H_
#define _XTASK_TASK_H_

#include <functional>
#include <memory>
#include <utility>
#include "ThreadPool.hpp"
#include "XSignal.hpp"

template <class Type>
class Task{};

template <>
class Task<void()>{
public:
    Task(std::function<void()> func,ThreadPool::TaskPriority priority = ThreadPool::TaskPriority::DEFAULT);
    Task(const Task<void()> &) = delete;
    Task(Task<void()> &&) = default;
    ~Task() = default;
    
    Task &operator=(const Task<void()> &) = delete;
    Task &operator=(Task<void()> &&) = default;

    void run();
    Task<void()> &then(Task<void()> &&task);
    
    Signal<void()> onComplete;
    
protected:
private:
    std::function<void()> m_funcs;
    ThreadPool::TaskPriority m_priority;
    std::unique_ptr<Task<void()>> m_then;
};

template<>
Task<void()>::Task(std::function<void()> func,ThreadPool::TaskPriority priority = ThreadPool::TaskPriority::DEFAULT)
    :m_funcs(func),
     m_priority(priority),
     m_then(nullptr){}
     
template<>
void Task<void()>::run(){
    ThreadPool::instance().addTask(m_priority,[&onComplete,&m_funcs]()->void{
        m_funcs();
        onComplete.emit();
        if(m_then)m_then.run();
    });
}

template<>
Task<void()> &Task<void()>::then(Task<void()> &&task){
    m_then = std::make_unique<Task<void>>(std::move(task));
    return *m_then;
}
     

#endif //_XTASK_TASK_H_
