#include "../inc/midi.h"


void ecrire_piste2(FILE *file)
{
	uint64_t mark = MIDI_write_track_header(file);
	
	MIDI_Instrument_Change(file, 0, 90) ;
	for(int i=C3 ; i<=C3+12 ; i=i+1)
	{
		MIDI_only_one_note_with_duration(file, 1, i, 64, QUARTER) ;        
	}
	MIDI_delta_time(file, 0) ;
	MIDI_Note(file, ON, 1, C3, 64) ;

	MIDI_delta_time(file, 0) ;
	MIDI_Note(file, ON, 1, C3+2, 64);

	MIDI_delta_time(file, QUARTER*2) ;
	MIDI_Note(file, OFF, 1, C3, 0) ;
	MIDI_delta_time(file, QUARTER*2) ;
	MIDI_Note(file, OFF, 1, C3+2, 0) ;
	// for(int i=0 ; i<=127 ; i=i+1){
	// 	MIDI_Program_Change(file, 0, i) ;
	// 	Note_unique_avec_duree(file, 0, C3 + 9, 64, noire) ;        
	// }

	MIDI_write_end_of_track(file) ;
	MIDI_write_track_lengh(file, mark);
}


void	midi_test(char *filename)
{
	printf("Hello World !\n");
	FILE *fichier_midi = fopen(filename, "wb") ;
	MIDI_write_file_header(fichier_midi, 1, 2, QUARTER) ;
	//metadatas
	MIDI_write_metadata(fichier_midi, 500000);
	//musique
	ecrire_piste2(fichier_midi) ;
	fclose(fichier_midi) ; 
}

int main(int argc, char **argv)
{
	t_server_data server_data = {.is_setup = 0, .sockfd = 0, .light = 0, .temperature = 0};
	struct timeval tv;
	t_music_data music_data = {.measure_value=500000};

	tcp_connection(&server_data);


	gettimeofday(&music_data.last_measure ,NULL);
	gettimeofday(&tv, NULL);
	// uint32_t micro_time = gettimeofday_in_usecs();
	printf("secs : %ld\n", tv.tv_sec );
	printf("micros : %d\n", tv.tv_usec);
	for (int i = 0; i < 8;i++)
	{
		//Appel pour voir les valeurs du serv
		struct timeval tv_tmp;
		gettimeofday(&tv_tmp, NULL);
		uint32_t diff_value =
		(tv_tmp.tv_sec - music_data.last_measure.tv_sec) * 1000000 + 
		(tv_tmp.tv_usec - music_data.last_measure.tv_usec);
		
		//Rattraper les mesures de retard si besoin
		//Attendre le temps restant

		usleep(500000);
		printf("Paf : %d\n", i);
		printf("Diff : %u\n", diff_value);
		//Ecrire mesure
	}

	// tcp_connection(&server_data);

	midi_test(argc == 2 ? argv[1] : "output.mid");
}