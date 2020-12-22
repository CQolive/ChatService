#pragma once 
#include<queue>
#include<iostream>
#include<pthread.h>
#include<string>

#define MSG_POOL_SIZE 1024

class MsgPool
{
  public:
    MsgPool()
    {
      Capacity = MSG_POOL_SIZE;
      pthread_mutex_init(&MsgQueLock_,NULL);
      pthread_cond_init(&SynComQue_,NULL);
      pthread_cond_init(&SynProQue_,NULL);

    }
    ~MsgPool()
    {
      pthread_mutex_destroy(&MsgQueLock_);
      pthread_cond_destroy(&SynComQue_);
      pthread_cond_destroy(&SynProQue_);
    }

    void PushMsgPool(std::string&msg)
    {
      pthread_mutex_lock(&MsgQueLock_);
      while(IsFull())
      {
        pthread_cond_wait(&SynProQue_,&MsgQueLock_);
      }
      MsgQue_.push(msg);
      pthread_mutex_unlock(&MsgQueLock_);
      pthread_cond_signal(&SynComQue_);
    }
    void PopMsgPool(std::string* msg)
    {
      pthread_mutex_lock(&MsgQueLock_);
      while(MsgQue_.empty())
      {
        pthread_cond_wait(&SynComQue_,&MsgQueLock_);
      }
       *msg =  MsgQue_.front();
       MsgQue_.pop();
      pthread_mutex_unlock(&MsgQueLock_);
      pthread_cond_signal(&SynProQue_);
    }
  private:
    bool IsFull()
    {
      if(MsgQue_.size() == Capacity)
        return true;
      return false;
    }
  private:
    std::queue<std::string> MsgQue_;
    //约束队列大小，防止异常情况下，队列无限扩容吗，导致内存占用过大被系统强杀
    size_t Capacity;
    //互斥
    pthread_mutex_t MsgQueLock_;
    //同步 消费者条件变量
    pthread_cond_t SynComQue_;
    //生产者条件变量
    pthread_cond_t SynProQue_;

};
