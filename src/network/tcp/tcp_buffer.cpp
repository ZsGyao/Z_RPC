/**
  ******************************************************************************
  * @file           : tcp_buffer.cpp
  * @author         : zgys
  * @brief          : None
  * @attention      : None
  * @date           : 23-3-17
  ******************************************************************************
  */

#include "src/network/tcp/tcp_buffer.h"
#include "src/common/log.h"

namespace zrpc {

    TcpBuffer::TcpBuffer(int size) {
        m_buffer.reserve(size);
    }

    int TcpBuffer::readAble() const {
        return m_write_index - m_read_index;
    }

    int TcpBuffer::writeAble() const {
        return m_size - m_write_index;
    }

    int TcpBuffer::readIndex() const {
        return m_read_index;
    }

    int TcpBuffer::writeIndex() const {
        return m_write_index;
    }

    void TcpBuffer::writeToBuffer(const char* buf, int size) {
        if (size > writeAble()) {
            int new_size = (int)(1.5 * (m_write_index + size));
            resizeBuffer(new_size);
        }
        memcpy(&m_buffer[m_write_index], buf, size);
        m_write_index += size;
    }

    void TcpBuffer::readFromBuffer(std::vector<char>& re, int size) {
        if (readAble() == 0) {
            DebugLog << "read buffer empty!";
            return;
        }
        int read_size = readAble() > size ? size : readAble();
        std::vector<char> tmp(read_size);

        memcpy(&tmp[0], &m_buffer[m_read_index], read_size);
        re.swap(tmp);
        m_read_index += read_size;
        adjustBuffer();
    }

    void TcpBuffer::resizeBuffer(int size) {
        std::vector<char> tmp(size);
        int min = std::min(size, readAble());
        memcpy(&tmp[0], &m_buffer[m_read_index], min);

        m_buffer.swap(tmp);
        m_read_index = 0;
        m_write_index = m_read_index + min;
    }

    void TcpBuffer::clearBuffer() {
        m_buffer.clear();
        m_read_index  = 0;
        m_write_index = 0;
    }

    int TcpBuffer::getSize() const {
        return (int)m_buffer.size();
    }

    std::vector<char> TcpBuffer::getBufferVector() const {
        return m_buffer;
    }

    std::string TcpBuffer::getBufferString() {
        std::string re(readAble(), '0');
        memcpy(&re[0],  &m_buffer[m_read_index], readAble());
        return re;
    }

    void TcpBuffer::recycleRead(int index) {
        int j = m_read_index + index;
        if (j > (int)m_buffer.size()) {
            ErrorLog << "recycleRead error";
            return;
        }
        m_read_index = j;
        adjustBuffer();
    }

    void TcpBuffer::recycleWrite(int index) {
        int j = m_write_index + index;
        if (j > (int)m_buffer.size()) {
            ErrorLog << "recycleWrite error";
            return;
        }
        m_write_index = j;
        adjustBuffer();
    }

    void TcpBuffer::adjustBuffer() {
        if (m_read_index > static_cast<int>(m_buffer.size() / 3)) {
            std::vector<char> new_buffer(m_buffer.size());
            int count = readAble();

            memcpy(&new_buffer[0], &m_buffer[m_read_index], count);

            m_buffer.swap(new_buffer);
            m_write_index = count;
            m_read_index = 0;
            new_buffer.clear();
        }
    }

}