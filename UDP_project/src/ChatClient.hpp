#pragma once
#include<iostream>
#include<stdio.h>
#include<sys/socket.h>
#include<arpa/inet.h>
#include<sys/types.h>

#include<unistd.h>
#include<string>

#include"LogSvr.hpp"

#define UDPPORT 17777
#define TCPPORT 17778

class ChatClient
{
  public:
    ChatClient(std::string SvrIp = "127.0.0.1")
    {
      UdpSock_ = -1;
      UdpPort_ = UDPPORT;

      TcpSock_ = -1;
      TcpPort_ = TCPPORT;

      SvrIp_ = SvrIp;
    }
    ~ChatClient()
    {
        if(UdpSock_ > 0)
        {
          close(UdpSock_);
        }
    }

    void Init()
    {

      UdpSock_ = socket(AF_INET,SOCK_DGRAM,IPPROTO_UDP);
      if(UdpSock_ < 0)
      {
        LOG(ERROR,"client create udp socket failed")<<std::endl;
        exit(1);
      }
      TcpSock_ = socket(AF_INET,SOCK_STREAM,IPPROTO_TCP);
      if(TcpSock_ < 0)
      {
        LOG(ERROR,"client create tcp socket failed")<<std::endl;
        exit(2);
      }

    }
    
    bool Connect2Server()
    {
      struct sockaddr_in peeraddr;
      peeraddr.sin_family = AF_INET;
      peeraddr.sin_port = htons(TcpPort_);
      peeraddr.sin_addr.s_addr = inet_addr(SvrIp_.c_str());

      int ret = connect(TcpSock_,(struct sockaddr*)&peeraddr,sizeof(peeraddr));
      if(ret < 0)
      {
        LOG(ERROR,"Connect Server failed")<<SvrIp_<<":"<<TcpPort_<<std::endl;
        return false;
      }
      return true;
    }

    bool Login();
    bool Register();
    bool SendMsg();
    bool RecvMsg();


  private:
    //发送正常的业务数据
    int UdpSock_;
    int UdpPort_;

    //处理登录注册请求的
    int TcpSock_;
    int TcpPort_;
    
    std::string SvrIp_;
};

