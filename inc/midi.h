#ifndef MIDI_H
# define MIDI_H
# include <stdio.h>
# include <stdlib.h>
# include <stdbool.h>
# include <sys/time.h>

// TCP
# include <netdb.h> 
# include <netinet/in.h> 
# include <string.h> 
# include <sys/socket.h> 
# include <sys/types.h> 
# include <unistd.h>
# define PORT 3001 

// TCP

# include "./midi_notes.h"
# define QUARTER 128 //noire
# define ON  0x90
# define OFF 0x80
// # define C3  60
# define percu 9
# define reverb 0x5B
# define chorus 0x5D
# define phaser 0x5F

typedef struct		s_server_data
{
	uint8_t			is_setup;
	int				sockfd;
	int				connfd;
	int				read_state;
	int32_t			temperature;
	int32_t			light;
	int32_t			motors_activity;
	int32_t			vibrations;

}					t_server_data;

typedef struct		s_music_data
{
	uint32_t		quarter_value;
	FILE			*midi_file;
	uint32_t		midi_mark;
	uint32_t		partition_duration;
	uint32_t		measure_value;
	uint32_t		measures_writed;
	struct timeval	last_measure;
	struct timeval	entry_time;
	
}					t_music_data;
void wait_for_connection(t_server_data *data);
int32_t tcp_get_fresh_data(t_server_data *data);
void	tcp_connect(t_server_data *data);
// t_server_data		g_server_data;

void	midi_test(char *filename);
void	MIDI_write_metadata(FILE *file, uint32_t tempo);
void	MIDI_tempo(FILE *file, uint32_t tempo);
void	MIDI_delta_time(FILE *fichier, uint32_t duree);
void	MIDI_write_variable_length_quantity(FILE *fichier, uint32_t i);
void	MIDI_Instrument_Change(FILE *fichier, unsigned char canal, unsigned char instrument);
void	MIDI_Note(FILE *fichier, unsigned char etat, unsigned char canal, unsigned char Note_MIDI, unsigned char velocite);
void	MIDI_only_one_note_with_duration(FILE *fichier, unsigned char canal, unsigned char Note_MIDI, unsigned char velocite, unsigned long duree);
void	MIDI_Control_Change(FILE *fichier, unsigned char canal, unsigned char type, unsigned char valeur);
void	MIDI_write_file_header(FILE *fichier, unsigned char SMF, unsigned short pistes, unsigned short nbdiv);
uint32_t MIDI_write_track_header(FILE *fichier);
void MIDI_write_end_of_track(FILE *fichier);
void MIDI_write_track_lengh(FILE *fichier, uint32_t marque);
void ecrire_piste2(FILE *fichier);



#endif