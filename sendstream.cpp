// g++ sendstream.cpp -o sendstream
//  { while true; do cat ~/audiodump.wav; done; } | ./sendstream

#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <unistd.h>
#include <signal.h>
#include <stdio.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/time.h>
#include <stdlib.h>
#include <memory.h>
#include <assert.h>


int senddata(int s, char* data, size_t len, const char* addr, int port) {
	struct sockaddr_in sin;
	sin.sin_family = AF_INET;	
	sin.sin_port = htons(port); // htons for network byte order	
	sin.sin_addr.s_addr = inet_addr(addr);
		
	sendto(s, data, len, 0, (struct sockaddr *)&sin, sizeof(sin));	
}

int createsocket() {
	int s = socket(AF_INET, SOCK_DGRAM, 0);
	printf("socket: %d\n", s);
	struct sockaddr_in sin;
	sin.sin_family = AF_INET;
	sin.sin_port = 0; //htons(9999);
	sin.sin_addr.s_addr = INADDR_ANY;
	int ret = bind(s, (struct sockaddr *)&sin, sizeof(sin));
	printf("bind ret: %d\n", ret);
	return s;
}

int main(int argc, char** argv) {
	assert(argc == 3);
	int s1 = createsocket(), s2 = createsocket();
	
	while(1) {
		char data[4096];
		fread(data, 1, sizeof(data), stdin);
		
		senddata(s2, data, sizeof(data), argv[2], 6662);
		senddata(s1, data, sizeof(data), argv[1], 6661);

		usleep(50*  1000); // TODO ...
	}
	
}