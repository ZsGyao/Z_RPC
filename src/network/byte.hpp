/**
  ******************************************************************************
  * @file           : byte.hpp
  * @author         : zgys
  * @brief          : None
  * @attention      : None
  * @date           : 23-3-13
  ******************************************************************************
  */


#ifndef Z_RPC_BYTE_HPP
#define Z_RPC_BYTE_HPP

#include <stdint.h>
#include <string.h>
#include <arpa/inet.h>

namespace zrpc {
    int32_t getInt32FromNetByte(const char* buf) {
        int32_t tmp;
        memcpy(&tmp, buf, sizeof(tmp));
        /* 将一个无符号长整形数从网络字节顺序转换为主机字节顺序 */
        return ntohl(tmp);
    }
}

#endif //Z_RPC_BYTE_HPP
