/**
  ******************************************************************************
  * @file           : config.h
  * @author         : zgys
  * @brief          : None
  * @attention      : None
  * @date           : 23-3-7
  ******************************************************************************
  */


#ifndef Z_RPC_CONFIG_H
#define Z_RPC_CONFIG_H

#include <string>
#include <memory>
#include <map>
#include <tinyxml2.h>
#include "src/common/singleton.hpp"

namespace zrpc {

    /**
    * @brief 日志级别枚举类
    */
    enum LogLevel {
        DEBUG = 1,   // debug 信息
        INFO  = 2,   // 一般 信息
        WARN  = 3,   // 警告 信息
        ERROR = 4,   // 错误 信息
        NONE  = 5    // 不打印日志
    };

    /**
     * @brief 配置类，将配置类作为单例，成员变量保存配置内容
     */
    class Config {
    public:
        typedef std::shared_ptr<Config> ptr;

    public:
        /**
         * @brief 构造
         * @param file_path config路径
         */
        explicit Config(const char* file_path);

        ~Config();

        /**
         * @brief 解析配置
         */
        void readConf();

        /**
         * @brief 获取配置项 name
         * @param name 配置项名
         * @return 配置项节点
         */
        tinyxml2::XMLElement* getXmlNode(const std::string& name);

    private:
        /**
         * @brief 读数据库配置
         * @param[in,out] node 传入xml节点，返回解析完的数据库配置
         */
        void readDBConfig(tinyxml2::XMLElement* node);

        /**
         * @brief 读Log配置
         * @param[in,out] log_node 传入xml节点，返回解析完的log配置
         */
        void readLogConfig(tinyxml2::XMLElement* log_node);

        /**
         * @brief 读coroutine配置
         * @param[in,out] coroutine_node 传入xml节点，返回解析完的coroutine配置
         */
        void readCoroutineConfig(tinyxml2::XMLElement* coroutine_node);

        /**
         * @brief 读time_wheel配置
         * @param[in,out] time_wheel_node 传入xml节点，返回解析完的time_wheel配置
         */
        void readTimeWheelConfig(tinyxml2::XMLElement* time_wheel_node);

        /**
        * @brief 读server配置
        * @param[in,out] server_node 传入xml节点，返回解析完的server配置
        */
        void readServerConfig(tinyxml2::XMLElement* server_node);

        /**
        * @brief 读自己就是第一标签的配置
        * @param[in,out] other_node 传入xml节点，返回解析完的配置
        */
        void readOtherConfig(tinyxml2::XMLElement* other_node);

    public:
        // log parameters
        std::string m_log_path;                              // 日志存储路径
        std::string m_log_prefix;                            // 日志文件名
        int         m_log_max_file_size = 0;                 // 日志单个文件最大大小
        LogLevel    m_rpc_log_level     = LogLevel::DEBUG;   // rpc日志级别
        LogLevel    m_app_log_level     = LogLevel::DEBUG;   // app日志级别
        int         m_log_sync_interval = 500;               // 异步写入间隔

        // coroutine parameters
        int m_cor_stack_size        = 0;    // 协程栈大小
        int m_cor_pool_size         = 0;    // 协程池大小

        // other
        int m_msg_req_len           = 0;    // 请求信息长度
        int m_max_connect_timeout   = 0;    // ms
        int m_io_thread_num         = 0;    // io线程数

        // time wheel
        int m_time_wheel_bucket_num = 0;    // 时间轮的bucket数
        int m_time_wheel_interval   = 0;    // 时间轮更新时间

        // TODO server
        std::string m_server_ip;           // 网络ip
        uint16_t    m_server_port = 0;     // 网络端口
        std::string m_server_protocal;     // 协议类型 HTTP or TCP

        // TODO database


    private:
        std::string            m_file_path;  // xml文件路径
        tinyxml2::XMLDocument* m_xml_file;   // xml文件
    };

    typedef zrpc::Singleton<Config> Singleton_Conf;
    /* 一个全局变量，在头文件中用extern 声明 cpp中实现 。 多次声明，一次实现 */
    extern std::shared_ptr<Config> zRpcConfig;
}

#endif //Z_RPC_CONFIG_H
