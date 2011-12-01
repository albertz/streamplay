// Mac:
// gcc -I /Library/Frameworks/SDL.framework/Headers -I /Library/Frameworks/SDL_mixer.framework/Headers -framework SDL -framework SDL_mixer -framework Cocoa -D PORT=6661 sdlplaystream.c MacMain.m -o play6661

// Linux/Unix:
// gcc -lSDL -D PORT=6662 sdlplaystream.c -o play6662

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

void init_sdl (void)
{
	if (SDL_Init (SDL_INIT_VIDEO|SDL_INIT_AUDIO) < 0)
		exit (-1);
	atexit (SDL_Quit);
	screen = SDL_SetVideoMode (640, 480, 16, SDL_HWSURFACE);
	if (screen == NULL)
		exit (-1);
}

int sock = 0;

void Callback (void *userdata, Uint8 *stream, int len)
{
	printf("expected len: %d\n", len);
//	Uint8* data = malloc(len);
//	SDL_MixAudio(stream, data, len, SDL_MIX_MAXVOLUME);
//	free(data);
	char data[128*1024];
	struct sockaddr_in sin;
	int sinlen = sizeof(sin);
	int reclen = recvfrom(sock, data, sizeof(data), 0, (struct sockaddr *)&sin, &sinlen);
	printf("received: %d\n", reclen);
	if(reclen < 0) {
		printf("error: %s\n", strerror(errno));
		exit(-1);
	}
	SDL_MixAudio(stream, data, reclen, SDL_MIX_MAXVOLUME);
}

void play (void)
{
	//if (SDL_LoadWAV (stdin, &spec, &sound_buffer, &sound_len) == NULL)
	//	exit (-1);
	spec.callback = Callback;
	if (SDL_OpenAudio (&spec, NULL) < 0)
    {
		printf ("Kann audio nicht öffnen: %s\n", SDL_GetError ());
		exit (-1);
    }
	SDL_PauseAudio (0);
}

int createlistensocket(int port) {
	int s = socket(AF_INET, SOCK_DGRAM, 0);
	printf("socket: %d\n", s);
	struct sockaddr_in sin;
	sin.sin_family = AF_INET;
	sin.sin_port = htons(port);
	sin.sin_addr.s_addr = INADDR_ANY;
	int ret = bind(s, (struct sockaddr *)&sin, sizeof(sin));
	printf("bind ret: %d\n", ret);
	return s;	
}


int main (int argc, char** argv)
{
//	signal(SIGINT, interrupt);
	
	// define PORT outside
	sock = createlistensocket(PORT);
	
	init_sdl ();
	signal(SIGPIPE, SIG_IGN);
	signal(SIGINT, SIG_DFL);
	
	play ();
	while(1)
	SDL_Delay (1000);
	/* Speicher nur freigeben, wenn WAV nicht mehr spielt. */
	// SDL_FreeWAV (sound_buffer);
	return 0;
}
