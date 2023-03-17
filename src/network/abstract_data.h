//
// Created by zgys on 23-3-17.
//

#ifndef ZRPC_ABSTRACT_DATA_H
#define ZRPC_ABSTRACT_DATA_H

namespace zrpc {
    class AbstractData {
    public:
        AbstractData() = default;
        virtual ~AbstractData() {};

        bool decode_success = false;
        bool encode_success = false;
    };
}

#endif //ZRPC_ABSTRACT_DATA_H
