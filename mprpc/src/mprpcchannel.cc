#include <string>
#include "rpcheader.pb.h"
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <unistd.h>
#include <errno.h>
#include "mprpcapplication.h"
#include "zookeeperutil.h"

void MprpcChannel::CallMethod(const google::protobuf::MethodDescriptor *method,
                              google::protobuf::RpcController *controller, const google::protobuf::Message *request,
                              google::protobuf::Message *response, google::protobuf::Closure *done)
{
    const google::protobuf::ServiceDescriptor *sd = method->service();
    std::string service_name = sd->name();
    std::string method_name = method->name();

    // std::cout << "im using callMethod!" << std::endl;

    int arg_size = 0;
    std::string args_str;
    if (request->SerializeToString(&args_str))
    {
        arg_size = args_str.size();
    }
    else
    {
        // std::cout << "request serialize failed!" << std::endl;
        controller->SetFailed("request serialize failed!");
        return;
    }

    // 定义请求头
    mprpc::RpcHeader rpcHeader;
    rpcHeader.set_service_name(service_name);
    rpcHeader.set_method_name(method_name);
    rpcHeader.set_args_size(arg_size);

    std::string rpc_header_str;
    uint32_t header_size = 0;
    if (rpcHeader.SerializeToString(&rpc_header_str))
    {
        std::cout << rpc_header_str.c_str() << std::endl;
        header_size = rpc_header_str.size();
    }
    else
    {
        // std::cout << "response header serialize failed!" << std::endl;
        controller->SetFailed("response header serialize failed!");
        return;
    }

    std::string send_rpc_str;
    send_rpc_str.insert(0, std::string((char *)&header_size, 4));
    send_rpc_str += rpc_header_str;
    send_rpc_str += args_str;

    // 打印调试信息
    std::cout << "============================================" << std::endl;
    std::cout << "header_size: " << header_size << std::endl;
    std::cout << "rpc_header_str: " << rpc_header_str << std::endl;
    std::cout << "service_name: " << service_name << std::endl;
    std::cout << "method_name: " << method_name << std::endl;
    std::cout << "args_str: " << args_str << std::endl;
    std::cout << "arg_size: " << arg_size << std::endl;
    std::cout << "============================================" << std::endl;

    // 使用tcp编程，完成发送
    int clientfd = socket(AF_INET, SOCK_STREAM, 0);
    if (-1 == clientfd)
    {
        // std::cout << "create socket error! errno : " << errno << std::endl;
        char errtxt[512] = {0};
        sprintf(errtxt, "create socket error! errno : %d", errno);
        controller->SetFailed(errtxt);
        return;
    }

    zkClient zkCli;
    zkCli.Start();

    std::string method_path = "/" + service_name + "/" + method_name;
    std::string host_data = zkCli.GetData(method_path.c_str());

    if (host_data == "")
    {
        controller->SetFailed(method_path + "is not exists!");
        return;
    }
    int idx = host_data.find(":");
    if (idx == -1)
    {
        controller->SetFailed(method_path + "address is invalid!");
        return;
    }
    std::string ip = host_data.substr(0, idx);
    uint16_t port = atoi(host_data.substr(idx + 1, host_data.size() - idx).c_str());

    struct sockaddr_in server_addr;
    // std::string ip = MprpcApplication::getInstance().getConfig().Load("rpcserverip");
    // uint16_t port = atoi(MprpcApplication::getInstance().getConfig().Load("rpcserverport").c_str());
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(port);
    server_addr.sin_addr.s_addr = inet_addr(ip.c_str());

    if (-1 == connect(clientfd, (struct sockaddr *)&server_addr, sizeof(server_addr)))
    {
        // std::cout << "connect error! errno : " << errno << std::endl;
        char errtxt[512] = {0};
        sprintf(errtxt, "connect error! errno : %d", errno);
        controller->SetFailed(errtxt);
        close(clientfd);
        return;
    }
    if (-1 == send(clientfd, send_rpc_str.c_str(), send_rpc_str.size(), 0))
    {
        // std::cout << "send error! errno : " << errno << std::endl;
        char errtxt[512] = {0};
        sprintf(errtxt, "send error! errno : %d", errno);
        controller->SetFailed(errtxt);
        close(clientfd);
        return;
    }
    char recv_buf[1024];
    int recv_size = 0;
    if (-1 == (recv_size = recv(clientfd, recv_buf, 1024, 0)))
    {
        // std::cout << "recv error! errno : " << errno << std::endl;
        char errtxt[512] = {0};
        sprintf(errtxt, "recv error! errno : %d", errno);
        controller->SetFailed(errtxt);
        close(clientfd);
        return;
    }

    // std::string response_str(recv_buf, recv_size);
    if (!response->ParseFromArray(recv_buf, recv_size))
    {
        // std::cout << "parse error, response_str : " << recv_buf << std::endl;
        char errtxt[512] = {0};
        sprintf(errtxt, "parse error! response_str : %s", recv_buf);
        controller->SetFailed(errtxt);
        close(clientfd);
        return;
    }

    close(clientfd);
}