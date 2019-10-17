#ifndef XTASK_UTILITY_H
#define XTASK_UTILITY_H

#include "Future.h"

namespace xtask{
    template <class Function>
    auto async(Function &&f)
        -> typename std::enable_if<!std::is_void<return_type_t<Function>>::value,Future<return_type_t<Function>>>::type{
        using ReturnType = return_type_t<Function>;
        auto fb = std::make_shared<detail::FutureBase<ReturnType>>();
        ThreadPool::instance().addTask([func = std::forward<Function>(f),fut = fb]()noexcept->void{
            try{
                fut->emplace(func());
            }catch(...){
                fut->set_exception(std::current_exception());
            }
        });
        return Future<ReturnType>(fb);
    }
    template <class Function>
    auto async(Function &&f)
        -> typename std::enable_if<std::is_void<return_type_t<Function>>::value,Future<return_type_t<Function>>>::type{
        using ReturnType = return_type_t<Function>;
        auto fb = std::make_shared<detail::FutureBase<ReturnType>>();
        ThreadPool::instance().addTask([func = std::forward<Function>(f),fut = fb]()noexcept->void{
            try{
                func();
                fut->emplace();
            }catch(...){
                fut->set_exception(std::current_exception());
            }
        });
        return Future<ReturnType>(fb);
    }
}

#endif
