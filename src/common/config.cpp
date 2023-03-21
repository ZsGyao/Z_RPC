/**
  ******************************************************************************
  * @file           : config.cpp
  * @author         : zgys
  * @brief          : None
  * @attention      : None
  * @date           : 23-3-7
  ******************************************************************************
  */

#include "src/common/config.h"
#include "src/common/log.h"
#include "src/network/tcp/tcp_server.h"
#include "src/network/net_address.h"
#include <iostream>
#include <algorithm>

namespace zrpc {

    zrpc::Logger::ptr zRpcLogger;
    zrpc::TcpServer::ptr zRpcServer;

    Config::Config(const char* file_path) : m_file_path(file_path) {
       // std::cout << "**** Debug [config.cpp " << __LINE__ << "] Config construct ****" << std::endl;

        m_xml_file = new tinyxml2::XMLDocument();
        tinyxml2::XMLError rt = m_xml_file->LoadFile(file_path); // 成功返回 0
        if(rt) {
            std::cout << "start Z_RPC server error! read config file [" << file_path
                      << "]  error info: [" << m_xml_file->ErrorStr()
                      << "]  error id: [" << m_xml_file->ErrorID()
                      << "]  error row and column: [" << m_xml_file->ErrorLineNum() << "]" << std::endl;
            exit(0);
        }
        readConf();
    }

    Config::~Config() {
      //  std::cout << "**** Debug [config.cpp " << __LINE__ << "] ~Config destroy ****" << std::endl;
        delete m_xml_file;
    }

    void Config::readConf() {
      //  std::cout << "**** Debug [config.cpp "<< __LINE__ <<"] readConf ****" << std::endl;

        // log 并创建 Logger
        tinyxml2::XMLElement* root = m_xml_file->RootElement();
        tinyxml2::XMLElement* log_node = root->FirstChildElement("log");
        if(!log_node) {
            std::cout << "start Z_PRC error! read config file " << m_file_path
                      << ", cannot read [log] xml node" << std::endl;
            exit(0);
        }
        readLogConfig(log_node);

        /* 获取配置中的配置后创建一个日志器，用来写入日志，包括 RPC_LOG 和 APP_LOG */
        zRpcLogger = std::make_shared<Logger>();
        zRpcLogger->init(m_log_prefix, m_log_path, m_log_max_file_size, m_log_sync_interval);

        // coroutine
        tinyxml2::XMLElement* coroutine_node = root->FirstChildElement("coroutine");
        if(!coroutine_node) {
            std::cout << "start Z_PRC error! read config file " << m_file_path
                      << ", cannot read [coroutine] xml node" << std::endl;
            exit(0);
        }
        readCoroutineConfig(coroutine_node);

        // time_wheel
        tinyxml2::XMLElement* time_wheel_node = root->FirstChildElement("time_wheel");
        if(!time_wheel_node) {
            std::cout << "start Z_PRC error! read config file " << m_file_path
                      << ", cannot read [time_wheel] xml node" << std::endl;
            exit(0);
        }
        readTimeWheelConfig(time_wheel_node);

        // msg_req_len
        // max_connect_timeout
        // io_thread_num
        readOtherConfig(root);

        // TODO server
        tinyxml2::XMLElement* server_node = root->FirstChildElement("server");
        if(!server_node) {
            std::cout << "start Z_PRC error! read config file " << m_file_path
                      << ", cannot read [server] xml node" << std::endl;
            exit(0);
        }
        readServerConfig(server_node);

        // TODO database

    }

