/**
  ******************************************************************************
  * @file           : zRpc_Pb_codec.h
  * @author         : zgys
  * @brief          : None
  * @attention      : None
  * @date           : 23-3-19
  ******************************************************************************
  */

#ifndef ZRPC_ZRPC_PB_CODEC_H
#define ZRPC_ZRPC_PB_CODEC_H

#include "src/network/abstract_codec.h"
#include "src/network/tcp/tcp_buffer.h"
#include "src/network/zRpcPb/zrpc_pb_data.h"

namespace zrpc {
    class ZRpcPbCodeC : public AbstractCodeC {
    public:
        ZRpcPbCodeC() = default;

        ~ZRpcPbCodeC() = default;

        void encode(TcpBuffer* buf, AbstractData* data) override;

        void decode(TcpBuffer* buf, AbstractData* data) override;

        ProtocalType getProtocalType() override;

    private:
        const char* encodePbData(ZRpcPbStruct* data, int& len);
    };
}

#endif //ZRPC_ZRPC_PB_CODEC_H
