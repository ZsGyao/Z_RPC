/**
  ******************************************************************************
  * @file           : abstract_codec.h
  * @author         : zgys
  * @brief          : None
  * @attention      : None
  * @date           : 23-3-18
  ******************************************************************************
  */

#ifndef ZRPC_ABSTRACT_CODEC_H
#define ZRPC_ABSTRACT_CODEC_H

#include <string>
#include <memory>
#include "src/network/tcp/tcp_buffer.h"
#include "src/network/abstract_data.h"

namespace zrpc {

    enum ProtocalType {
        zRpcPb_Protocal = 1,
        Http_Protocal   = 2
    };

    /* 抽象编解码器 */
    class AbstractCodeC {
    public:
        typedef std::shared_ptr<AbstractCodeC> ptr;

        AbstractCodeC() = default;

        virtual ~AbstractCodeC() {}

        /* virtual encode method, describe how to encode protocal  */
        virtual void encode(TcpBuffer* buf, AbstractData* data) = 0;
        /* virtual decode method, describe how to decode protocal  */
        virtual void decode(TcpBuffer* buf, AbstractData* data) = 0;

        virtual ProtocalType getProtocalType() = 0;
    };
}

#endif //ZRPC_ABSTRACT_CODEC_H
