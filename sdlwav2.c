#include <SDL/SDL.h>
#include <stdlib.h> 
#include <signal.h>

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

void Callback (void *userdata, Uint8 *stream, int len)
{
	Uint8* data = malloc(len);
	fread(data, 1, len, stdin);
	SDL_MixAudio(stream, data, len, SDL_MIX_MAXVOLUME);
	free(data);
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

int main (int argc, char** argv)
{
	signal(SIGPIPE, SIG_IGN);
	signal(SIGINT, interrupt);
	
	init_sdl ();
	play ();
	while(1)
	SDL_Delay (1000);
	/* Speicher nur freigeben, wenn WAV nicht mehr spielt. */
	// SDL_FreeWAV (sound_buffer);
	return 0;
}
