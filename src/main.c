#include "../inc/midi.h"

void midi_setup_file(char *filename, t_music_data *music_data)
{
	music_data->midi_file = fopen(filename, "wb");
	MIDI_write_file_header(music_data->midi_file, 1, 2, QUARTER);
	//metadatas
	MIDI_write_metadata(music_data->midi_file, music_data->quarter_value);
	music_data->midi_mark = MIDI_write_track_header(music_data->midi_file);
	MIDI_Instrument_Change(music_data->midi_file, 0, 90);
}

void midi_write_measure_note(t_music_data *music_data, unsigned char state,
							 unsigned char channel, unsigned char note, unsigned char velocity)
{
	MIDI_delta_time(music_data->midi_file, 0);
	MIDI_Note(music_data->midi_file, state, channel, note, velocity);
}

void midi_write_measure(t_music_data *music_data,
						uint32_t measures_to_write)
{
	// T = 1/4
	//Code part
	midi_write_measure_note(music_data, ON, 1, A3, 64);

	//Code part
	MIDI_delta_time(music_data->midi_file, 0);
	MIDI_Note(music_data->midi_file, OFF, 1, 10, 64);

	// T = 2/4
	MIDI_delta_time(music_data->midi_file, QUARTER);
	MIDI_Note(music_data->midi_file, OFF, 1, 10, 0);
	//Code part
	midi_write_measure_note(music_data, OFF, 1, A3, 0);

	midi_write_measure_note(music_data, ON, 1,
							A0, 128);

	//Code part
	MIDI_delta_time(music_data->midi_file, 0);
	MIDI_Note(music_data->midi_file, OFF, 1, 10, 0);

	// T = 3/4

	MIDI_delta_time(music_data->midi_file, QUARTER);
	MIDI_Note(music_data->midi_file, OFF, 1, 10, 0);
	//Code part

	midi_write_measure_note(music_data, OFF, 1, A0, 0);

	midi_write_measure_note(music_data, ON, 1,
							A0, 128);

	//Code part
	MIDI_delta_time(music_data->midi_file, 0);
	MIDI_Note(music_data->midi_file, OFF, 1, 10, 0);

	// T = 4/4
	MIDI_delta_time(music_data->midi_file, QUARTER);
	MIDI_Note(music_data->midi_file, OFF, 1, 10, 0);
	//Code part
	midi_write_measure_note(music_data, OFF, 1, A0, 0);

	//Code part
	MIDI_delta_time(music_data->midi_file, 0);
	MIDI_Note(music_data->midi_file, OFF, 1, 10, 0);

	// T = end
	MIDI_delta_time(music_data->midi_file, QUARTER);
	MIDI_Note(music_data->midi_file, OFF, 1, 10, 0);
	// MIDI_delta_time(music_data->midi_file, 0) ;
	// MIDI_Note(music_data->midi_file, OFF, 1, 10, 0) ;

	// MIDI_Note(music_data->midi_file, OFF, 1, C3, 0) ;
}

void midi_write_end(t_music_data *music_data)
{
	MIDI_write_end_of_track(music_data->midi_file);
	MIDI_write_track_lengh(music_data->midi_file, music_data->midi_mark);
	fclose(music_data->midi_file);
}

void terminate_session(int signal, void *ptr)
{
}

//probably replaced by a simple strcmp
int8_t cmp_filename(struct dirent *file1, struct dirent *file2)
{
	char *name1tmp = file1->d_name;

	printf("Name = %s\n", name1tmp);
	uint32_t DD = 0;

	uint32_t MM = 0;

	uint32_t YY = 0;

	uint32_t HH = 0;

	uint32_t mm = 0;

	uint32_t SS = 0;

	uint32_t ret = 0;

	ret = sscanf(name1tmp, "%d_%d_%d__%d_%d_%d.json", &YY, &MM, &DD, &HH, &mm, &SS);

	printf("ret : %d\n", ret);
	printf("Time : %d/%d/%d %d:%d:%d\n", DD, MM, YY, HH, mm, SS);
}

void json_deserialize(uint32_t file_length, char *file_content)
{
	jsmn_parser p;
	jsmntok_t t[1280];
	uint32_t r;
	uint32_t i;

	jsmn_init(&p);
	r = jsmn_parse(&p, file_content, file_length, t,
				   sizeof(t) / sizeof(t[0]));
	if (r < 0)
	{
		printf("Failed to parse JSON: %d\n", r);
		return;
	}

	/* Assume the top-level element is an object */
	if (r < 1 || t[0].type != JSMN_OBJECT)
	{
		printf("Object expected\n");
		return;
	}
	uint32_t	nu_of_measures = 0;
	uint32_t	obj_size;
	for (i = 1; i < r; i++)
	{
		// printf("type maillon %d : %d, taille : %d\n", i, t[i].type, t[i].size);
		// printf("%.*s\n", t[i].end - t[i].start,
			//    file_content + t[i].start);
		if (t[i].type == JSMN_OBJECT && t[i].size == 6)
		{
			obj_size = t[i].size;
			i++;
			if (JSON_cmp(file_content, &t[i], "Time") == 0)
			{
				printf("- Time: %.*s\n", t[i + 1].end - t[i + 1].start,
					file_content + t[i + 1].start);
			}



		}
		
	}
	printf("Number of measures : %d\n", nu_of_measures);
}

int main(int argc, char **argv)
{
	signal(SIGTERM, (void (*)(int))terminate_session);
	t_music_data music_data = {.partition_duration = 40000000,
							   .measure_value = 500000 * 4 * 2,
							   .measures_writed = 0,
							   .quarter_value = 500000 * 2};
	midi_setup_file("Test.midi", &music_data);

	FILE *file_ptr;
	DIR *rep = NULL;
	struct dirent *currentFile = NULL;
	struct dirent *tmpFile = NULL;
	rep = opendir("./data_files");
	if (rep == NULL) /* Si le dossier n'a pas pu être ouvert */
	{
		exit(1);
	}

	while ((currentFile = readdir(rep)) != NULL)
	{
		printf("Le fichier lu s'appelle '%s'\n", currentFile->d_name);
		// sscanf("")

		cmp_filename(currentFile, NULL);
	}

	char *file_content;
	if (!(file_ptr = fopen("./data_files/2021_03_09__23_13_24.json", "r")))
	{
		printf("Error while creating file\n");
		exit(-1);
	}
	printf("Hello world bis\n");
	uint32_t file_length;
	fseek(file_ptr, 0, SEEK_END);
	file_length = ftell(file_ptr);
	fseek(file_ptr, 0, SEEK_SET);
	file_content = malloc(file_length);
	fread(file_content, 1, file_length, file_ptr);

	// fgets(file_content, 1000, file_ptr);
	printf("File content : %s\n", file_content);
	printf("End of content\n");
	json_deserialize(file_length, file_content);
	fclose(file_ptr);
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