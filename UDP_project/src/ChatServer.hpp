#pragma once 
#include<sys/socket.h>
#include<arpa/inet.h>
#include<sys/types.h>
#include<unistd.h>
#include<pthread.h>
#include<string>
#include <iostream>

#include "MsgPool.hpp"
#include "LogSvr.hpp"
#include "ConnectInfo.hpp"
#include "UserManager.hpp"


#define UDP_PORT 17777
#define TCP_PORT 17778
#define THREAD_COUNT 2
//当前类实现
//1.接受客户端数据
//2.发送数据消息给客户端
//依赖UDP协议进行实现
class ChatServer
{
  public:
    ChatServer()
    {
      UdpSock_ = -1;
      UdpPort_ = UDP_PORT;
      MsgPool_ = NULL;
      TcpSock_ = -1;
      TcpPort_ = TCP_PORT;
      UserMana_ = NULL;
    }
    ~ChatServer()
    {
      if(MsgPool_)
      {
        delete MsgPool_;
        MsgPool_ = NULL;
      }
      if(UserMana_)
      {
        delete UserMana_;
        UserMana_ = NULL;
      }

    }

    //上次调用IninServer函数来初始化UDP服务
    void IninServer()
    {
      //1.创建套接字
      UdpSock_ = socket(AF_INET,SOCK_DGRAM,IPPROTO_UDP) ;
      if(UdpSock_ < 0 )
      {
        LOG(FATAL,"Create socket faild");
        exit(1);
      }
      //2.绑定地址信息
      struct sockaddr_in addr;
      addr.sin_family = AF_INET;
      addr.sin_port = htons(UdpPort_);
      addr.sin_addr.s_addr = inet_addr("0.0.0.0");
      
      int ret = bind(UdpSock_,(struct sockaddr*)&addr,sizeof(addr));
      if(ret < 0)
      {
        LOG(FATAL,"bind addrinfo failed");
        exit(2);
      }

      LOG(INFO,"Udp bind success");
      //初始化数据池
      MsgPool_ = new MsgPool();
      if(!MsgPool_)
      {
        LOG(FATAL,"Create MsgPool failed");
        exit(3);
      }
      LOG(INFO,"Create MsgPool success");

      UserMana_ = new UserManager();
      if(UserMana_ == NULL)
      {
        LOG(FATAL,"Create User Manager failed");
        exit(8);
      }

      //创建TCP-socket 
      TcpSock_ = socket(AF_INET,SOCK_STREAM,IPPROTO_TCP);
      if(TcpSock_ < 0 )
      {
        LOG(FATAL,"Create Tcp socket faild");
        exit(5);
      }

      struct sockaddr_in tcpaddr;
      tcpaddr.sin_family = AF_INET;
      tcpaddr.sin_port = htons(TCP_PORT);
      tcpaddr.sin_addr.s_addr = inet_addr("0.0.0.0");
      ret = bind(TcpSock_,(struct sockaddr*)&tcpaddr,sizeof(tcpaddr));
      if(ret < 0)
      {
        LOG(FATAL,"Bind Tcp addrinfo faild");
        exit(6);
      }

      ret = listen(TcpPort_,5);
      if(ret < 0)
      {
        LOG(FATAL,"Tcp listen failed");
        exit(7);
      }
      LOG(INFO,"Tcp listen 0.0.0.0:17778");
    }
    //初始化程序当中生产和消费线程
    void Start()
    {
      pthread_t tid;
      for(int i = 0;i < THREAD_COUNT;i++)
      {
        int ret = pthread_create(&tid,NULL,ProductMsgStart,(void*)this);
        if(ret < 0)
        {
          LOG(FATAL,"pthread_create new thread failed");
          exit(4);
        }
         ret = pthread_create(&tid,NULL,ConsumeMsgStart,(void*)this);
         if(ret < 0)
         {
           LOG(FATAL,"pthread_create new thread failed");
         //  Log(FATAL,__FILE__,__LINE__,"pthread");
           exit(4);
         }
      }
      LOG(INFO,"UdpChat Service start success");
    
      while(1)
      {
        struct sockaddr_in cliaddr;
        socklen_t cliaddrlen = sizeof(cliaddr);
        int newsock = accept(TcpSock_,(struct sockaddr*)&cliaddr,&cliaddrlen);
        if(newsock < 0)
        {
          LOG(ERROR,"Accept new connect failed");
          continue;
        }
        LoginConnect* lc = new LoginConnect(newsock,(void*)this);
        if(!lc)
        {
          LOG(ERROR,"Create LoginConnect failed");
        }
        //创建线程，处理登录，注册的请求
        pthread_t tid;
        int ret = pthread_create(&tid,NULL,LoginRegStart,(void*)lc);
        if(ret < 0)
        {
          LOG(ERROR,"Create User LoginConnect thread failed");
          continue;
        }
        LOG(INFO,"Create TcpConnect thread success");
      }

    }
  private:
    static void* ProductMsgStart(void* arg)
    {
      pthread_detach(pthread_self());
      ChatServer* cs = (ChatServer*)arg;
      while(1)
      {
        //recvfrom
        cs->RecvMsg();

      }
      return NULL;
    }
  private:
    static void* ConsumeMsgStart(void* arg)
    {
      pthread_detach(pthread_self());
      ChatServer* cs = (ChatServer*)arg;
      while(1)
      {
        cs->BroadcastMsg();
      }
      return NULL;
    }
    static void* LoginRegStart(void* arg)
    {
      pthread_detach(pthread_self());
      LoginConnect* lc = (LoginConnect*)arg;
      ChatServer* cs = (ChatServer*)lc->GetServer(); 
      //注册，登录请求
      //请求从cli端来，recv(sock,buf,size,0);
      char QuesType;
      ssize_t recvsize = recv(lc->GetTcpSock(),&QuesType,1,0);
      if(recvsize < 0)
      {
        LOG(ERROR,"Recv TagType failed");
        return NULL;
      }
      else if(recvsize == 0)
      {
        LOG(ERROR,"Client shutdown connect");
        return NULL;
      }

      uint64_t UserId = -1;
      int UserStaus = -1;
      //正常接收到一个请求标识
      switch(QuesType)
      {
        case REGISTER:
          //使用用户管理模块的注册
          UserStaus =  cs->DealRegister(lc->GetTcpSock(),&UserId);
          break;
        case LOGIN:
          //使用用户管理模块的登录
          UserStaus = cs->DealLogin(lc->GetTcpSock());
          break;
        case LOGINOUT:
          //使用用户管理模块的退出登录
          cs->DealLoginout();
          break;
        default:
          LOG(ERROR,"Recv Request type not a effective value");
          break;
      }
      //响应 send(sock,buf,size,0)
      RelyInfo ri;
      ri.Status = UserStaus;
      ri.UserId_ = UserId;
      ssize_t sendsize = send(lc->GetTcpSock(),&ri,sizeof(ri),0);
      if(sendsize < 0)
      {
        LOG(ERROR,"SendMsg Failed");
      }
      LOG(INFO,"SendMsg success");

      //将TCP连接释放掉
      close(lc->GetTcpSock());
      delete lc;
      return NULL;


      return NULL;
    }
    int DealRegister(int Sock,uint64_t* UserId)
    {
      //接受注册请求
      RegInfo ri;
      ssize_t recvsize = recv(Sock,&ri,sizeof(ri),0);
      if(recvsize < 0)
      {
        LOG(ERROR,"Recv TagType failed");
        return OFFLINE;
      }
      else if(recvsize == 0)
      {
        LOG(ERROR,"Client shutdown connect");
        //特殊处理对端关闭的情况
      }
      //调用用户管理模块进行注册请求处理
      int ret = UserMana_->Register(ri.NickName_,ri.School_,ri.Passwd_,UserId);
      //返回注册成功之后给用户的UserId
      if(ret == -1)
      {
        return REGFAILED;
      }
      //返回当前状态
      return REGISTERED;

    }
    int DealLogin(int Sock)
    {
      struct LoginInfo li;
      ssize_t recvsize = recv(Sock,&li,sizeof(li),0);
      if(recvsize < 0)
      {
        LOG(ERROR,"Recv TagType failed");
        return OFFLINE;
      }
      else if(recvsize == 0)
      {
        LOG(ERROR,"Client shutdown connect");
        //需要处理
      }

      int ret = UserMana_->Login(li.UserId_,li.Passwd_);
      if(ret == -1)
      {
        return LOGINFAILED;
      }
      return LOGINED;

    }
    int DealLoginout()
    {

    }
  private:
    void RecvMsg()
    {
      char buf[10240] = {0};
      struct sockaddr_in cliaddr;
      socklen_t cliaddrlen = sizeof(struct sockaddr_in);
      int recvsize = recvfrom(UdpSock_,buf,sizeof(buf)-1,0,(struct sockaddr*)&cliaddr,&cliaddrlen);
      if(recvsize < 0)
      {
        LOG(ERROR,"recvfrom msg failed");
      }
      else{
        //正常逻辑
        std::string msg;
        msg.assign(buf,recvsize);
        LOG(INFO,msg);
        //需要增加用户管理，只有注册登录的人才可以向服务器发送消息
        //1.校验当前的消息是否注册用户或者老用户发送的
        //    1.1 不是，则认为使非法消息
        //    1.2 使区分一下是否第一次发送消息
        //       是：保存地址信息，并且更新状态为在线，将数据放到数据池当中 
        //       是老用户：直接将数据放到数据池当中
        //2.需要校验，势必是和用户管理模块打交道
        UserMana_->IsLogin(cliaddr,cliaddrlen);
        MsgPool_->PushMsgPool(msg);
      }
    }

    void BroadcastMsg()
    {
      //1.获取要给哪一个用户进行发送
      //2.获取发送内容
      //SendMsg();
      std::string msg;
      MsgPool_->PopMsgPool(&msg);
    }
    //给一个客户端发送单个消息接口
    void SendMsg(const std::string& msg,struct sockaddr_in& cliaddr,socklen_t&len)
    {
      ssize_t sendsize = sendto(UdpSock_,msg.c_str(),msg.size(),0,(struct sockaddr*)&cliaddr,len);
      if(sendsize < 0)
      {
        LOG(ERROR,"sendto msg faild");
      }
      else
      {
        //成功
      }
    }
  private:
    int UdpSock_;
    int UdpPort_;
    //数据池
    MsgPool* MsgPool_;

    //tcp处理注册，登录请求
    int TcpSock_;
    int TcpPort_;
    //用户管理
    UserManager* UserMana_;
};