/**
  ******************************************************************************
  * @file           : zrpc_pb_codec.cpp
  * @author         : zgys
  * @brief          : None
  * @attention      : None
  * @date           : 23-3-20
  ******************************************************************************
  */

#include "src/network/zRpcPb/zrpc_pb_codec.h"
#include "src/common/log.h"
#include "src/network/byte.hpp"
#include "src/common/msg_req.h"

namespace zrpc {
    static const char PB_START   = 0x02;      // start char
    static const char PB_END     = 0x03;      // end char
    static const int MSG_REQ_LEN = 20;        // default length of msg_req

    void ZRpcPbCodeC::encode(TcpBuffer* buf, AbstractData* data) {
        if (!buf || !data) {
            ErrorLog << "encode error! buf or data nullptr";
            return;
        }
        DebugLog << "ZRpcPbCodeC::encode start";
        auto tmp = dynamic_cast<ZRpcPbStruct*>(data);

        int len = 0;
        const char* re = encodePbData(tmp, len);
        if (re == nullptr || len == 0 || !tmp->encode_success) {
            ErrorLog << "encode error";
            data->encode_success = false;
            return;
        }
        DebugLog << "encode package len = " << len;
        if (buf) {
            buf->writeToBuffer(re, len);
            DebugLog << "success encode and write to buffer, write_index=" << buf->writeIndex();
        }
        data = tmp;
        if (re) {
            free((void*)re);
            re = nullptr;
        }
        DebugLog << "ZRpcPbCodeC::encode end";
    }

    void ZRpcPbCodeC::decode(TcpBuffer* buf, AbstractData* data) {
        if (!buf || !data) {
            ErrorLog << "decode error! buf or data is nullptr";
            return;
        }

        std::vector<char> tmp = buf->getBufferVector();
        // int total_size = buf->readAble();
        int start_index = buf->readIndex();
        int end_index = -1;
        int32_t pk_len= -1;

        bool parse_full_pack = false;

        for (int i = start_index; i < buf->writeIndex(); ++i) {
            // first find start
            if (tmp[i] == PB_START) {
                if (i + 1 < buf->writeIndex()) {
                    pk_len = getInt32FromNetByte(&tmp[i + 1]);
                    DebugLog << "parse pk_len =" << pk_len;
                    int j = i + pk_len - 1;
                    DebugLog << "j =" << j << ", i =" << i;

                    if (j >= buf->writeIndex()) {
                        DebugLog << "receive package not complete, or pk_start find error, continue next parse";
                        continue;
                    }
                    if (tmp[j] == PB_END) {
                        start_index = i;
                        end_index = j;
                        DebugLog << "parse success, start index in tcp_buffer is " << i << ", end index is " << j;
                        parse_full_pack = true;
                        break;
                    }
                }
            }
        }
        if (!parse_full_pack) {
            WarnLog << "not parse full package, return";
            return;
        }

        buf->recycleRead(end_index + 1 - start_index);
        DebugLog << "m_read_buffer size=" << buf->getBufferVector().size() << ", read_index=" << buf->readIndex() << ", write_index=" << buf->writeIndex();

        auto pb_struct = dynamic_cast<ZRpcPbStruct*>(data);
        pb_struct->pk_len = pk_len;
        pb_struct->decode_success = false;

        int msg_req_len_index = start_index + sizeof(char) + sizeof(int32_t);
        if (msg_req_len_index >= end_index) {
            ErrorLog << "parse error, msg_req_len_index[" << msg_req_len_index << "] >= end_index[" << end_index << "]";
            // drop this error package
            return;
        }

        pb_struct->msg_req_len = getInt32FromNetByte(&tmp[msg_req_len_index]);
        if (pb_struct->msg_req_len == 0) {
            ErrorLog << "parse error, msg_req empty";
            return;
        }

        DebugLog << "msg_req_len= " << pb_struct->msg_req_len;
        int msg_req_index = msg_req_len_index + sizeof(int32_t);
        DebugLog << "msg_req_len_index= " << msg_req_index;

        char msg_req[50] = {0};

        memcpy(&msg_req[0], &tmp[msg_req_index], pb_struct->msg_req_len);
        pb_struct->msg_req = std::string(msg_req);
        DebugLog << "msg_req= " << pb_struct->msg_req;

        int service_name_len_index = msg_req_index + pb_struct->msg_req_len;
        if (service_name_len_index >= end_index) {
            ErrorLog << "parse error, service_name_len_index[" << service_name_len_index << "] >= end_index[" << end_index << "]";
            // drop this error package
            return;
        }

        DebugLog << "service_name_len_index = " << service_name_len_index;
        int service_name_index = service_name_len_index + sizeof(int32_t);

        if (service_name_index >= end_index) {
            ErrorLog << "parse error, service_name_index[" << service_name_index << "] >= end_index[" << end_index << "]";
            return;
        }

        pb_struct->service_name_len = getInt32FromNetByte(&tmp[service_name_len_index]);

        if (pb_struct->service_name_len > pk_len) {
            ErrorLog << "parse error, service_name_len[" << pb_struct->service_name_len << "] >= pk_len [" << pk_len << "]";
            return;
        }
        DebugLog << "service_name_len = " << pb_struct->service_name_len;

        char service_name[512] = {0};

        memcpy(&service_name[0], &tmp[service_name_index], pb_struct->service_name_len);
        pb_struct->service_full_name = std::string(service_name);
        DebugLog << "service_name = " << pb_struct->service_full_name;

        int err_code_index = service_name_index + pb_struct->service_name_len;
        pb_struct->err_code = getInt32FromNetByte(&tmp[err_code_index]);

        int err_info_len_index = err_code_index + sizeof(int32_t);

        if (err_info_len_index >= end_index) {
            ErrorLog << "parse error, err_info_len_index[" << err_info_len_index << "] >= end_index[" << end_index << "]";
            // drop this error package
            return;
        }
        pb_struct->err_info_len = getInt32FromNetByte(&tmp[err_info_len_index]);
        DebugLog << "err_info_len = " << pb_struct->err_info_len;
        int err_info_index = err_info_len_index + sizeof(int32_t);

        char err_info[512] = {0};

        memcpy(&err_info[0], &tmp[err_info_index], pb_struct->err_info_len);
        pb_struct->err_info = std::string(err_info);

        int pb_data_len = pb_struct->pk_len
                          - pb_struct->service_name_len - pb_struct->msg_req_len - pb_struct->err_info_len
                          - 2 * sizeof(char) - 6 * sizeof(int32_t);

        int pb_data_index = err_info_index + pb_struct->err_info_len;
        DebugLog << "pb_data_len= " << pb_data_len << ", pb_index = " << pb_data_index;

        if (pb_data_index >= end_index) {
            ErrorLog << "parse error, pb_data_index[" << pb_data_index << "] >= end_index[" << end_index << "]";
            return;
        }
        // DebugLog << "pb_data_index = " << pb_data_index << ", pb_data.length = " << pb_data_len;

        std::string pb_data_str(&tmp[pb_data_index], pb_data_len);
        pb_struct->pb_data = pb_data_str;

        // DebugLog << "decode succ,  pk_len = " << pk_len << ", service_name = " << pb_struct->service_full_name;

        pb_struct->decode_success = true;
        data = pb_struct;
    }

