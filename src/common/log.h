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
#include "config.h"

namespace zrpc {


    extern zrpc::Config::ptr zRpcConfig;

#define DebugLog \
	if (zrpc::OpenLog() && zrpc::LogLevel::DEBUG >= zrpc::zRpcConfig->m_log_level) \
		zrpc::LogWarp(zrpc::LogEvent::ptr(new zrpc::LogEvent(zrpc::LogLevel::DEBUG, \
        __FILE__, __LINE__, __func__, zrpc::LogType::RPC_LOG))).getStringStream()

#define InfoLog \
	if (zrpc::OpenLog() && zrpc::LogLevel::INFO >= zrpc::zRpcConfig->m_log_level) \
		zrpc::LogWarp(zrpc::LogEvent::ptr(new zrpc::LogEvent(zrpc::LogLevel::INFO, \
        __FILE__, __LINE__, __func__, zrpc::LogType::RPC_LOG))).getStringStream()

#define WarnLog \
	if (zrpc::OpenLog() && zrpc::LogLevel::WARN >= zrpc::gRpcConfig->m_log_level) \
		zrpc::LogWarp(zrpc::LogEvent::ptr(new zrpc::LogEvent(zrpc::LogLevel::WARN, \
        __FILE__, __LINE__, __func__, zrpc::LogType::RPC_LOG))).getStringStream()

#define ErrorLog \
	if (zrpc::OpenLog() && zrpc::LogLevel::ERROR >= zrpc::zRpcConfig->m_log_level) \
		zrpc::LogWarp(zrpc::LogEvent::ptr(new zrpc::LogEvent(zrpc::LogLevel::ERROR, \
        __FILE__, __LINE__, __func__, zrpc::LogType::RPC_LOG))).getStringStream()

#define AppDebugLog \
	if (zrpc::OpenLog() && zrpc::LogLevel::DEBUG >= zrpc::zRpcConfig->m_log_level) \
		zrpc::LogWarp(zrpc::LogEvent::ptr(new zrpc::LogEvent(zrpc::LogLevel::DEBUG, \
        __FILE__, __LINE__, __func__, zrpc::LogType::APP_LOG))).getStringStream()

#define AppInfoLog \
	if (zrpc::OpenLog() && zrpc::LogLevel::INFO >= zrpc::zRpcConfig->m_log_level) \
		zrpc::LogWarp(zrpc::LogEvent::ptr(new zrpc::LogEvent(zrpc::LogLevel::INFO, \
        __FILE__, __LINE__, __func__, zrpc::LogType::APP_LOG))).getStringStream()

#define AppWarnLog \
	if (zrpc::OpenLog() && zrpc::LogLevel::WARN >= zrpc::zRpcConfig->m_log_level) \
		zrpc::LogWarp(zrpc::LogEvent::ptr(new zrpc::LogEvent(zrpc::LogLevel::WARN, \
        __FILE__, __LINE__, __func__, zrpc::LogType::APP_LOG))).getStringStream()

#define AppErrorLog \
	if (zrpc::OpenLog() && zrpc::LogLevel::ERROR >= zrpc::zRpcConfig->m_log_level) \
		zrpc::LogWarp(zrpc::LogEvent::ptr(new zrpc::LogEvent(zrpc::LogLevel::ERROR, \
        __FILE__, __LINE__, __func__, zrpc::LogType::APP_LOG))).getStringStream()


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
     * @brief 是否打开日志
     * @return
     */
    bool OpenLog();

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
        LogEvent(LogLevel level, const char* file_name, int line, const char* func_name, LogType type);

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
        const char*       m_file_name;   // 文件名
        int               m_line = 0;    // 行号
        const char*       m_func_name;   //
        LogType           m_type;        // 日志类型
        std::string       m_msg_no;      //
        std::stringstream m_ss;          // stream 流
    };

    /**
     * @brief 日志包装类，在析构时写入日志
     */
    class LogWarp {
    public:
        LogWarp(LogEvent::ptr event);
        ~LogWarp();
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
        AsyncLogger(const char* file_name, const char* file_path, int max_size, LogType type);

        /**
         * @brief 析构函数
         */
        ~AsyncLogger();

        /**
         * @brief 向任务队列中加入任务
         * @param buffer 待写入的日志集合
         */
        void push(std::vector<std::string>& buffer);

        /**
         * @brief 将缓冲区的数据刷写到文件
         */
        void flush();

        /**
         * @brief 执行异步写入日志
         * @param arg 要写入的参数
         * @return
         */
        static void* execute(void* arg);

        /**
         * @brief 停止异步写日志
         */
        void stop();

    public:
        // 日志写入的任务队列，vector保存要写入的日志
        std::queue<std::vector<std::string>> m_tasks;

    private:
        const char*   m_file_name;               // 写入文件名
        const char*   m_file_path;               // 日志写入路径
        int            m_max_size = 0;            // 写入单个文件最大大小
        LogType        m_log_type;                // 日志类型
        int            m_no;                      // 写入文件的下标
        bool           m_need_reopen = false;     // 是否需要重新打开文件
        FILE*          m_file_handle = nullptr;   // 文件句柄
        std::string    m_date;                    // 日期

        MutexType      m_mutex;                   // 互斥量
        pthread_cond_t m_condition;               // 线程初始化条件
        bool           m_stop = false;            // 异步写入是否停止

    public:
        pthread_t m_thread;
        sem_t     m_semaphore;
    };

    class Logger {
    public:
        typedef std::shared_ptr<Logger> ptr;
        typedef Mutex MutexType;

        Logger();

        ~Logger();

        void init(const char* file_name, const char* file_path, int max_size, int sync_inteval);

        // log();
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
        std::vector<std::string> m_rpc_buffer;     // 保存RPC日志
        std::vector<std::string> m_app_buffer;     // 保存用户日志

    private:
        MutexType        m_app_buff_mutex;
        MutexType        m_rpc_buff_mutex;
        bool             m_is_init = false;    // 是否初始化
        AsyncLogger::ptr m_async_rpc_logger;   // 异步写入rpc日志的AsyncLogger实例指针
        AsyncLogger::ptr m_async_app_logger;   // 异步写入app日志的AsyncLogger实例指针

        int              m_sync_inteval = 0;
    };
}



#endif //Z_RPC_LOG_H