    void Config::readServerConfig(tinyxml2::XMLElement* server_node) {
      //  std::cout << "**** Debug [config.cpp " << __LINE__ << "] readServerConfig ****" << std::endl;

        // ip
        tinyxml2::XMLElement* node = server_node->FirstChildElement("ip");
        if(!node || !node->GetText()) {
            std::cout << "start Z_PRC error! read config file " << m_file_path
                      << ", cannot read [server.ip] xml node" << std::endl;
            exit(0);
        }
        m_server_ip = std::string(node->GetText());
        if(m_server_ip.empty()) {
            m_server_ip = "0.0.0.0";
        }

        // port
        node = server_node->FirstChildElement("port");
        if(!node || !node->GetText()) {
            std::cout << "start Z_PRC error! read config file " << m_file_path
                      << ", cannot read [server.port] xml node" << std::endl;
            exit(0);
        }
        m_server_ip = std::atoi(node->GetText());

        // protocal
        node = server_node->FirstChildElement("protocal");
        if(!node || !node->GetText()) {
            std::cout << "start Z_PRC error! read config file " << m_file_path
                      << ", cannot read [server.protocal] xml node" << std::endl;
            exit(0);
        }
        m_server_protocal = std::string(node->GetText());
        std::transform(m_server_protocal.begin(), m_server_protocal.end(), m_server_protocal.begin(), ::toupper);

        // TODO 根据protocal选择是HTTP协议还是TCP协议
        zrpc::IPAddress::ptr addr = std::make_shared<zrpc::IPAddress>(m_server_ip, m_server_port);
        if (m_server_protocal == "HTTP") {
            zRpcServer = std::make_shared<TcpServer>(addr, Http_Protocal);
        } else if (m_server_protocal == "zRpcPb") {
            zRpcServer = std::make_shared<TcpServer>(addr, zRpcPb_Protocal);
        } else {
            ErrorLog << "Unkonw protocal, check [server.protocal] xml node!";
        }
        char buff[512];
        sprintf(buff, "read config from file [%s]: [log_path: %s], [log_prefix: %s], [log_max_size: %d MB], [log_level: %s], "
                      "[coroutine_stack_size: %d KB], [coroutine_pool_size: %d], "
                      "[msg_req_len: %d], [max_connect_timeout: %d s], "
                      "[iothread_num:%d], [timewheel_bucket_num: %d], [timewheel_inteval: %d s], [server_ip: %s], [server_Port: %d], [server_protocal: %s]\n",
                m_file_path.c_str(), m_log_path.c_str(), m_log_prefix.c_str(), m_log_max_file_size / 1024 / 1024,
                levelToString(m_rpc_log_level).c_str(), m_cor_stack_size, m_cor_pool_size, m_msg_req_len,
                m_max_connect_timeout, m_io_thread_num, m_time_wheel_bucket_num, m_time_wheel_interval, m_server_ip.c_str(), m_server_port, m_server_protocal.c_str());

        std::string s(buff);
        InfoLog << s;
    }

    void Config::readCoroutineConfig(tinyxml2::XMLElement* coroutine_node) {
      //  std::cout << "**** Debug [config.cpp "<< __LINE__ <<"] readCoroutineConfig ****" << std::endl;

        // coroutine_stack_size
        tinyxml2::XMLElement* node = coroutine_node->FirstChildElement("coroutine_stack_size");
        if(!node || !node->GetText()) {
            std::cout << "start Z_PRC error! read config file " << m_file_path
                      << ", cannot read [coroutine_stack_size] xml node" << std::endl;
            exit(0);
        }
        m_cor_stack_size = 1024 * std::atoi(node->GetText()); // K

        // coroutine_pool_size
        node = coroutine_node->FirstChildElement("coroutine_pool_size");
        if(!node || !node->GetText()) {
            std::cout << "start Z_PRC error! read config file " << m_file_path
                      << ", cannot read [coroutine_pool_size] xml node" << std::endl;
            exit(0);
        }
        m_cor_pool_size = std::atoi(node->GetText());
    }

    void Config::readDBConfig(tinyxml2::XMLElement* node) {

    }

    void Config::readTimeWheelConfig(tinyxml2::XMLElement* time_wheel_node) {
      //  std::cout << "**** Debug [config.cpp "<< __LINE__ <<"] readTimeWheelConfig ****" << std::endl;

        // bucket_num
        tinyxml2::XMLElement* node = time_wheel_node->FirstChildElement("bucket_num");
        if(!node || !node->GetText()) {
            std::cout << "start Z_PRC error! read config file " << m_file_path
                      << ", cannot read [bucket_num] xml node" << std::endl;
            exit(0);
        }
        m_time_wheel_bucket_num = std::atoi(node->GetText());

        // interval
        node = time_wheel_node->FirstChildElement("interval");
        if(!node || !node->GetText()) {
            std::cout << "start Z_PRC error! read config file " << m_file_path
                      << ", cannot read [interval] xml node" << std::endl;
            exit(0);
        }
        m_time_wheel_interval = std::atoi(node->GetText());
    }

