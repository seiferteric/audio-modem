all:
	gcc audio_modem.c tx_data.c -lSDL -lm -o send
	gcc rx_data.c -lasound -lm -o rec
