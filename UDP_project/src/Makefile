CFLAGS=-std=c++11
LDFLAGS=-lpthread -ljsoncpp


SVRBIN=../bin/svr/ChatSvr

.PHONY:ChatSvr
$(SVRBIN):ChatServer.cpp
	g++ $^ -o $@ $(CFLAGS) $(LDFLAGS)


.PHONY:clean
clean:
	rm -f $(SVRBIN) 


	

