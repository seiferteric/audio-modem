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

#include <alsa/asoundlib.h>
#include <stdio.h>
#include <math.h>
#define BAUD 9600

int main() {
	int rc;
	int size;
	snd_pcm_t *handle=NULL;
	snd_pcm_hw_params_t *params=NULL;
	unsigned int val = 48000;
	unsigned int den;
	snd_pcm_uframes_t frames = 1024;
	//unsigned int frames = 1024;
	short *buffer=NULL;

	rc = snd_pcm_open(&handle, "default", SND_PCM_STREAM_CAPTURE, 0);
	if (rc < 0)
	{
		fprintf(stderr, "unable to open pcm device: %s\n", snd_strerror(rc));
		exit(1);
	}

	snd_pcm_hw_params_alloca(&params);
	snd_pcm_hw_params_any(handle, params);
	snd_pcm_hw_params_set_access(handle, params, SND_PCM_ACCESS_RW_INTERLEAVED);
	snd_pcm_hw_params_set_format(handle, params, SND_PCM_FORMAT_S16_LE);
	snd_pcm_hw_params_set_channels(handle, params, 1);
	snd_pcm_hw_params_set_rate_near(handle, params, &val, 0);
	snd_pcm_hw_params_set_period_size_near(handle, params, &frames, 0);
	rc = snd_pcm_hw_params(handle, params);
	if (rc < 0)
	{
		fprintf(stderr, "unable to set hw parameters: %s\n", snd_strerror(rc));
		exit(1);
	}
	snd_pcm_hw_params_get_period_size(params, &frames, 0);
	snd_pcm_hw_params_get_rate_numden(params, &val, &den);
	size = frames * 2;
	
	printf("frames: %lu\n", frames);
	buffer = (short *) malloc(size);
	//snd_pcm_hw_params_get_period_time(params, &val, &dir);
	printf("samples/sec: %u/%u\n", val, den);
  	printf("BAUD: %d\n", BAUD);
	short time_buff[5];
	double tick = 0.0;
	memset(time_buff, 0, 5);
	unsigned char byte_rec=0;
	int msg=0;
	int bit=0;
	unsigned short msg_len=0;
	unsigned short msg_read=0;
	const double er = 1.0/(2.0*val);
	int calibrate	= 1;
	double ref_pow	= 1.0;
	char* msg_string = NULL;
	while (1) 
	{
		
		rc = snd_pcm_readi(handle, (void*)buffer, frames);
		
		
		if (rc == -EPIPE)
		{
			fprintf(stderr, "overrun occurred\n");
			snd_pcm_prepare(handle);
		} else if (rc < 0) 
		{
			fprintf(stderr, "error from read: %s\n", snd_strerror(rc));
		} else if (rc != (int)frames)
		{
			fprintf(stderr, "short read, read %d frames\n", rc);
		}

//Process--------------------------------------------------------------------------
	int offset = 0;

	while(offset < frames)
	{
			int i;
			double power =0.0;
		
			for(i=0;i<5;i++)
			{
	
				power += sqrt((double)time_buff[i]*time_buff[i])/5.0;
				if(i<4){time_buff[i]=time_buff[i+1];}

			}
			time_buff[4] = buffer[offset];

			tick += (1.0/val);
			if(tick>=1.0/BAUD - er)// && tick<=1.0/BAUD + er)
			{

		
					tick-=1.0/BAUD;

					if((power>=(ref_pow+6000)) && (!calibrate))
					{	
							byte_rec = byte_rec >> 1;
							byte_rec = byte_rec | 0x80;	
					}
					else
					{
							byte_rec = byte_rec >> 1;
							byte_rec = byte_rec & 0x7F;
							
					}
					if(calibrate)
					{
						static int c = 0;
						ref_pow += power/500;
						//if(power > ref_pow)	
						//	ref_pow = power;
						c++;
						if(c==500)
						{
							printf("Noise Threshold: %lf\n", ref_pow);
							calibrate=0;
						}
					}
					
					
					//if(byte_rec) printf("%x ", byte_rec);
					if(byte_rec == 0xAB && msg==0)
					{
						msg=1;
						printf("\nMsg: ");
						fflush(0);
					}
					if(msg==1)
					{
						if(bit==8)
						{
							bit = 1;
							switch(msg_read)
							{
								case 0:
								((unsigned char*)&msg_len)[0] = byte_rec;
								msg_read++;
								break;
								
								case 1:
								((unsigned char*)&msg_len)[1] = byte_rec;
								msg_string = malloc(msg_len+1);
								msg_string[msg_len] = '\0';
								msg_read++;
								break;
							
								default:
								msg_string[msg_read-2]=byte_rec;
								msg_read++;
								if(msg_read==msg_len+2)
								{
									printf("%s", msg_string);
									fflush(0);
									free(msg_string);
									msg_string	= NULL;
									msg		= 0;
									msg_len		= 0;
									bit		= 0;
									msg_read	= 0;
									byte_rec	= 0;
									break;
								}
								
								
								
							}
						}
						else
						{
							bit++;
						}
					}
					
						
				
			}
			offset++;
	}
//---------------------------------------------------------------------------------


    
	}

	snd_pcm_drain(handle);
	snd_pcm_close(handle);
	free(buffer);

	return 0;
}

