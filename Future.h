#ifndef _XTASK_FUTURE_H_
#define _XTASK_FUTURE_H_

#include <exception>
#include <memory>
#include "return_type.h"
#include "ThreadPool.h"

namespace xtask{
    template <class Type>
    struct FutureBase{
        FutureBase()
            :m_data(),
             m_done(false),
             m_exception(nullptr),
             m_then(nullptr){}
        ~FutureBase() = default;

        Type m_data;
        bool m_done;
        std::exception_ptr m_exception;
        std::function<void()> m_then;
    };

    template <class Type>
    class Future{
    public:
        Future(std::shared_ptr<FutureBase<Type>> fut);
        Future(const Future &);
        Future(Future &&);
        ~Future();

        Future &operator=(const Future &);
        Future &operator=(Future &&);

        Type get();
        void wait();

        template <class Function>
        auto then(Function &&function) -> Future<return_type<Function>>{
            m_future->m_then = []()->void{
                
            };
        }
    protected:
    private:
        std::shared_ptr<FutureBase<Type>> m_future;
    };
}

#endif //_XTASK_FUTURE_H_