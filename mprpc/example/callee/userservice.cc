#include <iostream>
#include <string>
#include "user.pb.h"
#include "mprpcapplication.h"
#include "rpcprovider.h"
#include "logger.h"

class UserService : public fixbug::UserServiceRpc
{
public:
    bool Login(std::string name, std::string pwd)
    {
        std::cout << "doing Login:" << std::endl;
        std::cout << "name : " << name << "pwd : " << pwd << std::endl;
        return true;
    }
    bool Register(uint32_t id, std::string name, std::string pwd)
    {
        std::cout << "doing Register:" << std::endl;
        std::cout << "id : " << id << "name : " << name << "pwd : " << pwd << std::endl;
        return true;
    }
    // 重写的rpc远程调用方法
    void Login(::google::protobuf::RpcController *controller,
               const ::fixbug::LoginRequest *request,
               ::fixbug::LoginResponse *response,
               ::google::protobuf::Closure *done)
    {
        // 应用提取数据
        std::string name = request->name();
        std::string pwd = request->pwd();

        // 处理本地业务
        bool login_result = Login(name, pwd);

        // 填写响应消息
        fixbug::ResultCode *code = response->mutable_result();
        code->set_errcode(0);
        code->set_errmsg("");
        response->set_success(login_result);

        // 执行回调，序列化和网络发送
        done->Run();
    }
    void Register(::google::protobuf::RpcController *controller,
                  const ::fixbug::RegisterRequest *request,
                  ::fixbug::RegisterResponse *response,
                  ::google::protobuf::Closure *done)
    {
        uint32_t id = request->id();
        std::string name = request->name();
        std::string pwd = request->pwd();

        bool register_result = Register(id, name, pwd);

        // 填写响应消息
        fixbug::ResultCode *code = response->mutable_result();
        code->set_errcode(0);
        code->set_errmsg("");
        response->set_success(register_result);

        // 执行回调，序列化和网络发送
        done->Run();
    }
};

int main(int argc, char **argv)
{
    LOG_INFO("first log message!");
    LOG_ERROR("%s-%s-%d", __FILE__, __FUNCTION__, __LINE__);

    MprpcApplication::Init(argc, argv);

    RpcProvider provider;
    provider.NotifyService(new UserService());

    provider.run();

    return 0;
}