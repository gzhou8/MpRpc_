#include "zookeeperutil.h"
#include "mprpcapplication.h"

void global_watcher(zhandle_t *zh, int type,
                    int state, const char *path, void *watcherCtx)
{
    if (type == ZOO_SESSION_EVENT)
    {
        if (state == ZOO_CONNECTED_STATE)
        {
            sem_t *sem = (sem_t *)zoo_get_context(zh);
            sem_post(sem);
        }
    }
}

zkClient::zkClient() : m_zhandle(nullptr)
{
}
zkClient::~zkClient()
{
    if (m_zhandle != nullptr)
    {
        zookeeper_close(m_zhandle);
    }
}
void zkClient::Start()
{
    std::string host = MprpcApplication::getInstance().getConfig().Load("zookeeperip");
    std::string port = MprpcApplication::getInstance().getConfig().Load("zookeeperport");
    std::string connstr = host + ":" + port;

    m_zhandle = zookeeper_init(connstr.c_str(), global_watcher, 30000, nullptr, nullptr, 0);

    if (nullptr == m_zhandle)
    {
        std::cout << "zookeeper init failed!" << std::endl;
        exit(EXIT_FAILURE);
    }

    sem_t sem;
    sem_init(&sem, 0, 0);
    zoo_set_context(m_zhandle, &sem);

    sem_wait(&sem);
    std::cout << "zookeeper init success!" << std::endl;
}
void zkClient::Create(const char *path, const char *data,
                      int datalen, int state)
{
    char path_buffer[128];
    int bufferlen = sizeof(path_buffer);
    int flag;
    flag = zoo_exists(m_zhandle, path, 0, nullptr);
    if (ZNONODE == flag)
    {
        flag = zoo_create(m_zhandle, path, data, datalen, &ZOO_OPEN_ACL_UNSAFE, state, path_buffer, bufferlen);
        if (flag == ZOK)
        {
            std::cout << "ZNODE create success, path : " << path << std::endl;
        }
        else
        {
            std::cout << "flag : " << flag << std::endl;
            std::cout << "ZNODE create error, path : " << path << std::endl;
            exit(EXIT_FAILURE);
        }
    }
}
std::string zkClient::GetData(const char *path)
{
    char buffer[64];
    int bufferlen = sizeof(buffer);
    int flag = zoo_get(m_zhandle, path, 0, buffer, &bufferlen, nullptr);
    if (flag != ZOK)
    {
        std::cout << "get ZNODE error, path : " << path << std::endl;
        return "";
    }
    else
    {
        return buffer;
    }
}