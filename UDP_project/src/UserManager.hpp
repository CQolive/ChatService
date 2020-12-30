#pragma once 

#include<iostream>
#include<arpa/inet.h>
#include<string>
#include<cstring>
#include<unordered_map>
#include<vector>

#include"LogSvr.hpp"
#define OFFLINE 0
#define REGISTERED 1
#define USERLOGINED 2
#define ONLINE 3
class UserInfo
{
  public:
    //在注册和登陆阶段使用的是TCP，所以不能保存TCP的地址信息，而是要等到当前登陆上来的这个用户第一次使用UDP协议发送消息的时候，将UDP的地址信息保存下来
    //保存下来之后，进行群发的时候，就可以找到有效的UDP的地址信息
    UserInfo(const std::string& NickName,const std::string& School,uint64_t UserId,const std::string& Passwd)
    {
        NickName_ = NickName;
        School_ = School;
        UserId_ = UserId;
        Passwd_ = Passwd;
        memset(&CliAddr_,'0',sizeof(struct sockaddr_in));
        CliAddrLen_ = -1;
        UserStatus_ = OFFLINE;
    }

    void SetUserStatus(int Status)
    {
        UserStatus_ = Status;
    }
    std::string& GetPassws()
    {
      return Passwd_;
    }
    int& GetUserStatus()
    {
      return UserStatus_;
    }
    void SetCliAddrInfo(const struct sockaddr_in& CliAddr)
    {
      memcpy(&CliAddr_,&CliAddr,sizeof(CliAddr));
    }
    void SetCliAddrLen(const socklen_t& CliAddrLen)
    {
      CliAddrLen_ = CliAddrLen;
    }

    struct sockaddr_in& GetCliAddrInfo()
    {
      return CliAddr_;
    }

    socklen_t& GetCliAddrLen()
    {
      return CliAddrLen_;
    }
  private:
    std::string NickName_;
    std::string School_;
    //用户ID
    uint64_t UserId_;
    std::string Passwd_;
    //保存的是udp客户端的地址信息
    struct sockaddr_in CliAddr_;
    socklen_t CliAddrLen_;

    //保存当前用户的状态 状态机
    int UserStatus_;
};

class UserManager
{
  public:
    UserManager()
    {
      UserMap_.clear();
      OnlineUserVec_.clear();
      pthread_mutex_init(&Lock_,NULL);
      PreparUserId_ = 0;
    }
    ~UserManager()
    {
      pthread_mutex_destroy(&Lock_);
    }

    //当一个新用户需要注册的时候，NickName,School,Passwd,==>UserId
    int Register(const std::string& NickName,const std::string& School,const std::string& Passwd,uint64_t* UserId)
    {
      if(NickName.size() == 0 || School.size() == 0 || Passwd.size() == 0)
      {
        return -1;
      }
      pthread_mutex_lock(&Lock_);
      UserInfo userinfo(NickName,School,PreparUserId_,Passwd);
      //更改当前用户状态，改成已注册状态
      userinfo.SetUserStatus(REGISTERED);
      //插入到map
      UserMap_.insert(std::make_pair(PreparUserId_,userinfo));
      *UserId = PreparUserId_;
      PreparUserId_++;
      pthread_mutex_unlock(&Lock_);

      return 0;

    }
    int Login(const uint64_t& UserId,const std::string& Passwd)
    {
      if(Passwd.size() < 0)
      {
        return -1;
      }

      int LoginStatus = -1;

      //返回登陆的状态
      //1.先在map中查找是否存在这样的id
      //不存在
      //存在
      //  密码正确
      //  密码不正确
      std::unordered_map<uint64_t,UserInfo>::iterator iter;
      //防止迭代器失效
      pthread_mutex_lock(&Lock_);
      iter = UserMap_.find(UserId);
      if(iter != UserMap_.end())
      {
        //查找到了
        if(Passwd == iter->second.GetPassws())
        {
          //密码正确
          iter->second.GetUserStatus() = USERLOGINED;
          LoginStatus = 0;

        }
        else 
        {
          //密码不正确
        LOG(ERROR,"User Passwd is not correct passwd is ")<<Passwd<<std::endl;
          LoginStatus = -1;
        }
      }
      else 
      {
        //没找到
        LOG(ERROR,"UserId not found UserId is ")<<UserId<<std::endl;
          LoginStatus = -1;
      }
      pthread_mutex_unlock(&Lock_);
      return LoginStatus;

    }
    int Loginout();
    bool IsLogin(uint64_t UserId,const struct sockaddr_in& CliAddr,const socklen_t& CliaddrLen)
    {
      if(sizeof(CliAddr) < 0 || CliaddrLen < 0)
      {
        return false;
      }
      //1.检测当前用户是否存在
      std::unordered_map<uint64_t,UserInfo>::iterator iter;
      pthread_mutex_lock(&Lock_);
      iter = UserMap_.find(UserId);
      if(iter == UserMap_.end())
      {
        pthread_mutex_unlock(&Lock_);
        LOG(ERROR,"User not exist")<<std::endl;
        return false;
      }
      //2.判断当前用户状态，来判断是否完成注册和登录
      if(iter->second.GetUserStatus() == OFFLINE || iter->second.GetUserStatus() == REGISTERED)
      {
        pthread_mutex_unlock(&Lock_);
        LOG(ERROR,"User Status error");
        return false;
      }
      //3.判断当前用户是否第一次发送消息
      if(iter->second.GetUserStatus() == ONLINE)
      {
        pthread_mutex_unlock(&Lock_);
        return true;
      }

      //第一次发送消息
      if(iter->second.GetUserStatus() == USERLOGINED)
      {
        //增加地址信息，地址长度，改变状态为ONLINE
        //printf("%s:%d\n",inet_ntoa(CliAddr.sin_addr),ntohs(CliAddr.sin_port));
        iter->second.SetCliAddrInfo(CliAddr);
        iter->second.SetCliAddrLen(CliaddrLen);
        iter->second.SetUserStatus(ONLINE);

        OnlineUserVec_.push_back(iter->second);
      }
        pthread_mutex_unlock(&Lock_);
      return true;
    }

    void GetOnlineUserInfo(std::vector<UserInfo>* vec)
    {
      *vec = OnlineUserVec_;
    }
  private:
    //保存所有注册用户信息
      std::unordered_map<uint64_t,UserInfo> UserMap_;
      pthread_mutex_t Lock_;
    //保存在线用户的信息 -- Udp 判断在线的标准，是判断是否使用udp协议发送消息
      std::vector<UserInfo> OnlineUserVec_;
      //预分配的用户ID
      uint64_t PreparUserId_;
};
