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

#define _GNU_SOURCE
#include <stdio.h>
#include <SDL/SDL.h>
#include <errno.h>

int main(int argc, char* argv[])
{
  unsigned int BAUD = 100;
  if(argc > 1) {
    if(!sscanf(argv[1], "%5u", &BAUD) || BAUD > 15000 || BAUD < 1) {
      fprintf(stderr, "Invalid BAUD specified, using default...\n");
      BAUD = 100;
    }
  }
  printf("BAUD: %u\n", BAUD);
	if(SDL_Init(SDL_INIT_AUDIO) == -1)
	{
		printf("SDL_Init failed: %s\n", SDL_GetError());
		return -1;
	}
	atexit(SDL_Quit);

	if(init_me() == -1)
	{
		return -1;
	}
	printf("Audio modem terminal, to quit, press Ctrl-D, you may begin Tx/Rx now:\n\n");	
	
	while(1)
	{	
		int len		= 0;
		size_t rlen	= 0;
		char* tx_str	= NULL;	
		len = getline(&tx_str, &rlen, stdin);
	/*
		if(len==-1)
		{
			int er = errno;
			if(er==0)
				break;
			printf("Error #%d:%s\n", errno, strerror(er));
			break;
		}		
	*/				
		if(tx_str && (rlen+1 < 0xFFFF) && len!=-1)
		{
			if(transmit_string(tx_str, BAUD) == -1)
				printf("Failed to open audio interface!");
			
		}
		free(tx_str);
		tx_str = NULL;
		
	}
	cleanup();

	return 0;
}

