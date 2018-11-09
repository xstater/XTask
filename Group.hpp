#ifndef _XTASK_GROUP_H_
#define _XTASK_GROUP_H_

#include <mutex>
#include <array>
#include <bitset>
#include "XSignal.hpp"

template<class TaskType,int N>
class Group{
public:
    Group();
    ~Group();
    
    void add(TaskType &&task);
    void run();
    
    Signal<void()> onComplete;
protected:
private:
    std::mutex m_mutex;
    std::bitset<N> m_bits;
    std::array<TaskType,N> m_tasks;
};

#endif //_XTASK_GROUP_H_
