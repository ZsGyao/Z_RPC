/**
  ******************************************************************************
  * @file           : log.h
  * @author         : zgys
  * @brief          : 日志类
  * @attention      : 参考sylar实现
  * @date           : 23-3-1
  ******************************************************************************
  */


#ifndef Z_RPC_LOG_H
#define Z_RPC_LOG_H

#include <memory>
#include <string>
#include <sstream>
#include <vector>
#include <queue>
#include <semaphore.h>
#include "src/network/mutex.hpp"

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

    enum LogType {
        RPC_LOG = 1, // RPC 系统日志
        APP_LOG = 2  // APP 用户使用日志
    };

    /**
     * @brief 将 level 的字符串转为 Level
     * @param[in] str level 字符串
     * @return 日志等级枚举
     */
    LogLevel stringToLevel(const std::string& str);

    /**
     * @brief 将 LogLevel 转为 string
     * @param[in] level 日志等级
     * @return string 日志等级
     */
    std::string levelToString(const LogLevel level);

    /**
     * @brief 日志事件类
     */
    class LogEvent {
    public:
        typedef std::shared_ptr<LogEvent> ptr;

        /**
         * @brief 构造函数
         * @param level 日志等级
         * @param file_name 文件名
         * @param line 行号
         * @param func_name 执行的函数名
         * @param type 日志类型
         */
        LogEvent(LogLevel level, const std::string& file_name, int line, const std::string& func_name, LogType type);

        /**
         * @brief 析构函数
         */
        ~LogEvent();

        /**
         * @brief 获取 stream 流
         * @return stream 流
         */
        std::stringstream& getStringStream();

        void log();

    private:
        timeval           m_timeval;     // 时间结构体，日志发生时间
        LogLevel          m_level;       // 日志等级
        pid_t             m_pid = 0;     // 进程 id
        pid_t             m_tid = 0;     // 线程 id
        uint64_t          m_cor_id = 0;  // 协程 id
        string            m_file_name;   // 文件名
        int               m_line = 0;    // 行号
        string            m_func_name;   //
        LogType           m_type;        // 日志类型
        std::string       m_msg_no;      //
        std::stringstream m_ss;          // stream 流
    };

    class LogTmp {
    public:
        LogTmp(LogEvent::ptr event);
        ~LogTmp();
        std::stringstream& getStringStream();

    private:
        LogEvent::ptr m_event;
    };

    /**
     * @brief 异步写日志器
     */
    class AsyncLogger {
    public:
        typedef std::shared_ptr<AsyncLogger> ptr;
        typedef zrpc::Mutex MutexType;

        /**
         * @brief 构造函数
         * @param file_name 写入文件名
         * @param file_path 写入文件路径
         * @param max_size 最大大小
         * @param type 日志类型
         */
        AsyncLogger(const std::string& file_name, const std::string& file_path, int max_size, LogType type);

        /**
         * @brief 析构函数
         */
        ~AsyncLogger();

        /**
         * @brief
         * @param buffer
         */
        void push(std::vector<std::string>& buffer);

        void flush();

        static void* excute(void*);

        void stop();

    public:
        std::queue<std::vector<std::string>> m_tasks; // 日志写入的任务队列

    private:
        const string   m_file_name;
        const string   m_file_path;
        int            m_max_size = 0;
        LogType        m_log_type;
        int            m_no;
        bool           m_need_reopen = false;
        FILE*          m_file_handle = nullptr;
        std::string    m_date;

        MutexType      m_mutex;
        pthread_cond_t m_condition;
        bool           m_stop = false;

    public:
        pthread_t m_thread;
        sem_t     m_semaphore;
    };

    class Logger {
    public:
        typedef std::shared_ptr<Logger> ptr;

        Logger();

        ~Logger();

        void init(std::string& file_name, std::string& file_path, int max_size, int sync_inteval);

        void log();
        void pushRpcLog(const std::string& log_msg);
        void pushAppLog(const std::string& log_msg);

        void loopFunc();

        void flush();

        void start();

        AsyncLogger::ptr getAsyncLogger() {
            return m_async_rpc_logger;
        }

        AsyncLogger::ptr getAsyncAppLogger() {
            return m_async_app_logger;
        }

    public:
        std::vector<std::string> m_buffer;
        std::vector<std::string> m_app_buffer;

    private:
        Mutex            m_app_buff_mutex;
        Mutex            m_buff_mutex;
        bool             m_is_init = false;
        AsyncLogger::ptr m_async_rpc_logger;
        AsyncLogger::ptr m_async_app_logger;

        int              m_sync_inteval = 0;
    };
}



#endif //Z_RPC_LOG_H