    void Config::readOtherConfig(tinyxml2::XMLElement* other_node) {
     //   std::cout << "**** Debug [config.cpp "<< __LINE__ <<"] readOtherConfig ****" << std::endl;

        // msg_req_len
        tinyxml2::XMLElement* msg_req_len_node = other_node->FirstChildElement("msg_req_len");
        if(!msg_req_len_node || !msg_req_len_node->GetText()) {
            std::cout << "start Z_PRC error! read config file " << m_file_path
                      << ", cannot read [msg_req_len] xml node" << std::endl;
            exit(0);
        }
        m_msg_req_len = std::atoi(msg_req_len_node->GetText());

        // max_connect_timeout
        tinyxml2::XMLElement* max_connect_timeout_node = other_node->FirstChildElement("max_connect_timeout");
        if(!max_connect_timeout_node || !max_connect_timeout_node->GetText()) {
            std::cout << "start Z_PRC error! read config file " << m_file_path
                      << ", cannot read [max_connect_timeout] xml node" << std::endl;
            exit(0);
        }
        m_max_connect_timeout = std::atoi(max_connect_timeout_node->GetText());

        // io_thread_num
        tinyxml2::XMLElement* io_thread_num_node = other_node->FirstChildElement("io_thread_num");
        if(!io_thread_num_node || !io_thread_num_node->GetText()) {
            std::cout << "start Z_PRC error! read config file " << m_file_path
                      << ", cannot read [io_thread_num] xml node" << std::endl;
            exit(0);
        }
        m_io_thread_num = std::atoi(io_thread_num_node->GetText());
    }

    void Config::readLogConfig(tinyxml2::XMLElement* log_node) {
     //   std::cout << "**** Debug [config.cpp " << __LINE__ << "] readLogConfig ****" << std::endl;
        // log_path
        tinyxml2::XMLElement* node = log_node->FirstChildElement("log_path");
        if(!node || !node->GetText()) {
            std::cout << "start Z_PRC error! read config file " << m_file_path
                      << ", cannot read [log_path] xml node" << std::endl;
            exit(0);
        }
        m_log_path = std::string(node->GetText());

        // log_prefix
        node = log_node->FirstChildElement("log_prefix");
        if(!node || !node->GetText()) {
            std::cout << "start Z_PRC error! read config file " << m_file_path
                      << ", cannot read [log_prefix] xml node" << std::endl;
            exit(0);
        }
        m_log_prefix = std::string(node->GetText());

        // log_max_file_size
        node = log_node->FirstChildElement("log_max_file_size");
        if(!node || !node->GetText()) {
            std::cout << "start Z_PRC error! read config file " << m_file_path
                      << ", cannot read [log_max_file_size] xml node" << std::endl;
            exit(0);
        }
        m_log_max_file_size = std::atoi(node->GetText()) * 1024 * 1024; // M

        // rpc_log_level
        node = log_node->FirstChildElement("rpc_log_level");
        if(!node || !node->GetText()) {
            std::cout << "start Z_PRC error! read config file " << m_file_path
                      << ", cannot read [rpc_log_level] xml node" << std::endl;
            exit(0);
        }
        std::string log_level = std::string(node->GetText());
        m_rpc_log_level = stringToLevel(log_level);

        // app_log_level
        node = log_node->FirstChildElement("app_log_level");
        if(!node || !node->GetText()) {
            std::cout << "start Z_PRC error! read config file " << m_file_path
                      << ", cannot read [app_log_level] xml node" << std::endl;
            exit(0);
        }
        log_level = std::string(node->GetText());
        m_app_log_level = stringToLevel(log_level);

        // log_sync_interval
        node = log_node->FirstChildElement("log_sync_interval");
        if(!node || !node->GetText()) {
            std::cout << "start Z_PRC error! read config file " << m_file_path
                      << ", cannot read [log_sync_interval] xml node" << std::endl;
            exit(0);
        }
        m_log_sync_interval = std::atoi(node->GetText());

    }

    tinyxml2::XMLElement* Config::getXmlNode(const std::string& name) {
        return m_xml_file->RootElement()->FirstChildElement(name.c_str());
    }

    std::shared_ptr<Config> zRpcConfig =  zrpc::Singleton_Conf::GetInstance("/home/zgys/workspace/zRPC/config/zrpc_server.xml");
}