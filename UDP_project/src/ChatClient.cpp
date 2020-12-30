#include"ChatClient.hpp"

void Menu()
{
  std::cout<<"------------------------------------"<<std::endl;
  std::cout<<"|1.register                 2.login|"<<std::endl;
  std::cout<<"                                    "<<std::endl;
  std::cout<<"|3.logout                   4.exit |"<<std::endl;
  std::cout<<"------------------------------------"<<std::endl;
}

int main(int argc,char* argv[])
{
  if(argc != 2)
  {
    std::cout<<"./Cli [ip]"<<std::endl;
    exit(1);
  }
  ChatClient* cc = new ChatClient(argv[1]);
  //1.初始化服务
  cc->Init();
  while(1)
  {
    Menu();
    int Select = -1;
    std::cout<<"Please Select service: ";
    fflush(stdout);
    std::cin>>Select;
    //2.注册
    if(Select == 1)
    {
      //注册
      if(!cc->Register())
      {
        std::cout<<"Register failed! Please Try Again!"<<std::endl;
      }
      else
      {
        std::cout<<"Register success! Please login!"<<std::endl;
      }
    }
    else if(Select == 2)
    {
      //登录
      if(!cc->Login())
      {
        std::cout<<"login failed! Please check Your UserId or Password"<<std::endl;
      }
      else 
      {
        std::cout<<"login success! Please Chat Online!"<<std::endl;
      }
    }
    else if(Select == 4)
    {
      //退出登录
      break;
    }

  }
  //3.登录
  //4.发送&接收数据
  
  delete cc;
  return 0;
}
