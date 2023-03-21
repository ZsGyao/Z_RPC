/**
  ******************************************************************************
  * @file           : net_address.h
  * @author         : zgys
  * @brief          : None
  * @attention      : None
  * @date           : 23-3-13
  ******************************************************************************
  */


#ifndef Z_RPC_NET_ADDRESS_H
#define Z_RPC_NET_ADDRESS_H

#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <sys/un.h>
#include <string>
#include <memory>

namespace zrpc {

    class NetAddress {
    public:
        typedef std::shared_ptr<NetAddress> ptr;

        NetAddress() = default;

        virtual ~NetAddress() = default;
        virtual sockaddr* getSockAddr() = 0;
        virtual int getFamily() const = 0;
        virtual std::string toString() const = 0;
        virtual socklen_t getSockLen() const = 0;
    };


    class IPAddress : public NetAddress {
    public:
        IPAddress(const std::string& ip, uint16_t port);

        explicit IPAddress(const std::string& addr);

        explicit IPAddress(uint16_t port);

        explicit IPAddress(sockaddr_in addr);

        sockaddr* getSockAddr() override;

        int getFamily() const override;

        socklen_t getSockLen() const override;

        std::string toString() const override;

        std::string getIP() const {
            return m_ip;
        }

        int getPort() const {
            return m_port;
        }

    public:
        static bool CheckValidIPAddr(const std::string& addr);

    private:

        std::string m_ip;
        uint16_t m_port;
        sockaddr_in m_addr;

    };

    class UnixDomainAddress : public NetAddress {
    public:
        explicit UnixDomainAddress(std::string& path);

        explicit UnixDomainAddress(sockaddr_un addr);

        sockaddr* getSockAddr() override;

        int getFamily() const override;

        socklen_t getSockLen() const override;

        std::string getPath() const {
            return m_path;
        }

        std::string toString() const override;


    private:

        std::string m_path;
        sockaddr_un m_addr;

    };
}

#endif //Z_RPC_NET_ADDRESS_H
