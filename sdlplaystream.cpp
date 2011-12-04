// Mac:
// clang++ -stdlib=libc++ -I /Library/Frameworks/SDL.framework/Headers -I /Library/Frameworks/SDL_mixer.framework/Headers -framework SDL -framework SDL_mixer -framework Cocoa sdlplaystream.cpp MacMain.m -o play

// Linux/Unix:
// g++|clang++ -std=c++0x -lSDL sdlplaystream.cpp -o play

#include <SDL/SDL.h>
#include <stdlib.h> 
#include <signal.h>
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
#include <errno.h>
#include <thread>
#include <deque>
#include <vector>


void custom_assert(bool c, const char* msg) {
	if(!c) {
		fprintf(stderr, "custom error: %s\n", msg);
		exit(-1);
	}
}

void sys_assert(bool c, const char* msg) {
	if(!c) {
		fprintf(stderr, "system error: %s: %s\n", msg, strerror(errno));
		exit(-1);
	}
}

void sdl_assert(bool c, const char* msg) {
	if(!c) {
		fprintf(stderr, "SDL error: %s: %s\n", msg, SDL_GetError());
		exit(-1);
	}
}

void init_sdl (void)
{
	sdl_assert(
		SDL_Init(SDL_INIT_AUDIO) >= 0,
		"SDL_Init failed");
	atexit (SDL_Quit);
}

int sock = 0;
std::mutex data_mutex;
std::deque<Uint8> data;

void Callback (void *userdata, Uint8 *stream, int len)
{
	//printf("expected len: %d\n", len);

	std::vector<Uint8> buf(len);
	data_mutex.lock();
	if(data.size() < len) {
		//printf("WARNING: underrun, expected: %d, have: %lu\n", len, data.size());
		while(data.size() < len) {
			data_mutex.unlock();
			usleep(500);
			data_mutex.lock();
		}
	}
	std::deque<Uint8>::iterator j = data.begin();
	for(int i = 0; i < len; ++i, ++j)
		buf[i] = *j;
	data.erase(data.begin(), j);
	data_mutex.unlock();
	
	SDL_MixAudio(stream, &buf[0], buf.size(), SDL_MIX_MAXVOLUME);
}

void play (void)
{
	SDL_AudioSpec spec;
	spec.freq = 44100;
	spec.format = AUDIO_S16LSB;
	spec.channels = 2;
	spec.samples = 32;
	spec.callback = Callback;
	spec.userdata = NULL;
	SDL_AudioSpec spec_obtained;
	sdl_assert(
		SDL_OpenAudio (&spec, &spec_obtained) >= 0,
		"SDL_OpenAudio failed");
	custom_assert(spec.freq == spec_obtained.freq, "freq 44.1kHz not supported");
	custom_assert(spec.format == spec_obtained.format, "format S16LSB not supported");
	custom_assert(spec.channels == spec_obtained.channels, "stereo not supported");	
	SDL_PauseAudio (0);
}

int createlistensocket(int port) {
	int s = socket(AF_INET, SOCK_DGRAM, 0);
	sys_assert(s > 0, "socket creation failed");
	struct sockaddr_in sin;
	sin.sin_family = AF_INET;
	sin.sin_port = htons(port);
	sin.sin_addr.s_addr = INADDR_ANY;
	int ret = bind(s, (struct sockaddr *)&sin, sizeof(sin));
	sys_assert(ret == 0, "bind failed");
	return s;	
}


int main (int argc, char** argv) {
	custom_assert(argc == 2, "usage: . <port>");
	int port = atoi(argv[1]);
	
	sock = createlistensocket(port);
	printf("listening on %d\n", port);
	
	init_sdl();
	signal(SIGPIPE, SIG_IGN);
	signal(SIGINT, SIG_DFL);
	
	play ();
	uint64_t c = 0;
	uint64_t recsum = 0;
	while(true) {
		c++;
		Uint8 buf[128*1024];
		struct sockaddr_in sin;
		socklen_t sinlen = sizeof(sin);
		int reclen = recvfrom(sock, buf, sizeof(buf), 0, (struct sockaddr *)&sin, &sinlen);
		//printf("received: %d\n", reclen);
		sys_assert(reclen >= 0, "recvfrom error");
		recsum += reclen;
		if(c % 100000 == 0)
			printf("received %llu MB\n", recsum / 1024 / 1024);
		{
			std::lock_guard<std::mutex> lock(data_mutex);
			for(int i = 0; i < reclen; ++i)
				data.push_back(buf[i]);
		}
	}
	
	return 0;
}
