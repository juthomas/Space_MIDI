#include "../../inc/midi.h"

void ecrire_piste1(FILE *fichier)
{   
	unsigned long marque = MIDI_ecrire_en_tete_piste(fichier) ;
	// 500000 = 0.5 sec de tempo (120 noires par min)
	MIDI_tempo(fichier, 500000) ;   
	MIDI_fin_de_la_piste(fichier) ;
	ecrire_taille_finale_piste(fichier, marque) ;    
}

void MIDI_tempo(FILE *fichier, unsigned long duree)
{
	MIDI_delta_time(fichier, 0) ;
	unsigned char octets[6] = {0xFF, 0x51, 0x03} ;
	octets[3] = duree >> 16 ;
	octets[4] = duree >> 8 ;
	octets[5] = duree ;
	fwrite(&octets, 6, 1, fichier) ;
}

void MIDI_delta_time(FILE *fichier, unsigned long duree)
{
	ecrire_variable_length_quantity(fichier, duree) ;
}

void ecrire_variable_length_quantity(FILE *fichier, unsigned long i)
{
	bool continuer ;
	if (i > 0x0FFFFFFF) {
		printf("ERREUR : delai > 0x0FFFFFFF ! \n") ;
		exit(EXIT_FAILURE) ;
	}
	
	unsigned long filo = i & 0x7F ;
	i = i >> 7 ;
	while (i != 0)
	{
		filo = (filo << 8)  + ((i & 0x7F) | 0x80) ;
		i = i >> 7 ;
	}
	
	do
	{
		fwrite(&filo, 1, 1, fichier) ;
		continuer = filo & 0x80 ;
		if (continuer)
		{
			filo = filo >> 8 ;
		}
	} while (continuer) ;
}

//changer d'instrument
void MIDI_Program_Change(FILE *fichier, unsigned char canal, unsigned char instrument)
{
	unsigned char octets[2] ;
	MIDI_delta_time(fichier, 0) ;
	octets[0] = 0xC0 + canal % 16 ;//16 canaux max
	octets[1] = instrument % 128 ;//128 instruments max
	fwrite(&octets, 2, 1, fichier) ;
}

void MIDI_Note(unsigned char etat, FILE *fichier, unsigned char canal, unsigned char Note_MIDI, unsigned char velocite)
{
	unsigned char octets[3] ;
	octets[0] = etat + canal % 16 ;
	octets[1] = Note_MIDI % 128 ;
	octets[2] = velocite % 128 ;//Volume
	fwrite(&octets, 3, 1, fichier) ;
}

void Note_unique_avec_duree(FILE *fichier, unsigned char canal, unsigned char Note_MIDI, unsigned char velocite, unsigned long duree)
{
	MIDI_delta_time(fichier, 0) ;
	MIDI_Note(ON,  fichier, canal, Note_MIDI, velocite) ;
	MIDI_delta_time(fichier, duree) ;
	MIDI_Note(OFF, fichier, canal, Note_MIDI, 0) ;
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
	
	MIDI_Program_Change(fichier, 0, 90) ;
	for(int i=C3 ; i<=C3+12 ; i=i+1){
		Note_unique_avec_duree(fichier, 1, i, 64, noire) ;        
	}
	MIDI_delta_time(fichier, 0) ;
	MIDI_Note(ON,  fichier, 1, C3, 64) ;

	MIDI_delta_time(fichier, 0) ;
	MIDI_Note(ON,  fichier, 1, C3+2, 64) ;

	MIDI_delta_time(fichier, noire*2) ;
	MIDI_Note(OFF, fichier, 1, C3, 0) ;
	MIDI_delta_time(fichier, noire*2) ;
	MIDI_Note(OFF, fichier, 1, C3+2, 0) ;





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
	ecrire_piste1(fichier_midi);
	//musique
	ecrire_piste2(fichier_midi) ;
	fclose(fichier_midi) ; 
}