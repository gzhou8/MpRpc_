#pragma once

#include "mprpcconfig.h"
#include "mprpcchannel.h"
#include "mprpccontorller.h"


//框架的基础类，用于初始化
class MprpcApplication
{
public:
    static void Init(int argc, char **argv);
    static MprpcApplication& getInstance();
    static MprpcConfig& getConfig();
private:
    static MprpcConfig m_config;
    MprpcApplication(){}
    MprpcApplication(const MprpcApplication&) = delete;
    MprpcApplication(MprpcApplication&&) = delete;
};