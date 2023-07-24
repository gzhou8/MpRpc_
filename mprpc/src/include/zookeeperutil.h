#pragma once

#include <zookeeper/zookeeper.h>
#include <semaphore.h>
#include <string>

class zkClient
{
public:
    zkClient();
    ~zkClient();
    void Start();
    void Create(const char *path, const char *data,
                int datalen, int state = 0);
    std::string GetData(const char *path);
private:
    zhandle_t *m_zhandle;
};