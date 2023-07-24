#include "mprpcconfig.h"
#include <iostream>

// 读取配置文件
void MprpcConfig::LoadConfigFile(const char *config_file)
{
    FILE *pf = fopen(config_file, "r");
    if (nullptr == pf)
    {
        std::cout << config_file << "is not valid!" << std::endl;
        exit(EXIT_FAILURE);
    }

    while (!feof(pf))
    {
        char buf[512];
        fgets(buf, 512, pf);

        std::string src_buf(buf);

        // // 去掉前面多余空格
        // int idx = src_buf.find_first_not_of(' ');
        // if (idx != -1)
        // {
        //     src_buf.substr(idx, src_buf.size() - idx);
        // }
        // // 去掉后面多余空格
        // idx = src_buf.find_last_not_of(' ');
        // if (idx != -1)
        // {
        //     src_buf.substr(0, idx + 1);
        // }

        // 忽略 # 注释
        if (src_buf[0] == '#' || src_buf.empty())
        {
            continue;
        }
        // 解析配置文件
        int idx = src_buf.find('=');
        if (idx == -1)
        {
            continue;
        }
        std::string key;
        std::string value;
        key = src_buf.substr(0, idx);
        Trim(key);
        int endidx = src_buf.find('\n');
        value = src_buf.substr(idx + 1, endidx - idx - 1);
        Trim(value);
        m_configMap.insert({key, value});
    }
}
// 查询配置项
std::string MprpcConfig::Load(const std::string &key)
{
    auto it = m_configMap.find(key);
    if (it == m_configMap.end())
    {
        return "";
    }
    return it->second;
}

void MprpcConfig::Trim(std::string &read_buf)
{
    // 去掉前面多余空格
    int idx = read_buf.find_first_not_of(' ');
    if (idx != -1)
    {
        read_buf = read_buf.substr(idx, read_buf.size() - idx);
    }
    // 去掉后面多余空格
    idx = read_buf.find_last_not_of(' ');
    if (idx != -1)
    {
        read_buf = read_buf.substr(0, idx + 1);
    }
}