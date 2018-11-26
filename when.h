#ifndef _XTASK_WHEN_H_
#define _XTASK_WHEN_H_

#include <memory>
#include <iterator>
#include <initializer_list>
#include <atomic>
#include "Future.h"

namespace xtask{
    template<class Iterator>
    auto when_all(Iterator begin,Iterator end) -> Future<void> {
        using FutureType = typename std::iterator_traits<Iterator>::value_type;
        using DifferenceType = typename std::iterator_traits<Iterator>::difference_type;

        auto future_ptr = std::make_shared<FutureBase<void>>();
        auto counter_ptr = std::make_shared<std::atomic<DifferenceType>>(end - begin + 1);

        future_ptr->m_status = Status::running;

        for(auto itr = begin;itr != end; ++itr){
            itr->then([future_ptr,counter_ptr](FutureType fut)->void{
                auto val = counter_ptr->fetch_sub(1);
                if(val <= 2){
                    future_ptr->m_status = Status::done;
                    if(future_ptr->m_then){
                        switch(future_ptr->m_then_policy){
                            case Policy::pool:
                                ThreadPool::instance().addTask(future_ptr->m_then);
                                break;
                            case Policy::thread:
                                std::async(std::launch::async,future_ptr->m_then);
                                break;
                            case Policy::synchronized:
                                future_ptr->m_then();
                                break;
                        }
                    }
                }
            });
        }

        return Future<void>(future_ptr);
    }

    template <class Iterator>
    auto when_any(Iterator begin,Iterator end) -> typename std::iterator_traits<Iterator>::value_type{
        using FutureType = typename std::iterator_traits<Iterator>::value_type;
        using DifferenceType = typename std::iterator_traits<Iterator>::difference_type;

        auto future_ptr = std::make_shared<typename FutureType::value_type>();



        return FutureType(future_ptr);
    }
}

#endif //_XTASK_WHEN_H_