    ProtocalType ZRpcPbCodeC::getProtocalType() {
        return zRpcPb_Protocal;
    }

    const char* ZRpcPbCodeC::encodePbData(ZRpcPbStruct* data, int& len) {
        if (data->service_full_name.empty()) {
            ErrorLog << "parse error, service_full_name is empty";
            data->encode_success = false;
            return nullptr;
        }
        if (data->msg_req.empty()) {
            data->msg_req = MsgReqUtil::genMsgNumber();
            data->msg_req_len = data->msg_req.length();
            DebugLog << "generate msg_number = " << data->msg_req;
        }

        int32_t pk_len = 2 * sizeof(char) + 6 * sizeof(int32_t)
                         + data->pb_data.length() + data->service_full_name.length()
                         + data->msg_req.length() + data->err_info.length();

        DebugLog << "encode pk_len = " << pk_len;
        char *buf = reinterpret_cast<char *>(malloc(pk_len));
        char *tmp = buf;
        *tmp = PB_START;
        tmp++;

        int32_t pk_len_net = htonl(pk_len);
        memcpy(tmp, &pk_len_net, sizeof(int32_t));
        tmp += sizeof(int32_t);

        int32_t msg_req_len = data->msg_req.length();
        DebugLog << "msg_req_len= " << msg_req_len;
        int32_t msg_req_len_net = htonl(msg_req_len);
        memcpy(tmp, &msg_req_len_net, sizeof(int32_t));
        tmp += sizeof(int32_t);

        if (msg_req_len != 0) {
            memcpy(tmp, &(data->msg_req[0]), msg_req_len);
            tmp += msg_req_len;
        }

        int32_t service_full_name_len = data->service_full_name.length();
        DebugLog << "src service_full_name_len = " << service_full_name_len;
        int32_t service_full_name_len_net = htonl(service_full_name_len);
        memcpy(tmp, &service_full_name_len_net, sizeof(int32_t));
        tmp += sizeof(int32_t);

        if (service_full_name_len != 0) {
            memcpy(tmp, &(data->service_full_name[0]), service_full_name_len);
            tmp += service_full_name_len;
        }

        int32_t err_code = data->err_code;
        DebugLog << "err_code= " << err_code;
        int32_t err_code_net = htonl(err_code);
        memcpy(tmp, &err_code_net, sizeof(int32_t));
        tmp += sizeof(int32_t);

        int32_t err_info_len = data->err_info.length();
        DebugLog << "err_info_len= " << err_info_len;
        int32_t err_info_len_net = htonl(err_info_len);
        memcpy(tmp, &err_info_len_net, sizeof(int32_t));
        tmp += sizeof(int32_t);

        if (err_info_len != 0) {
            memcpy(tmp, &(data->err_info[0]), err_info_len);
            tmp += err_info_len;
        }

        memcpy(tmp, &(data->pb_data[0]), data->pb_data.length());
        tmp += data->pb_data.length();
        DebugLog << "pb_data_len= " << data->pb_data.length();

        int32_t checksum = 1;
        int32_t checksum_net = htonl(checksum);
        memcpy(tmp, &checksum_net, sizeof(int32_t));
        tmp += sizeof(int32_t);

        *tmp = PB_END;

        data->pk_len = pk_len;
        data->msg_req_len = msg_req_len;
        data->service_name_len = service_full_name_len;
        data->err_info_len = err_info_len;

        // checksum has not been implemented yet, directly skip checksum
        data->check_num = checksum;
        data->encode_success = true;

        len = pk_len;

        return buf;
    }
}