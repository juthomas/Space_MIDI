#include "../../inc/midi.h"

/**
  * Write the details of the function here
  * @param [in] (input parameter description, including the role of each parameter, the value and the relationship between parameters)
  * @param [out] (description of output parameters)
  * @return (description of return value)
  * @see (this function refers to other related functions, here as a link)
  * @note (description of the need to pay attention to the problem)
*/
void useless_function()
{

}

/**
  * Write metadata Midi line
  * @param [file] Midi file pointer
  * @param [tempo] quarter note (noire) value in micro-seconds
*/
void write_metadata(FILE *file, unsigned long tempo)
{
	unsigned long header_index = MIDI_ecrire_en_tete_piste(file);
	// 500000 = 0.5 sec de tempo (120 noires par min)
	MIDI_tempo(file, tempo);
	MIDI_fin_de_la_piste(file);
	ecrire_taille_finale_piste(file, header_index);
}

/**
  * Write tempo in Midi line
  * @param [file] Midi file pointer
  * @param [tempo] quarter note (noire) value in micro-seconds
*/
void MIDI_tempo(FILE *file, unsigned long tempo)
{
	MIDI_delta_time(file, 0) ;
	unsigned char bytes[6] = {0xFF, 0x51, 0x03} ;
	bytes[3] = tempo >> 16 ;
	bytes[4] = tempo >> 8 ;
	bytes[5] = tempo ;
	fwrite(&bytes, 6, 1, file) ;
}

/**
  * Write Delta time
  * @param [file] Midi file pointer
  * @param [duration] Waiting duration in micro-seconds
*/
void MIDI_delta_time(FILE *file, unsigned long duration)
{
	write_variable_length_quantity(file, duration) ;
}

/**
  * Write variable lenght quantity
  * @param [file] Midi file pointer
  * @param [duration] Waiting duration in micro-seconds
*/
void write_variable_length_quantity(FILE *file, unsigned long duration)
{
	bool pass;
	
	if (duration > 0x0FFFFFFF) {
		printf("ERROR : delay > 0x0FFFFFFF ! \n") ;
		exit(EXIT_FAILURE) ;
	}
	
	unsigned long filo = duration & 0x7F ;
	duration = duration >> 7 ;
	while (duration != 0)
	{
		filo = (filo << 8)  + ((duration & 0x7F) | 0x80) ;
		duration = duration >> 7 ;
	}
	
	do
	{
		fwrite(&filo, 1, 1, file) ;
		pass = filo & 0x80 ;
		if (pass)
		{
			filo = filo >> 8 ;
		}
	} while (pass) ;
}

/**
  * Instrument change or instrument selection
  * @param [file] Midi file pointer
  * @param [channel] Selection of midi channel (0-16)
  * @param [instrument] Selection of midi instrument (1-128)
*/
void MIDI_Instrument_Change(FILE *fichier, unsigned char channel, unsigned char instrument)
{
	unsigned char octets[2] ;
	MIDI_delta_time(fichier, 0) ;
	octets[0] = 0xC0 + channel % 16 ;//16 canaux max
	octets[1] = instrument % 128 ;//128 instruments max
	fwrite(&octets, 2, 1, fichier) ;
}

/**
  * Instrument change or instrument selection
  * @param [file] Midi file pointer
  * @param [state] Logic state of note (ON/OFF)
  * @param [channel] Selection of midi channel (0-16)
  * @param [MIDI_note] Selection of midi note (1-127)
  * @param [velocity] Selection of velocity (power) (1-127)
*/
void MIDI_Note(FILE *file, unsigned char state, unsigned char channel, unsigned char MIDI_note, unsigned char velocity)
{
	unsigned char bytes[3] ;
	bytes[0] = state + channel % 16 ;
	bytes[1] = MIDI_note % 128 ;
	bytes[2] = velocity % 128 ;//Volume
	fwrite(&bytes, 3, 1, file) ;
}

/**
  * One note pressed with duration
  * @param [file] Midi file pointer
  * @param [state] Logic state of note (ON/OFF)
  * @param [channel] Selection of midi channel (0-16)
  * @param [MIDI_note] Selection of midi note (1-127)
  * @param [velocity] Selection of velocity (power) (1-127)
*/
void Only_one_note_with_duration(FILE *file, unsigned char channel, unsigned char MIDI_note, unsigned char velocity, unsigned long duration)
{
	MIDI_delta_time(file, 0) ;
	MIDI_Note(file, ON, channel, MIDI_note, velocity) ;
	MIDI_delta_time(file, duration) ;
	MIDI_Note(file, OFF, channel, MIDI_note, 0) ;
}

