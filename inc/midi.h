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


#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/types.h>

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


#define SHM_KEY 0x1240


int errno;

typedef struct		s_sensors
{	
	uint32_t date;
	uint32_t time;
	uint16_t photodiode_1;	  // 0 - 4095
	uint16_t photodiode_2;	  // 0 - 4095
	uint16_t photodiode_3;	  // 0 - 4095
	uint16_t photodiode_4;	  // 0 - 4095
	uint16_t photodiode_5;	  // 0 - 4095
	uint16_t photodiode_6;	  // 0 - 4095
	uint16_t temperature_1;	  // 0 - 4095
	uint16_t temperature_2;	  // 0 - 4095
	uint16_t temperature_3;	  // 0 - 4095
	uint16_t temperature_4;	  // 0 - 4095
	uint16_t temperature_5;	  // 0 - 4095
	uint16_t temperature_6;	  // 0 - 4095
	uint16_t temperature_7;	  // 0 - 4095
	uint16_t temperature_8;	  // 0 - 4095
	uint16_t temperature_9;	  // 0 - 4095
	uint16_t temperature_10;  // 0 - 4095
	int8_t microphone;		  // 0 - 1
	uint16_t spectro_current; // 0 - 65535
	uint8_t organ_current;	  // 0 - 255
	uint16_t vin_current;	  // 0 - 65535//
	uint8_t q7_current;		  // 0 - 255
	uint8_t t5v_current;	  // 0 - 255
	uint8_t t3_3v_current;	  // 0 - 255
	uint16_t motor_current;	  // 0 - 65535
	uint8_t carousel_state;	  // 0 - 119
	uint8_t lid_state;		  // 0 - 53
	uint16_t organ_1;		  // 0 - 1023
	uint16_t organ_2;		  // 0 - 1023
	uint16_t organ_3;		  // 0 - 1023
	uint16_t organ_4;		  // 0 - 1023
	uint16_t organ_5;		  // 0 - 1023
	uint16_t organ_6;		  // 0 - 1023
	uint64_t timestamp;		  // 0 - oo
	struct s_sensors *next;
}					t_sensors;

typedef struct s_sensors2
{
   uint16_t photodiode_1;    // 0 - 4095
   uint16_t photodiode_2;    // 0 - 4095
   uint16_t photodiode_3;    // 0 - 4095
   uint16_t photodiode_4;    // 0 - 4095
   uint16_t photodiode_5;    // 0 - 4095
   uint16_t photodiode_6;    // 0 - 4095
   uint16_t temperature_1;   // 0 - 4095
   uint16_t temperature_2;   // 0 - 4095
   uint16_t temperature_3;   // 0 - 4095
   uint16_t temperature_4;   // 0 - 4095
   uint16_t temperature_5;   // 0 - 4095
   uint16_t temperature_6;   // 0 - 4095
   uint16_t temperature_7;   // 0 - 4095
   uint16_t temperature_8;   // 0 - 4095
   uint16_t temperature_9;   // 0 - 4095
   uint16_t temperature_10;  // 0 - 4095
   int8_t microphone;        // 0 - 1
   uint16_t spectro_current; // 0 - 65535
   uint8_t organ_current;    // 0 - 255
   uint16_t vin_current;     // 0 - 65535//
   uint8_t q7_current;       // 0 - 255
   uint8_t t5v_current;      // 0 - 255
   uint8_t t3_3v_current;    // 0 - 255
   uint16_t motor_current;   // 0 - 65535
   uint8_t carousel_state;   // 0 - 119
   uint8_t lid_state;        // 0 - 53
   uint16_t organ_1;         // 0 - 1023
   uint16_t organ_2;         // 0 - 1023
   uint16_t organ_3;         // 0 - 1023
   uint16_t organ_4;         // 0 - 1023
   uint16_t organ_5;         // 0 - 1023
   uint16_t organ_6;         // 0 - 1023
   uint64_t timestamp;       // 0 - oo
} t_sensors2;

typedef struct s_circular_buffer
{
   int16_t older_block;
   t_sensors2 data[42];
} t_circular_buffer;

struct shmseg
{
   int cnt;
   int complete;
   char buf[sizeof(t_circular_buffer)];
};


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
	uint64_t		data_time;
	uint32_t		delta_time; //TODO : replace all measures writed by this
	uint32_t		current_quarter_value;
	uint32_t		quarter_value_goal;
	uint32_t		quarter_value_step;

	float			quarter_value_step_updating;

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
void 	MIDI_write_track_length(FILE *fichier, uint32_t marque);



#endif
