#pragma once 
#include<iostream>

#define REGISTER 0
#define LOGIN 1
#define LOGINOUT 2
//注册信息
struct RegInfo
{
    char NickName_[15];
    char School_[20];
    char Passwd_[20];
};
//登录信息
struct LoginInfo
{
  u_int64_t UserId_;//返回给用户的ID号
  char Passwd_[20];
};

enum USerStatus
{
  REGFAILED = 0,
  REGISTERED,
  LOGINFAILED,
  LOGINED
};

//应答信息
struct RelyInfo
{
  //当前状态，注册完成，登陆完成
  //OFFLINE REGISTER LOGINED ONLINE
  int Status;
  uint64_t UserId_;
};
class LoginConnect
{
  public:
    LoginConnect(int sock,void* Server)
    {
      Sock_ = sock;
      Server_ = Server;
    }
    int GetTcpSock()
    {
      return Sock_;
    }
    void* GetServer()
    {
      return Server_;
    }
  private:
    int Sock_;
    //可以保存ChatServer类的实例化指针
    void* Server_;

};
