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

    class Config {
    public:
        typedef std::shared_ptr<Config> ptr;

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

        tinyxml2::XMLElement* getXmlNode(const std::string& name);

    public:
        // log parameters
        std::string m_log_path;                              // 日志存储路径
        std::string m_log_prefix;                            // 日志文件名
        int         m_log_max_size      = 0;                 // 日志单个文件最大大小
        LogLevel    m_rpc_log_level     = LogLevel::DEBUG;   // rpc日志级别
        LogLevel    m_app_log_level     = LogLevel::DEBUG;   // app日志级别
        int         m_log_sync_interval = 500;               // 异步写入间隔

        // coroutine parameters
        int m_cor_stack_size       = 0;    // 协程栈大小
        int m_cor_pool_size        = 0;    // 协程池大小
        int m_msg_req_len          = 0;    // 请求信息长度
        int m_max_connect_timeout  = 0;    // ms
        int m_iothread_num         = 0;    // io线程数
        int m_timewheel_bucket_num = 0;    // 时间轮的bucket数
        int m_timewheel_inteval    = 0;    // 时间轮更新时间

    private:
        std::string            m_file_path;  // xml文件路径
        tinyxml2::XMLDocument* m_xml_file;   // xml文件
    };
}

#endif //Z_RPC_CONFIG_H
