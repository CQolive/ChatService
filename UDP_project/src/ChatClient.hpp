#pragma once
#include<iostream>
#include<stdio.h>
#include<sys/socket.h>
#include<arpa/inet.h>
#include<sys/types.h>
#include<string.h>

#include<unistd.h>
#include<string>
#include"json/json.h"

#include"LogSvr.hpp"
#include"ConnectInfo.hpp"

#define UDPPORT 17777
#define TCPPORT 17778

struct MySelf
{
  std::string NickName_;
  std::string School_;
  std::string Passwd_;
  uint64_t UserId_;
};

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

    }

    bool Connect2Server()
    {
      //创建套接字
      TcpSock_ = socket(AF_INET,SOCK_STREAM,IPPROTO_TCP);
      if(TcpSock_ < 0)
      {
        LOG(ERROR,"client create tcp socket failed")<<std::endl;
        exit(2);
      }
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

    bool Register()
    {
      if(!Connect2Server())
      {
        return false;
      }

      //1.发送注册标识
      char type = REGISTER;
      ssize_t send_size = send(TcpSock_,&type,1,0);
      if(send_size < 0)
      {
        LOG(ERROR,"Send Register type failed")<<std::endl;
        return false;
      }
      //2.发送注册内容
      struct RegInfo ri;
      std::cout<<"please Enter Your NickName: ";
      std::cin>>ri.NickName_;
      std::cout<<"Please Enter Your school:";
      std::cin>>ri.School_;
      while(1)
      {
        std::cout<<"Please Enter Your password:";
        std::string PasswdOne;
        std::string PasswdTwo;
        std::cin>>PasswdOne;
        std::cout<<"Please Enter Your Password again:";
        std::cin>>PasswdTwo;

        if(PasswdOne == PasswdTwo)
        {
          strcpy(ri.Passwd_,PasswdOne.c_str());
          break;
        }
        else
        {
          std::cout<<"The password did not match twice"<<std::endl;
        }
      }

      send(TcpSock_,&ri,sizeof(ri),0);
      if(send_size < 0)
      {
        LOG(ERROR,"Send Register type failed")<<std::endl;
        return false;
      }
      //3，解析应答状态和获取用户ID
      struct RelyInfo resp;
      ssize_t recv_size = recv(TcpSock_,&resp,sizeof(resp),0);
      if(recv_size < 0)
      {
        LOG(ERROR,"recv register response failed")<<std::endl;
        return false;
      }
      else if(recv_size == 0)
      {
        LOG(ERROR,"Peer shutdown connect")<<std::endl;
        return false;
      }

      if(resp.Status != REGISTERED)
      {
        LOG(ERROR,"Register Failed")<<std::endl;
        return false;
      }
      LOG(INFO,"Register success UserId is ")<<resp.UserId_<<std::endl;
      //4.保存服务端返回的USERID
      me_.NickName_ = ri.NickName_;
      me_.School_ = ri.School_;
      me_.Passwd_ = ri.Passwd_;
      me_.UserId_ = resp.UserId_;

      close(TcpSock_);
      return true;
    }
    bool Login()
    {
      if(!Connect2Server())
      {
        return false;
      }
      //1.发送登录标识
      char type = LOGIN;
      ssize_t send_size = send(TcpSock_,&type,1,0);
      if(send_size < 0)
      {
        LOG(ERROR,"Send Login type failed")<<std::endl;
        return false;
      }
      //2.发送登陆数据
      struct LoginInfo li;
      li.UserId_ = me_.UserId_;
      strcpy(li.Passwd_,me_.Passwd_.c_str());

      send_size = send(TcpSock_,&li,sizeof(li),0);
      if(send_size < 0)
      {
        LOG(ERROR,"Send Login data failed")<<std::endl;
        return false;
      }
      //3.解析登陆状态
      struct RelyInfo resp;
      ssize_t recv_size = recv(TcpSock_,&resp,sizeof(resp),0);
      if(recv_size < 0)
      {
        LOG(ERROR,"recv Login response failed")<<std::endl;
        return false;
      }
      else if(recv_size == 0)
      {
        LOG(ERROR,"Peer shutdown connect")<<std::endl;
        return false;
      }
      if(resp.Status != LOGINED)
      {
        LOG(ERROR,"Login failed Status is")<<resp.Status<<std::endl;
        return false;
      }
      LOG(INFO,"Login success")<<std::endl;
      close(TcpSock_);
      return true;
    }
    //Udp数据的收发
    bool SendMsg(const std::string& msg)
    {
      struct sockaddr_in peeraddr;
      peeraddr.sin_family = AF_INET;
      peeraddr.sin_port = htons(UdpPort_);
      peeraddr.sin_addr.s_addr = inet_addr(SvrIp_.c_str());
      ssize_t send_size = sendto(UdpSock_,msg.c_str(),msg.size(),0,(struct sockaddr*)&peeraddr,sizeof(peeraddr));
      if(send_size < 0)
      {
        LOG(ERROR,"Send Msg to Server Failed")<<std::endl;
        return false;
      }
        return true;
    }
    bool RecvMsg(std::string* msg)
    {
      char buf[MESSAGE_MAX_SIZE];
      memset(buf,'\0',sizeof(buf));
      struct sockaddr_in svraddr;
      socklen_t svraddrlen = sizeof(svraddr);
      ssize_t recv_size = recvfrom(UdpSock_,buf,sizeof(buf)-1,0,(struct sockaddr*)&svraddr,&svraddrlen);
      if(recv_size < 0)
      {
        LOG(ERROR,"recv msg from server failed");
        return false;
      }
      (*msg).assign(buf,recv_size);
      return true;
    }


  private:
    //发送正常的业务数据
    int UdpSock_;
    int UdpPort_;

    //处理登录注册请求的
    int TcpSock_;
    int TcpPort_;

    //保护服务端的IP
    std::string SvrIp_;
    //客户端自己的信息
    MySelf me_;
};

