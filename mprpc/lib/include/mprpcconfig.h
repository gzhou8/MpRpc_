#pragma once

#include <unordered_map>
#include <string>


//rpcserverip rpcserverport zookeeperip zookeeperport

class MprpcConfig
{
public:
    //读取配置文件
    void LoadConfigFile(const char* config_file);
    //查询配置项
    std::string Load(const std::string &key);
private:
    std::unordered_map<std::string,std::string> m_configMap;

    void Trim(std::string &read_buf);
};