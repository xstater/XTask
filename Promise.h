#ifndef _XTASK_PROMISE_H_
#define _XTASK_PROMISE_H_

#include <utility>
#include <exception>
#include <memory>
#include "Future.h"

namespace xtask{
    template <class Type>
    class Promise{
    public:
        Promise() = default;
        Promise(const Promise &) = delete;
        Promise(Promise &&) = default;
        ~Promise() = default;

        Promise &operator=(const Promise &) = delete;
        Promise &operator=(Promise &&) = default;

        void set(const Type &data);
        void set(std::exception_ptr exception);

        Future<Type> get_future();
    protected:
    private:
        std::shared_ptr<FutureBase<Type>> m_future;
    };
}

#endif //_XTASK_PROMISE_H_