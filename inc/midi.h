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
	int16_t				photodiode_1;
	int16_t				photodiode_2;
	int16_t				photodiode_3;
	int16_t				photodiode_4;
	int16_t				photodiode_5;
	int16_t				photodiode_6;
	int16_t				temperature_1;
	int16_t				temperature_2;
	int16_t				temperature_3;
	int16_t				temperature_4;
	int16_t				temperature_5;
	int16_t				temperature_6;
	int16_t				temperature_7;
	int16_t				temperature_8;
	int16_t				temperature_9;
	int16_t				temperature_10;
	int8_t				first_sample;
	float				spectro_current;
	float				organ_current;
	float				vin_current;
	float				q7_current;
	float				t5v_current;
	float				t3_3v_current;
	float				motor_current;
	int16_t				carousel_state;
	int16_t				lid_state;
	int32_t				spectrum;
	int16_t				organ_1;
	int16_t				organ_2;
	int16_t				organ_3;
	int16_t				organ_4;
	int16_t				organ_5;
	int16_t				organ_6;
	struct s_sensors	*next;
}					t_sensors;


typedef struct		s_music_data
{
	uint32_t		quarter_value;// valeur d'une noire pour les metadatas
	FILE			*midi_file;
	FILE			*midi_file_redundancy;
	uint32_t		midi_mark;
	uint32_t		midi_mark_redundancy;
	uint32_t		partition_duration;
	uint32_t		measure_value;//
	uint32_t		measures_writed;//
	uint32_t		data_time;
	uint32_t		delta_time; //TODO : replace all measures writed by this
	uint32_t		current_quarter_value;
	uint32_t		quarter_value_goal;
	uint32_t		quarter_value_step;
	uint32_t		entry_data_time;
	struct timeval	last_measure;//
	struct timeval	entry_time;//
	
}					t_music_data;

typedef struct		s_note
{
	uint8_t			beg_eighth;
	uint8_t			eighth_duration;
	uint8_t			velocity;
	uint8_t			channel;
	uint8_t			note;
	uint8_t			active;
}					t_note;

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
