#include <iostream>
#include "mprpcapplication.h"
#include "user.pb.h"
#include "mprpcchannel.h"

int main(int argc, char **argv)
{
    MprpcApplication::Init(argc, argv);

    // 演示调用远程发布的方法Login
    fixbug::UserServiceRpc_Stub stub(new MprpcChannel());

    fixbug::LoginRequest request;
    request.set_name("zhang san");
    request.set_pwd("123456");

    fixbug::LoginResponse response;
    MprpcController controller;
    stub.Login(&controller, &request, &response, nullptr);
    // RpcChannel->RpcChannel::callMethod(),集中来做所有rpc方法的序列化和网络发送

    if (0 == response.result().errcode())
    {
        std::cout << "rpc login response success : " << response.success() << std::endl;
    }
    else
    {
        std::cout << "rpc login response error : " << response.result().errmsg() << std::endl;
    }

    fixbug::RegisterRequest rst;
    fixbug::RegisterResponse rsp;
    rst.set_id(1);
    rst.set_name("mprpc");
    rst.set_pwd("666666");
    stub.Register(nullptr, &rst, &rsp, nullptr);

    if (controller.Failed())
    {
        std::cout << controller.ErrorText() << std::endl;
    }
    else
    {
        if (0 == rsp.result().errcode())
        {
            std::cout << "rpc register response success : " << rsp.success() << std::endl;
        }
        else
        {
            std::cout << "rpc register response error : " << rsp.result().errmsg() << std::endl;
        }
    }

    return 0;
}