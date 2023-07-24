#include "logger.h"
#include <time.h>
#include <iostream>

// 单例模式实例化
Logger &Logger::getInstance()
{
    static Logger logger;
    return logger;
}

Logger::Logger()
{
    // 启动写日志线程
    std::thread writeLogTask([&]()
                             {
        for(;;)
        {
            time_t now = time(nullptr);
            tm *nowtm = localtime(&now);

            char file_name[128];
            sprintf(file_name,"%d-%d-%d-log.txt",nowtm->tm_yday+1900, nowtm->tm_mon+1, nowtm->tm_mday);

            FILE *fp = fopen(file_name,"a+");
            if(fp == nullptr)
            {
                std::cout << "file open error!" << std::endl;
                exit(EXIT_FAILURE);
            }

            std::string msg = m_lckque.pop();
            char time_buf[128];
            sprintf(time_buf,"%d-%d-%d => [%s]",nowtm->tm_hour,
                                                nowtm->tm_min,
                                                nowtm->tm_sec,
                                                (m_loglevel == INFO ? INFO : ERROR));
            msg.insert(0,time_buf);
            msg.append("\n");

            fputs(msg.c_str(),fp);
            fclose(fp);

        } });
    // 分离线程作为守护线程在后台运行
    writeLogTask.detach();
}

// 设置日志级别
void Logger::set_loglevel(LogLevel level)
{
    m_loglevel = level;
}
// 写入日志信息
void Logger::Log(std::string msg)
{
    m_lckque.push(msg);
}
