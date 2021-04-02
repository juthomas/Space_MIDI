#ifndef MIDI_H
# define MIDI_H
# include <stdio.h>
# include <stdlib.h>
# include <stdbool.h>
# include <sys/time.h>
# include <string.h> 
# include <unistd.h>
# include <errno.h>
# include <sys/stat.h> 
# include <stdint.h>
# include <time.h> //ADD raspbian
# include <signal.h> //ADD raspbian

// Folder, (ls)
# include <dirent.h> 

// TCP ??
// # include <netdb.h> 
//# include <netinet/in.h> 
// # include <sys/socket.h> 
// # include <sys/types.h> 

// # define PORT 3001 


// JSON
// #include "./json_parser.h"


// MIDI
// # include "./midi_notes.h"
# define QUARTER 128 //noire
# define ON  0x90
# define OFF 0x80
// # define C3  60
# define percu 9
# define reverb 0x5B
# define chorus 0x5D
# define phaser 0x5F

int errno;

typedef struct		s_sensors
{	
	uint32_t			date;
	uint32_t			time;
	float				photodiode_1;
	float				photodiode_2;
	float				photodiode_3;
	float				photodiode_4;
	float				photodiode_5;
	float				photodiode_6;
	float				temperature_1;
	float				temperature_2;
	float				temperature_3;
	float				temperature_4;
	float				temperature_5;
	float				temperature_6;
	float				temperature_7;
	float				temperature_8;
	float				temperature_9;
	float				temperature_10;
	int8_t				lid_state;
	int8_t				first_sample;
	float				spectro_current;
	float				electro_current;
	float				organ_current;
	float				vin_current;
	float				q7_current;
	float				t5v_current;
	float				t3_3v_current;
	float				motor_current;
	int32_t				position_360;
	int32_t				spectrum;
	float				organ;
	struct s_sensors	*next;
}					t_sensors;


typedef struct		s_music_data
{
	uint32_t		quarter_value;
	FILE			*midi_file;
	uint32_t		midi_mark;
	uint32_t		partition_duration;
	uint32_t		measure_value;
	uint32_t		measures_writed;
	uint32_t		data_time;

	uint32_t		entry_data_time;
	struct timeval	last_measure;//
	struct timeval	entry_time;//
	
}					t_music_data;

//New header + static?							//durée d'une partition 40 000 000us
static t_music_data g_music_data = {.partition_duration = 40000000,
								//Measure value = quarter value * 4 (4/4) (4 noires par mesure)
							   .measure_value = 500000 * 4,
							   .measures_writed = 0,
							   // valeur d'une noire en us (pour le tempo)
							   .quarter_value = 500000 };;

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
void 	MIDI_write_end_of_track(FILE *fichier);
void 	MIDI_write_track_lengh(FILE *fichier, uint32_t marque);



#endif
