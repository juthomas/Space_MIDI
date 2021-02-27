#ifndef MIDI_H
# define MIDI_H
# include <stdio.h>
# include <stdlib.h>
# include <stdbool.h>

// TCP
# include <netdb.h> 
# include <netinet/in.h> 
# include <string.h> 
# include <sys/socket.h> 
# include <sys/types.h> 
# include <unistd.h>
# define PORT 3001 
void	tcp_connection();

// TCP

# define QUARTER 128 //noire
# define ON  0x90
# define OFF 0x80
# define C3  60
# define percu 9
# define reverb 0x5B
# define chorus 0x5D
# define phaser 0x5F

void	midi_test(char *filename);
void	MIDI_write_metadata(FILE *file, unsigned long tempo);
void	MIDI_tempo(FILE *file, unsigned long tempo);
void	MIDI_delta_time(FILE *fichier, unsigned long duree);
void	MIDI_write_variable_length_quantity(FILE *fichier, unsigned long i);
void	MIDI_Instrument_Change(FILE *fichier, unsigned char canal, unsigned char instrument);
void	MIDI_Note(FILE *fichier, unsigned char etat, unsigned char canal, unsigned char Note_MIDI, unsigned char velocite);
void	MIDI_only_one_note_with_duration(FILE *fichier, unsigned char canal, unsigned char Note_MIDI, unsigned char velocite, unsigned long duree);
void	MIDI_Control_Change(FILE *fichier, unsigned char canal, unsigned char type, unsigned char valeur);
void	MIDI_write_file_header(FILE *fichier, unsigned char SMF, unsigned short pistes, unsigned short nbdiv);
unsigned long MIDI_write_track_header(FILE *fichier);
void MIDI_write_end_of_track(FILE *fichier);
void MIDI_write_track_lengh(FILE *fichier, unsigned long marque);
void ecrire_piste2(FILE *fichier);



#endif