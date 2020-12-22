#pragma once 
#include<iostream>
#include<cstdio>
#include<cstdlib>
#include<sys/time.h>
#include<string.h>

//[时间 info/warning/error/fatal/debug 文件 行号] 具体的错误信息

const char* Level[]={
  "INFO",
  "WARNING",
  "ERROR",
  "FATAL",
  "DEBUG"
};
enum LogLevel
{
  INFO = 0,
  WARNING,
  ERROR,
  FATAL,
  DEBUG
};

class LogTime
{
  public:
    static int GetTimeStamp()
    {
      //第一个参数是一个结构体 struct timeval
      //第二个参数是时区，一般NULL
      struct timeval tv;
      gettimeofday(&tv,NULL);
      //一个大的整数
      return tv.tv_sec;
    }
    static void GetTimeStamp(std::string& timestamp)
    {
      //返回 年月日 时分秒
      time_t SysTime;
      time(&SysTime);

      struct tm* ST = localtime(&SysTime);
      //格式化字符串 [YYY-MM-DD HH-mm-ss]
      char TimeNow[23];
      snprintf(TimeNow,sizeof(TimeNow)-1,"%04d-%02d-%02d %02d:%02d:%02d",ST->tm_year + 1900,ST->tm_mon + 1,ST->tm_mday,ST->tm_hour,ST->tm_min,ST->tm_sec);
      timestamp.assign(TimeNow,strlen(TimeNow));
    }
};
inline std::ostream& Log(LogLevel lev,const char* file,int line, const std::string& logmsg)
    {
      std::string level_info = Level[lev];
      std::string timer_stamp;

      LogTime::GetTimeStamp(timer_stamp);
//[时间 info/warning/error/fatal/debug 文件 行号] 具体的错误信息
      std::cout<<"["<<timer_stamp<<" "<<level_info<<" "<<file<<":"<<line<<"]"<<logmsg;
      return std::cout;

    }
#define LOG(lev,msg) Log(lev,__FILE__,__LINE__,msg)
