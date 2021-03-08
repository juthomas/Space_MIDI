#include "../inc/midi.h"




void	midi_setup_file(char *filename, t_music_data *music_data)
{
	music_data->midi_file = fopen(filename, "wb") ;
	MIDI_write_file_header(music_data->midi_file, 1, 2, QUARTER) ;
	//metadatas
	MIDI_write_metadata(music_data->midi_file, music_data->quarter_value);
	music_data->midi_mark = MIDI_write_track_header(music_data->midi_file);
	MIDI_Instrument_Change(music_data->midi_file, 0, 90) ;

}

void	midi_write_measure_note(t_music_data *music_data, unsigned char state, \
		unsigned char channel, unsigned char note, unsigned char velocity)
{
	MIDI_delta_time(music_data->midi_file, 0) ;
	MIDI_Note(music_data->midi_file, state, channel, note, velocity) ;

}

void	midi_write_measure(t_music_data *music_data, \
			uint32_t measures_to_write)
{
	// T = 1/4
	//Code part
	midi_write_measure_note(music_data, ON, 1, A3, 64);

	//Code part
	MIDI_delta_time(music_data->midi_file, 0) ;
	MIDI_Note(music_data->midi_file, OFF, 1, 10, 64) ;

	// T = 2/4
	MIDI_delta_time(music_data->midi_file, QUARTER) ;
	MIDI_Note(music_data->midi_file, OFF, 1, 10, 0) ;
	//Code part
	midi_write_measure_note(music_data, OFF, 1, A3, 0);

	midi_write_measure_note(music_data, ON, 1,
	A0, 128);

	//Code part
	MIDI_delta_time(music_data->midi_file, 0) ;
	MIDI_Note(music_data->midi_file, OFF, 1, 10, 0) ;

	// T = 3/4

	MIDI_delta_time(music_data->midi_file, QUARTER) ;
	MIDI_Note(music_data->midi_file, OFF, 1, 10, 0) ;
	//Code part

	midi_write_measure_note(music_data, OFF, 1, A0, 0);
	

		midi_write_measure_note(music_data, ON, 1,
		A0, 128);

	//Code part
	MIDI_delta_time(music_data->midi_file, 0) ;
	MIDI_Note(music_data->midi_file, OFF, 1, 10, 0) ;

	// T = 4/4
	MIDI_delta_time(music_data->midi_file, QUARTER) ;
	MIDI_Note(music_data->midi_file, OFF, 1, 10, 0) ;
	//Code part
	midi_write_measure_note(music_data, OFF, 1, A0, 0);

	//Code part
	MIDI_delta_time(music_data->midi_file, 0) ;
	MIDI_Note(music_data->midi_file, OFF, 1, 10, 0) ;

	// T = end
	MIDI_delta_time(music_data->midi_file, QUARTER) ;
	MIDI_Note(music_data->midi_file, OFF, 1, 10, 0) ;
	// MIDI_delta_time(music_data->midi_file, 0) ;
	// MIDI_Note(music_data->midi_file, OFF, 1, 10, 0) ;

	// MIDI_Note(music_data->midi_file, OFF, 1, C3, 0) ;

}

void	midi_write_end(t_music_data *music_data)
{
	MIDI_write_end_of_track(music_data->midi_file);
	MIDI_write_track_lengh(music_data->midi_file, music_data->midi_mark);
	fclose(music_data->midi_file); 
}

void	terminate_session(int signal, void *ptr)
{

}


//probably replaced by a simple strcmp
int8_t	cmp_filename(struct dirent* file1, struct dirent* file2)
{
	char* name1tmp = file1->d_name;

	printf("Name = %s\n", name1tmp);
	uint32_t DD = strtol(name1tmp, &name1tmp, 10);
	if (*name1tmp == '_')
	{
		name1tmp++;
	}
	else
	{
		return (-1);
	}
	uint32_t MM = strtol(name1tmp, &name1tmp, 10);
	if (*name1tmp == '_')
	{
		name1tmp++;
	}
	else
	{
		return (-1);
	}
	uint32_t YY = strtol(name1tmp, &name1tmp, 10);
	if (*name1tmp == '_' && *(name1tmp + 1) == '_')
	{
		(name1tmp)+=2;
	}
	else
	{
		return (-1);
	}
	uint32_t HH = strtol(name1tmp, &name1tmp, 10);
	if (*name1tmp == '_')
	{
		name1tmp++;
	}
	else
	{
		return (-1);
	}
	uint32_t mm = strtol(name1tmp, &name1tmp, 10);
	if (*name1tmp == '_')
	{
		name1tmp++;
	}
	else
	{
		return (-1);
	}
	uint32_t SS = strtol(name1tmp, &name1tmp, 10);
	printf("Time : %d/%d/%d %d:%d:%d\n", DD, MM, YY, HH, mm, SS);

}

int main(int argc, char **argv)
{
	signal(SIGTERM, (void (*)(int))terminate_session);
	t_music_data music_data = {.partition_duration=40000000,
	.measure_value=500000 * 4 * 2,
	.measures_writed=0,
	.quarter_value=500000 * 2
	};
	midi_setup_file("Test.midi", &music_data);

	DIR* rep = NULL;
	struct dirent* currentFile = NULL;
	struct dirent* tmpFile = NULL;
	rep = opendir("./data_files");
	if (rep == NULL) /* Si le dossier n'a pas pu être ouvert */
	{
		exit(1);
	}
	
	while ((currentFile = readdir(rep)) != NULL)
	{
		printf("Le fichier lu s'appelle '%s'\n", currentFile->d_name);
		cmp_filename(currentFile, NULL);

	}

	for (;;)
	{
	//	regarder dans le dossier des inputs Json
	//	Si il y a des fichiers qui correspondent a ceux des datas
		// {
		//		Parser le Json
		//		Si Fichier midi pas encore cree 
				// {
					// Creer un nouveau fichier Midi
				// }
				// Tant que le json est pas fini
				// {
					// Creer nouveau fichier midi si il existe pas encore 
					// faire des mesures 
					// {
						// Creer en fonction des donnees du json + trucs dans la struct
					// }
					
					// Si c'est la fin du midi (defini dans un poich), save, en ouvrir un nouveau
				// }
				//detruire le json
		// }
	}


	// Signaux -> 
	// Fini le midi, (lui donne un nom avec date debut/fin)
	// Note dans les LOGS le temps d'arret midi et log
	// Free
	// exit





	// midi_write_measure(&server_data, &music_data, get_measures_to_write(music_data, tv_tmp));
	midi_write_end(&music_data);
}