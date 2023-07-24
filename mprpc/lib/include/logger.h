#pragma once
#include "lockqueue.h"

enum LogLevel
{
    INFO, // 普通信息
    ERROR // 错误信息
};

class Logger
{
public:
    // 单例模式实例化
    static Logger &getInstance();
    // 设置日志级别
    void set_loglevel(LogLevel level);
    // 写入日志信息
    void Log(std::string msg);

private:
    int m_loglevel;
    LockQueue<std::string> m_lckque;
    Logger();
    Logger(const Logger &) = delete;
    Logger(Logger &&) = delete;
};

// 定义宏

#define LOG_INFO(logmsgformat, ...)                  \
    do                                               \
    {                                                \
        Logger &logger = Logger::getInstance();      \
        logger.set_loglevel(INFO);                   \
        char c[1024] = {0};                          \
        snprintf(c, 1024, logmsgformat, ##__VA_ARGS__); \
        logger.Log(c);                               \
    } while (0);

#define LOG_ERROR(logmsgformat, ...)                \
    do                                               \
    {                                                \
        Logger &logger = Logger::getInstance();      \
        logger.set_loglevel(ERROR);                  \
        char c[1024] = {0};                          \
        snprintf(c, 1024, logmsgformat, ##__VA_ARGS__); \
        logger.Log(c);                               \
    } while (0);