// parametrage midi (attack, pitch, release, etc..)
void MIDI_Control_Change(FILE *fichier, unsigned char canal, unsigned char type, unsigned char valeur)
{
	unsigned char octets[3] ;
	MIDI_delta_time(fichier, 0) ;
	octets[0] = 0xB0 + canal % 16 ;
	octets[1] = type % 128 ;
	octets[2] = valeur % 128 ;
	fwrite(&octets, 3, 1, fichier) ;
}

void MIDI_ecrire_en_tete(FILE *fichier, unsigned char SMF, unsigned short pistes, unsigned short nbdiv)
{
	if ((SMF == 0) && (pistes > 1)) {
		printf("ERREUR : une seule piste possible en SMF 0 ! \n") ;
		exit(EXIT_FAILURE) ;
	}
	// MThd
	unsigned char octets[14] = {0x4d, 0x54, 0x68, 0x64, 0x00, 0x00, 0x00, 0x06} ;
	octets[8]  = 0 ; //
	octets[9]  = SMF ; // Type de fichier midi (0, 1 ou 2)
	octets[10] = pistes >> 8 ; // Nombre de pistes (j'usqu'a 0xFFFF)
	octets[11] = pistes ;
	octets[12] = nbdiv  >> 8 ; // 0x8000 pour SMPTE sinon nb de ticks pour une noire
	octets[13] = nbdiv ;     // Nombre de divisions de la noire
	fwrite(&octets, 14, 1, fichier) ;
}

unsigned long MIDI_ecrire_en_tete_piste(FILE *fichier)
{
	//MTrk
	unsigned char octets[8] = {0x4d, 0x54, 0x72, 0x6b, 0x00, 0x00, 0x00, 0x00} ;
	fwrite(&octets, 8, 1, fichier) ;
	return ftell(fichier) ;
}

void MIDI_fin_de_la_piste(FILE *fichier)
{
	MIDI_delta_time(fichier, 0) ;
	unsigned char octets[3] = {0xFF, 0x2F, 0x00} ;
	fwrite(&octets, 3, 1, fichier) ;
}

void ecrire_taille_finale_piste(FILE *fichier, unsigned long marque)
{
	unsigned char octets[4] ;
	unsigned long taille = ftell(fichier) - marque ;
	fseek(fichier, marque-4, SEEK_SET) ;    // On rembobine
	octets[0] = taille >> 24 ;
	octets[1] = taille >> 16 ;
	octets[2] = taille >> 8 ;
	octets[3] = taille ;
	fwrite(&octets, 4, 1, fichier) ;
	fseek(fichier, 0, SEEK_END) ;
}

void ecrire_piste2(FILE *fichier)
{
	unsigned long marque = MIDI_ecrire_en_tete_piste(fichier) ;
	
	MIDI_Instrument_Change(fichier, 0, 90) ;
	for(int i=C3 ; i<=C3+12 ; i=i+1){
		Only_one_note_with_duration(fichier, 1, i, 64, noire) ;        
	}
	MIDI_delta_time(fichier, 0) ;
	MIDI_Note(fichier, ON, 1, C3, 64) ;

	MIDI_delta_time(fichier, 0) ;
	MIDI_Note(fichier, ON, 1, C3+2, 64) ;

	MIDI_delta_time(fichier, noire*2) ;
	MIDI_Note(fichier, OFF, 1, C3, 0) ;
	MIDI_delta_time(fichier, noire*2) ;
	MIDI_Note(fichier, OFF, 1, C3+2, 0) ;





	// for(int i=0 ; i<=127 ; i=i+1){
	// 	MIDI_Program_Change(fichier, 0, i) ;
	// 	Note_unique_avec_duree(fichier, 0, C3 + 9, 64, noire) ;        
	// }
	
	MIDI_fin_de_la_piste(fichier) ;
	ecrire_taille_finale_piste(fichier, marque) ;    
}


void	midi_test(char *filename)
{
	printf("Hello World !\n");
	FILE *fichier_midi = fopen(filename, "wb") ;
	MIDI_ecrire_en_tete(fichier_midi, 1, 2, noire) ;
	//metadatas
	write_metadata(fichier_midi, 500000);
	//musique
	ecrire_piste2(fichier_midi) ;
	fclose(fichier_midi) ; 
}