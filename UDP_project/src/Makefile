CFLAGS=-std=c++11
LDFLAGS=-lpthread -ljsoncpp


#SVRBIN=../bin/svr/ChatSvr
SVRBIN=./ChatSvr
CLIBIN=./ChatCli

.PHONY:all
all:$(SVRBIN) $(CLIBIN)
$(SVRBIN):ChatServer.cpp
	g++ $^ -o $@ $(CFLAGS) $(LDFLAGS)
$(CLIBIN):ChatClient.cpp
	g++ $^ -o $@ -g


.PHONY:clean
clean:
	rm -f $(SVRBIN)  $(CLIBIN) 


	

