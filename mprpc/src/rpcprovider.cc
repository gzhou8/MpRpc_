#include "rpcprovider.h"
#include "rpcheader.pb.h"
#include "logger.h"
#include "zookeeperutil.h"

// 框架提供给外部使用的，可以发布/注册rpc方法的函数接口
void RpcProvider::NotifyService(::google::protobuf::Service *service)
{
    ServiceInfo service_info;
    // 获取服务对象的描述信息
    const google::protobuf::ServiceDescriptor *pserviceDesc = service->GetDescriptor();
    // 获取服务名字
    std::string service_name = pserviceDesc->name();
    // 获取对象的方法数量
    int methodCnt = pserviceDesc->method_count();

    // std::cout << "service_name : " << service_name << std::endl;
    LOG_INFO("service_name : %s", service_name.c_str());

    for (int i = 0; i < methodCnt; ++i)
    {
        const google::protobuf::MethodDescriptor *pmethodDesc = pserviceDesc->method(i);
        std::string method_name = pmethodDesc->name();
        service_info.m_methodMap.insert({method_name, pmethodDesc});
        // std::cout << "method_name : " << method_name << std::endl;
        LOG_INFO("method_name : %s", method_name.c_str());
    }
    service_info.m_service = service;
    m_serviceMap.insert({service_name, service_info});
}

// 启动rpc服务节点
void RpcProvider::run()
{
    std::string ip = MprpcApplication::getInstance().getConfig().Load("rpcserverip");
    uint16_t port = atoi(MprpcApplication::getInstance().getConfig().Load("rpcserverport").c_str());
    muduo::net::InetAddress address(ip, port);

    // 创建TcpServer对象
    muduo::net::TcpServer server(&m_eventLoop, address, "RpcProvider");

    // 创建连接回调和消息读写回调方法
    server.setConnectionCallback(std::bind(&RpcProvider::OnConnection, this, std::placeholders::_1));
    server.setMessageCallback(std::bind(&RpcProvider::OnMessage, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));

    // 设置线程数量
    server.setThreadNum(4);

    // 把rpc节点上的服务注册到zookeeper
    zkClient zkCli;
    zkCli.Start();

    for (auto &sp : m_serviceMap)
    {
        std::string service_path = "/" + sp.first;
        zkCli.Create(service_path.c_str(), nullptr, 0);
        for (auto &mp : sp.second.m_methodMap)
        {
            std::string method_path = service_path + "/" + mp.first;
            char method_path_data[128] = {0};
            sprintf(method_path_data, "%s:%d", ip.c_str(), port);
            zkCli.Create(method_path.c_str(),method_path_data,strlen(method_path_data),ZOO_EPHEMERAL);
        }
    }

    // std::cout << "Rpc service start at ip:" << ip << " port:" << port << std::endl;
    LOG_INFO("Rpc service start at ip : %s port : %d", ip.c_str(), port);

    //**，启动！
    server.start();
    m_eventLoop.loop();
}

void RpcProvider::OnConnection(const muduo::net::TcpConnectionPtr &conn)
{
    if (!conn->connected())
    {
        conn->shutdown();
    }
}

void RpcProvider::OnMessage(const muduo::net::TcpConnectionPtr &conn,
                            muduo::net::Buffer *buffer,
                            muduo::Timestamp receiveTime)
{
    // 接收所有字节流
    std::string recv_buf = buffer->retrieveAllAsString();

    // header_size(4) + {service_name + method_name} + args

    uint32_t header_size;
    // 取头前4字节，读取服务头部长度
    recv_buf.copy((char *)&header_size, 4, 0);
    // 读取服务头部
    std::string rpc_header_str = recv_buf.substr(4, header_size);
    mprpc::RpcHeader rpcHeader;
    std::string service_name;
    std::string method_name;
    uint32_t args_size;
    if (rpcHeader.ParseFromString(rpc_header_str))
    {
        service_name = rpcHeader.service_name();
        method_name = rpcHeader.method_name();
        args_size = rpcHeader.args_size();
    }
    else
    {
        // std::cout << "rpc_header_str : " << rpc_header_str << " parse error" << std::endl;
        LOG_ERROR("rpc_header_str : %s parse_error", rpc_header_str.c_str());
        return;
    }
    // 读取参数
    std::string args_str = recv_buf.substr(4 + header_size, args_size);

    // // 打印调试信息
    // std::cout << "==================================================" << std::endl;
    // std::cout << "header_size : " << header_size << std::endl;
    // std::cout << "rpc_header_str : " << rpc_header_str << std::endl;
    // std::cout << "service_name : " << service_name << std::endl;
    // std::cout << "method_name : " << method_name << std::endl;
    // std::cout << "args_str : " << args_str << std::endl;
    // std::cout << "arg_size: " << args_size << std::endl;
    // std::cout << "==================================================" << std::endl;

    // 获取service和method对象

    auto it = m_serviceMap.find(service_name);
    if (it == m_serviceMap.end())
    {
        std::cout << service_name << "is not exist!" << std::endl;
        return;
    }

    auto mit = it->second.m_methodMap.find(method_name);
    if (mit == it->second.m_methodMap.end())
    {
        std::cout << service_name << " : " << method_name << "is not exist!" << std::endl;
        return;
    }
    google::protobuf::Service *service = it->second.m_service;
    const google::protobuf::MethodDescriptor *method = mit->second;

    // 生成rpc方法的请求request和响应response
    google::protobuf::Message *request = service->GetRequestPrototype(method).New(); // 根据method动态生成request
    if (!request->ParseFromString(args_str))                                         // 将args_str反序列化后填入request
    {
        std::cout << "request parse error, content : " << args_str << std::endl;
    }
    google::protobuf::Message *response = service->GetResponsePrototype(method).New();

    // 给下面的method方法绑定一个Closure回调函数
    google::protobuf::Closure *done = google::protobuf::NewCallback<RpcProvider,
                                                                    const muduo::net::TcpConnectionPtr &,
                                                                    google::protobuf::Message *>(this, &RpcProvider::SendRpcResponse, conn, response);

    // 在框架上，根据远端rpc请求，调用当前rpc节点上发布的方法
    service->CallMethod(method, nullptr, request, response, done);
}

// Closure的回调操作，用于序列化响应和网络发送
void RpcProvider::SendRpcResponse(const muduo::net::TcpConnectionPtr &conn, google::protobuf::Message *response)
{
    std::string response_str;
    if (response->SerializeToString(&response_str))
    {
        // 序列化成功后把结果发给调用方
        std::cout << "response : " << response_str.c_str() << std::endl;
        conn->send(response_str);
    }
    else
    {
        std::cout << "serialize response_str error!" << std::endl;
    }
    conn->shutdown(); // 模拟tcp短连接，主动关闭连接
}