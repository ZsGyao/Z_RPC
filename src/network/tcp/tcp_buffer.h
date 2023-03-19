/**
  ******************************************************************************
  * @file           : tcp_buffer.h
  * @author         : zgys
  * @brief          : None
  * @attention      : None
  * @date           : 23-3-17
  ******************************************************************************
  */

#ifndef ZRPC_TCP_BUFFER_H
#define ZRPC_TCP_BUFFER_H

#include <memory>
#include <vector>

namespace zrpc {

    class TcpBuffer {
    public:
        typedef std::shared_ptr<TcpBuffer> ptr;

        explicit TcpBuffer(int size);

        ~TcpBuffer() = default;

        int readAble() const;

        int writeAble() const;

        int readIndex() const;

        int writeIndex() const;

        void writeToBuffer(const char* buf, int size);

        void readFromBuffer(std::vector<char>& re, int size);

        /**
         * @brief 将buffer中已读的删除，保留未读的，重设大小
         * @param size
         */
        void resizeBuffer(int size);

        void clearBuffer();

        int getSize() const;

        std::vector<char> getBufferVector() const;

        std::string getBufferString();

        void recycleRead(int index);

        void recycleWrite(int index);

        void adjustBuffer();

    private:

        int m_read_index  = 0;
        int m_write_index = 0;
        int m_size        = 0;

    public:
        std::vector<char> m_buffer;

    };
}

#endif //ZRPC_TCP_BUFFER_H
