#pragma once 
#include<iostream>
#include<string>
#include"json/json.h"
class Message
{
  public:
    //反序列化客户端发送给我们的json数据串
    void Deserialize(std::string Message)
    {
      Json::Reader reader;
      Json::Value val;
      reader.parse(Message,val,false);

      NickName_ = val["NickName_"].asString();
      School_ = val["School_"].asString();
      Msg_ = val["Msg_"].asString();
      UserId_ =val["UserId_"].asInt(); 
    }

    uint64_t& GetUserId()
    {
      return UserId_;
    }
  private:
    std::string NickName_;
    std::string School_;
    std::string Msg_;
    uint64_t UserId_;

};
