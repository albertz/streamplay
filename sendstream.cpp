// clang++ -std=c++0x sendstream.cpp -o sendstream
// g++ -std=c++0x sendstream.cpp -o sendstream
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
#include <time.h>
#include <vector>
#include <string>

int FREQ = 44100;
int BITS_PER_SAMPLE = 16 * 2;
int BYTES_PER_SAMPLE = BITS_PER_SAMPLE / 8;
int BUF_SAMPLE_NUM = 4;
int BUF_SIZE = BYTES_PER_SAMPLE * BUF_SAMPLE_NUM;

void sys_assert(bool c, const char* msg) {
	if(!c) {
		fprintf(stderr, "system error: %s: %s\n", msg, strerror(errno));
		exit(-1);
	}
}

uint64_t getticks() {
	struct timeval now;
	gettimeofday(&now, NULL);
	return uint64_t(now.tv_sec) * 1000 + now.tv_usec / 1000;
}

void senddata(int s, char* data, size_t len, const char* addr, int port) {
	struct sockaddr_in sin;
	sin.sin_family = AF_INET;	
	sin.sin_port = htons(port); // htons for network byte order	
	sin.sin_addr.s_addr = inet_addr(addr);
		
	sendto(s, data, len, 0, (struct sockaddr *)&sin, sizeof(sin));	
}

int createsocket() {
	int s = socket(AF_INET, SOCK_DGRAM, 0);
	sys_assert(s > 0, "socket creation failed");
	struct sockaddr_in sin;
	sin.sin_family = AF_INET;
	sin.sin_port = 0; //htons(9999);
	sin.sin_addr.s_addr = INADDR_ANY;
	int ret = bind(s, (struct sockaddr *)&sin, sizeof(sin));
	sys_assert(ret == 0, "bind failed");
	return s;
}

struct Target {
	std::string ip;
	int port;
	int sock;
};

void initAddrFromStr(Target& ret, const std::string& s) {
	size_t p = s.find(":");
	sys_assert(p != std::string::npos, "usage: <ip:port>+");
	ret.ip = s.substr(0, p);
	ret.port = atoi(s.substr(p+1).c_str());
	sys_assert(ret.port != 0, "usage: <ip:port>+");
}

int main(int argc, char** argv) {
	sys_assert(argc >= 2, "usage: . <ip:port>+");
	std::vector<Target> targets(argc - 1);

	for(int i = 0; i < targets.size(); ++i)
		initAddrFromStr(targets[i], argv[i+1]);

	for(Target& t : targets)
		t.sock = createsocket(); 
	
	for(auto t : targets)
		printf("sending to %s:%d\n", t.ip.c_str(), t.port);
	
	while(1) {
		char data[4096];
		fread(data, 1, sizeof(data), stdin);
		
		for(Target& t : targets)
			senddata(t.sock, data, sizeof(data), t.ip.c_str(), t.port);

		usleep(50*  1000); // TODO ...
	}
	
}