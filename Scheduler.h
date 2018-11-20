#ifndef _XTASK_SCHEDULER_H_
#define _XTASK_SCHEDULER_H_

#include "Task.h"

namespace xtask{
    class Scheduler{
    public:
        static Scheduler &instance(){
            static Scheduler scheduler;
            return scheduler;
        }



    protected:
    private:
        Scheduler() = default;
        ~Scheduler() = default;
    };
}

#endif //_XTASK_SCHEDULER_H_