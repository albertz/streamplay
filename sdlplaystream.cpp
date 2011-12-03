// Mac:
// g++ -I /Library/Frameworks/SDL.framework/Headers -I /Library/Frameworks/SDL_mixer.framework/Headers -framework SDL -framework SDL_mixer -framework Cocoa sdlplaystream.cpp MacMain.m -o play

// Linux/Unix:
// g++ -lSDL sdlplaystream.cpp -o play

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


SDL_Surface *screen;
SDL_AudioSpec spec;
Uint32 sound_len;
Uint8 *sound_buffer;
int sound_pos = 0;

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
		SDL_Init(SDL_INIT_VIDEO|SDL_INIT_AUDIO) >= 0,
		"SDL_Init failed");
	atexit (SDL_Quit);
	screen = SDL_SetVideoMode (640, 480, 16, SDL_HWSURFACE);
	sdl_assert(screen != NULL, "SDL_SetVideoMode failed");
}

int sock = 0;


void Callback (void *userdata, Uint8 *stream, int len)
{
	printf("expected len: %d\n", len);
//	Uint8* data = malloc(len);
//	SDL_MixAudio(stream, data, len, SDL_MIX_MAXVOLUME);
//	free(data);
	Uint8 data[128*1024];
	struct sockaddr_in sin;
	socklen_t sinlen = sizeof(sin);
	int reclen = recvfrom(sock, data, sizeof(data), 0, (struct sockaddr *)&sin, &sinlen);
	printf("received: %d\n", reclen);
	sys_assert(reclen >= 0, "recvfrom error");
	SDL_MixAudio(stream, data, reclen, SDL_MIX_MAXVOLUME);
}

void play (void)
{
	//if (SDL_LoadWAV (stdin, &spec, &sound_buffer, &sound_len) == NULL)
	//	exit (-1);
	spec.callback = Callback;
	sdl_assert(
		SDL_OpenAudio (&spec, NULL) >= 0,
		"SDL_OpenAudio failed");
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


int main (int argc, char** argv)
{
	sys_assert(argc == 2, "usage: . <port>");
	int port = atoi(argv[1]);
	
	sock = createlistensocket(port);
	printf("listening on %d\n", port);
	
	init_sdl();
	signal(SIGPIPE, SIG_IGN);
	signal(SIGINT, SIG_DFL);
	
	play ();
	while(1)
		SDL_Delay (1000);
	return 0;
}
