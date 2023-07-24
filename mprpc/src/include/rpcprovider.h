#pragma once
#include "google/protobuf/service.h"
#include <muduo/net/TcpServer.h>
#include <muduo/net/InetAddress.h>
#include <muduo/net/EventLoop.h>
#include <muduo/net/TcpConnection.h>
#include <string>
#include <functional>
#include "mprpcapplication.h"
#include <google/protobuf/descriptor.h>
#include <unordered_map>

// 服务提供类，发布rpc服务的网络对象类
class RpcProvider
{
public:
    void NotifyService(::google::protobuf::Service *service);

    void run();

private:
    muduo::net::EventLoop m_eventLoop;

    struct ServiceInfo
    {
        google::protobuf::Service *m_service;                                                     // 保存服务对象
        std::unordered_map<std::string, const google::protobuf::MethodDescriptor *> m_methodMap; // 保存服务方法
    };
    //存储注册成功的服务对象和其服务方法的所有信息
    std::unordered_map<std::string, ServiceInfo> m_serviceMap;

    // 连接回调函数
    void OnConnection(const muduo::net::TcpConnectionPtr &);
    // 读写回调函数
    void OnMessage(const muduo::net::TcpConnectionPtr &conn,
                   muduo::net::Buffer *buffer,
                   muduo::Timestamp receiveTime);
    //Closure的回调函数,用于序列化rpc的响应和网络发送
    void SendRpcResponse(const muduo::net::TcpConnectionPtr &, google::protobuf::Message*);
};