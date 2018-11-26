#ifndef _XTASK_WHEN_H_
#define _XTASK_WHEN_H_

#include <memory>
#include <initializer_list>
#include "Future.h"

namespace xtask{
    template <class Type>
    class when_all{
    public:
        template <class Iterator>
        when_all(Iterator beg,Iterator ed);
        when_all(std::initializer_list<Future<Type>> list);

        Future<void> get_future()const noexcept{
            
        }
    protected:
    private:
        std::shared_ptr<FutureBase<void>> m_future;
    };
}

#endif //_XTASK_WHEN_H_
