/*
Audio Modem Terminal V 0.0.1  
Copyrite (C) 2007 Eric Seifert

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
02110-1301, USA.
*/

#include <SDL/SDL.h>
#include <math.h>
#include <string.h>


#define PI 3.14159
#define BAUD 100

unsigned int offset	= 0;
unsigned int nsamples	= 0;
short* audio_data	= NULL;
short* new_mem		= NULL;

void fill_audio_buffer(void* data, Uint8 *stream, int len);
int init_me();
void cleanup();


void cleanup()
{
	SDL_CloseAudio();
	if(audio_data)
		free(audio_data);

}
int init_me()
{
	SDL_AudioSpec des;
	
	des.freq 		= 48000;
	des.format 		= AUDIO_S16;
	des.channels 		= 1;
	des.samples 		= 512;
	des.silence 		= 0;
	des.size 		= 1024;
	des.callback 		= fill_audio_buffer;
	des.userdata 		= 0;

	SDL_AudioSpec rec;
	if(SDL_OpenAudio(&des, &rec) == -1)
	{
		printf("SDL_Init failed: %s\n", SDL_GetError());
		exit(-1);
	}
	if(rec.freq != des.freq)
	{
		printf("Did not recieve desired samples per second!");
		exit(1);
	}
	SDL_PauseAudio(0);

	return 0;
}
int transmit_string(char* str)
{	

	unsigned short str_len	= strlen(str);
	unsigned short len_mod	= str_len-1;
	char* tx_str 		= malloc(str_len+2);
	tx_str[0] 		= 0xAB;
	tx_str[1]		= ((unsigned char*)&len_mod)[0];
	tx_str[2]		= ((unsigned char*)&len_mod)[1];
	memcpy(&tx_str[3], str, str_len-1);
	int tx_len 		= str_len+2;
	unsigned int old_off	= 0;
	unsigned int old_nsam	= 0;
	
	if(offset < nsamples) //we are still playing the last message
	{
		memmove(audio_data, ((void*)audio_data)+offset, nsamples-offset);
		new_mem = realloc(audio_data, (int)(2*8.0*tx_len*48000/BAUD) + (nsamples-offset));
		if(!new_mem)
			return -1;				
		audio_data	= new_mem;
		new_mem		= NULL;
		old_off		= offset;
		old_nsam	= nsamples;
		nsamples	= (int)(2*8.0*tx_len*48000/BAUD) + (nsamples-offset);
		offset		= 0;
	}
	else
	{
		if(audio_data)
			free(audio_data);
		audio_data	= (short*)malloc((int)(2*48000*tx_len*8.0/BAUD));
		if(!audio_data)
			return -1;
		nsamples 	= (int)(2*48000*tx_len*8.0/BAUD);
		offset		= 0;
	}
	int by;
	for(by=0;by<tx_len;by++)
	{
		int bi;
		for(bi=0;bi<8;bi++)
		{
			if(tx_str[by] & (0x01<<bi))
			{
				int i;
				for(i=0;i<48000/BAUD;i++)
					audio_data[((old_nsam-old_off)/2)+by*8*48000/BAUD + bi*48000/BAUD + i] = 0x7FFF*sin(2*PI*12000.0*i/48000.0);
			}
			else
			{
				int i;
				for(i=0;i<48000/BAUD;i++)
					audio_data[((old_nsam-old_off)/2)+by*8*48000/BAUD + bi*48000/BAUD + i] = 0;
			}
		}
	}
	free(tx_str);
	return 0;
}



void fill_audio_buffer(void* data, Uint8 *stream, int len)
{
	
	if(offset+len < nsamples)
	{
		memcpy(stream, ((void*)audio_data)+offset, len);
		offset+=len;
	}
	else
	{
		
		if(offset<nsamples)
		{
			memcpy(stream, ((void*)audio_data)+offset, nsamples-offset);
			memset(stream+nsamples-offset, 0, len-(nsamples-offset));
			offset=nsamples;
		}
		else
		{
			memset(stream, 0, len);
		}		
		
		
	}
}
