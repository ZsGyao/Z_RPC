/**
  ******************************************************************************
  * @file           : noncopyable.h
  * @author         : zgys
  * @brief          : 不可拷贝
  * @attention      : None
  * @date           : 23-3-1
  ******************************************************************************
  */


#ifndef Z_RPC_NONCOPYABLE_H
#define Z_RPC_NONCOPYABLE_H

namespace zrpc {
    /**
     * @brief 对象无法拷贝,赋值
     */
    class Noncopyable {
    public:
        /**
         * @brief 默认构造函数
         */
        Noncopyable() = default;

        /**
         * @brief 默认析构函数
         */
        ~Noncopyable() = default;

        /**
         * @brief 拷贝构造函数(禁用)
         */
        Noncopyable(const Noncopyable&) = delete;

        /**
         * @brief 赋值函数(禁用)
         */
        Noncopyable& operator=(const Noncopyable&) = delete;
    };

}

#endif //Z_RPC_NONCOPYABLE_H
