/**
  ******************************************************************************
  * @file           : config.cpp
  * @author         : zgys
  * @brief          : None
  * @attention      : None
  * @date           : 23-3-7
  ******************************************************************************
  */

#include "config.h"
#include "log.h"
#include <iostream>

namespace zrpc {

    extern zrpc::Logger::ptr zRpcLogger;
   // extern zrpc::TcpServer::ptr gRpcServer;

    Config::Config(const char* file_path) : m_file_path(file_path) {
        m_xml_file = new tinyxml2::XMLDocument();
        bool rt = m_xml_file->LoadFile(file_path);
        if(!rt) {
            std::cout << "start Z_RPC server error! read config file [" << file_path
                      << "]  error info: [" << m_xml_file->ErrorStr()
                      << "]  error id: [" << m_xml_file->ErrorID()
                      << "]  error row and column: [" << m_xml_file->ErrorLineNum() << "]" << std::endl;
            exit(0);
        }
    }

    Config::~Config() {

    }

    void Config::readConf() {
        tinyxml2::XMLElement* root = m_xml_file->RootElement();
        tinyxml2::XMLElement* log_node = root->FirstChildElement("log");
        if(!log_node) {
            std::cout << "start Z_PRC error! read config file " << m_file_path
                      << ", cannot read [log] xml node" << std::endl;
            exit(0);
        }
        readLogConfig(log_node);

        tinyxml2::XMLElement* time_wheel_node = root->FirstChildElement("time_wheel");
        if(!time_wheel_node) {
            std::cout << "start Z_PRC error! read config file " << m_file_path
                      << ", cannot read [time_wheel] xml node" << std::endl;
            exit(0);
        }

        tinyxml2::XMLElement* coroutine_node = root->FirstChildElement("coroutine");
        if(!coroutine_node) {
            std::cout << "start Z_PRC error! read config file " << m_file_path
                      << ", cannot read [coroutine] xml node" << std::endl;
            exit(0);
        }
    }

    void Config::readCoroutineConfig(tinyxml2::XMLElement* coroutine_node) {

    }

    void Config::readDBConfig(tinyxml2::XMLElement* node) {

    }

    void Config::readLogConfig(tinyxml2::XMLElement* log_node) {
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
        m_log_max_size = std::atoi(node->GetText());
        m_log_max_size = m_log_max_size * 1024 * 1024; // M

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

        // log_sync_inteval
        node = log_node->FirstChildElement("log_sync_interval");
        if(!node || !node->GetText()) {
            std::cout << "start Z_PRC error! read config file " << m_file_path
                      << ", cannot read [log_sync_interval] xml node" << std::endl;
            exit(0);
        }
        m_log_sync_interval = std::atoi(node->GetText());

        zRpcLogger = std::make_shared<Logger>();
        zRpcLogger->init(m_log_prefix.c_str(), m_log_path.c_str(), m_log_max_size, m_log_sync_interval);
    }

    tinyxml2::XMLElement* Config::getXmlNode(const std::string& name) {

    }
}