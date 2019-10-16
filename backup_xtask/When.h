#ifndef _XTASK_WHEN_H_
#define _XTASK_WHEN_H_

#include "Task.h"

namespace xtask{
    template <class Iterator>
    Task<void> when_all(Iterator begin,Iterator end){
        return Task<void>([begin,end]()->void{
            for(auto itr = begin;itr != end; ++itr){
                itr->wait();
            }
        });
    }

    template <class Iterator>
    Task<void> when_any(Iterator begin,Iterator end){
        auto flag_ptr = std::make_shared<bool>(false);
        auto mutex_ptr = std::make_shared<std::mutex>();
        auto task_ptr = std::make_shared<TaskBase<void>>();
        for(auto itr = begin;itr != end;++itr){
            itr->then([task_ptr,flag_ptr,mutex_ptr]()->void{
                {
                    std::lock_guard<std::mutex> lock(*mutex_ptr);
                    if(*flag_ptr)return;
                    *flag_ptr = true;
                    task_ptr->status = Status::done;
                }
                if(task_ptr->then){
                    switch(task_ptr->then_policy){
                        case Policy::pool:
                            ThreadPool::instance().addTask(task_ptr->then,task_ptr->exception);
                            break;
                        case Policy::synchronized:
                            task_ptr->then(task_ptr->exception);
                            break;
                        case Policy::thread:
                            std::thread t(task_ptr->then,task_ptr->exception);
                            t.detach();
                            break;
                    }
                }

            },Policy::synchronized);
        }
        return Task<void>(task_ptr);
    }
}

#endif //_XTASK_WHEN_H_
