#ifndef _XTASK_TASK_H_
#define _XTASK_TASK_H_

#include <functional>
#include <memory>
#include <utility>
#include "ThreadPool.hpp"
#include "XSignal.hpp"

class Task{
public:
    Task(std::function<void()> func,ThreadPool::TaskPriority priority = ThreadPool::TaskPriority::DEFAULT);
    Task(const Task &) = delete;
    Task(Task &&) = default;
    ~Task() = default;
    
    Task &operator=(const Task &) = delete;
    Task &operator=(Task &&) = default;

    void run();
    Task &then(Task &&task);
    Task &then(std::function<void()> task,ThreadPool::TaskPriority priority = ThreadPool::TaskPriority::DEFAULT);
    
    Signal<void()> onComplete;
    
protected:
private:
    std::function<void()> m_func;
    ThreadPool::TaskPriority m_priority;
    std::unique_ptr<Task> m_then;
};

Task::Task(std::function<void()> func,ThreadPool::TaskPriority priority)
    :m_func(std::move(func)),
     m_priority(priority),
     m_then(nullptr){}

void Task::run(){
    ThreadPool::instance().addTask(m_priority,[this]()->void{
        m_func();
        onComplete.emit();
    });
}

Task &Task::then(Task &&task){
    m_then = std::make_unique<Task>(std::move(task));
    onComplete.connect([this]()->void{m_then->run();});
    return *m_then;
}

Task &Task::then(std::function<void()> task, ThreadPool::TaskPriority priority) {
    m_then = std::make_unique<Task>(std::move(task),priority);
    onComplete.connect([this]()->void{m_then->run();});
    return  *m_then;
}


#endif //_XTASK_TASK_H_
