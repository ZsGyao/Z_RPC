//
// Created by zgys on 23-3-17.
//

#ifndef ZRPC_ABSTRACT_SLOT_H
#define ZRPC_ABSTRACT_SLOT_H

#include <memory>
#include <functional>

namespace zrpc {

    template<class T>
    class AbstractSlot {
    public:
        typedef std::shared_ptr<AbstractSlot> ptr;
        typedef std::weak_ptr<T> weakPtr;
        typedef std::shared_ptr<T> sharedPtr;

        AbstractSlot(weakPtr ptr, std::function<void(sharedPtr)> cb)
                : m_weak_ptr(ptr), m_cb(cb) {
        }

        ~AbstractSlot() {
            sharedPtr ptr = m_weak_ptr.lock();
            if (ptr) {
                m_cb(ptr);
            }
        }

    private:
        weakPtr m_weak_ptr;
        std::function<void(sharedPtr)> m_cb;
    };

}

#endif //ZRPC_ABSTRACT_SLOT_H
