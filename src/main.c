#include "../inc/midi.h"
#include "../inc/json_parser.h"
#include "../inc/midi_notes.h"

/**
  * @brief Create and open a new midi file
  * @param [filename] futur name of midi file
  * @param [music_data] Midi struct
*/
void midi_setup_file(char *filename, t_music_data *music_data)
{
	music_data->midi_file = fopen(filename, "wb");
	MIDI_write_file_header(music_data->midi_file, 1, 2, QUARTER);
	//metadatas
	MIDI_write_metadata(music_data->midi_file, music_data->quarter_value);
	music_data->midi_mark = MIDI_write_track_header(music_data->midi_file);
	MIDI_Instrument_Change(music_data->midi_file, 0, 90);
}

/**
  * @brief Write a note state into "midi_write_measure" function
  * @param [state] Logic state of note (ON/OFF)
  * @param [channel] Selection of midi channel (0-16)
  * @param [note] Selection of midi note (1-127)
  * @param [velocity] Selection of velocity (power) (1-127)
*/
void midi_write_measure_note(t_music_data *music_data, unsigned char state,
							 unsigned char channel, unsigned char note, unsigned char velocity)
{
	MIDI_delta_time(music_data->midi_file, 0);
	MIDI_Note(music_data->midi_file, state, channel, note, velocity);
}

/**
  * @brief Simply wait for a quarter of measure
  * @param [music_data] Midi struct
*/
void midi_delay_quarter(t_music_data *music_data)
{
	MIDI_delta_time(music_data->midi_file, 0);
	MIDI_Note(music_data->midi_file, OFF, 1, 10, 64);

	MIDI_delta_time(music_data->midi_file, QUARTER);
	MIDI_Note(music_data->midi_file, OFF, 1, 10, 0);
}

/**
  * @brief Function to write an 4 stroke measure
  * @param [music_data] Midi struct
  * @param [sensors_data] Struct that contain current sensors datas
*/
void midi_write_measure(t_music_data *music_data, t_sensors *sensors_data)
{
	printf("In midi Sensors data : \n");
	printf("date : %d Time : %d\n", sensors_data->date, sensors_data->time);
	printf("Music Time : %d\n", music_data->data_time);
	printf("photodiode : %f\n", sensors_data->photodiode);
	printf("temperature : %f\n", sensors_data->temperature);
	printf("comsumption : %f\n", sensors_data->comsumption);
	printf("position : %f\n", sensors_data->position);
	printf("orgue : %f\n", sensors_data->orgue);

	printf("\n\n");

//    ====================================================
//  ||                                                    ||
//  ||                      T = 1/4                       ||
//  ||                                                    ||
	midi_write_measure_note(music_data, ON, 1, A3, 64);
//  ||                                                    ||
//  ||                      T = 1/4                       ||
//  ||                                                    ||
//    ====================================================

	midi_delay_quarter(music_data);
	
//    ====================================================
//  ||                                                    ||
//  ||                      T = 2/4                       ||
//  ||                                                    ||
	midi_write_measure_note(music_data, OFF, 1, A3, 0);

	midi_write_measure_note(music_data, ON, 1,
							(sensors_data->photodiode - 40) * 3 + 50 , 64);
//  ||                                                    ||
//  ||                      T = 2/4                       ||
//  ||                                                    ||
//    ====================================================

	midi_delay_quarter(music_data);

//    ====================================================
//  ||                                                    ||
//  ||                      T = 3/4                       ||
//  ||                                                    ||
	midi_write_measure_note(music_data, OFF, 1, (sensors_data->photodiode - 40) * 3 + 50, 0);
	midi_write_measure_note(music_data, ON, 1,
							(sensors_data->temperature - 40) * 3 + 50, 64);
//  ||                                                    ||
//  ||                      T = 3/4                       ||
//  ||                                                    ||
//    ====================================================

	midi_delay_quarter(music_data);

//    ====================================================
//  ||                                                    ||
//  ||                      T = 4/4                       ||
//  ||                                                    ||
	midi_write_measure_note(music_data, OFF, 1, (sensors_data->temperature - 40) * 3 + 50, 0);
	midi_write_measure_note(music_data, ON, 1,
							(sensors_data->comsumption - 40) * 3 + 50, 64);
//  ||                                                    ||
//  ||                      T = 4/4                       ||
//  ||                                                    ||
//    ====================================================

	midi_delay_quarter(music_data);

//    ====================================================
//  ||                                                    ||
//  ||                      T = END                       ||
//  ||                                                    ||
	midi_write_measure_note(music_data, OFF, 1, (sensors_data->comsumption - 40) * 3 + 50, 0);
//  ||                                                    ||
//  ||                      T = END                       ||
//  ||                                                    ||
//    ====================================================
}

/**
  * @brief Write end of midi file (and close it)
  * @param [music_data] Midi struct of an opened midi file
*/
void midi_write_end(t_music_data *music_data)
{
	MIDI_write_end_of_track(music_data->midi_file);
	MIDI_write_track_lengh(music_data->midi_file, music_data->midi_mark);
	fclose(music_data->midi_file);
	music_data->midi_file = NULL;// new
}

/**
  * @brief Called when signal "SIGTERM" is sended
  * @param [signal] Signal Number (Probably 15)
*/
void terminate_session(int signal)
{
	if (g_music_data.midi_file)
	{
		midi_write_end(&g_music_data);
	}
	exit(0);
}

/**
  * @brief Convert an string date to integer date and time
  * @param [data_time] date to convert (format "YY/MM/DD HH:mm:ss")
  * @param [date] date output (format "YYYYMMDD")
  * @param [time] time output (format "HHmmSS")
*/
uint8_t date_time_to_date_and_time(char *date_time, uint32_t *date, uint32_t *time)
{
	uint32_t DD = 0;
	uint32_t mm = 0;
	uint32_t YY = 0;
	uint32_t HH = 0;
	uint32_t MM = 0;
	uint32_t SS = 0;

	if (sscanf(date_time, "%d/%d/%d %d:%d:%d", &YY, &mm, &DD, &HH, &MM, &SS) != 6)
	{
		return (0);
	}
	*date = YY * 10000 + mm * 100 + DD;
	// *time = HH * 10000 + mm * 100 + SS;
	*time = HH * 60 * 60 + MM * 60 + SS;
	return (1);
}

// TODO : rm file2, rename file1 to file
/**
  * @brief Check if filename syntax is "YYYY_MM_DD__HH_mm_SS.json"
  * @param [file] file to check
  * @return 6 if filename seems correct else 0
*/
int8_t cmp_filename(struct dirent *file1, struct dirent *file2)
{
	char *name1tmp = file1->d_name;

	printf("Name = %s\n", name1tmp);
	uint32_t DD = 0;

	uint32_t mm = 0;

	uint32_t YY = 0;

	uint32_t HH = 0;

	uint32_t MM = 0;

	uint32_t SS = 0;

	uint32_t ret = 0;

	ret = sscanf(name1tmp, "%d_%d_%d__%d_%d_%d.json", &YY, &mm, &DD, &HH, &MM, &SS);

	printf("ret : %d\n", ret);
	printf("Time : %d/%d/%d %d:%d:%d\n", DD, mm, YY, HH, MM, SS);
	return (ret == 6);
}

/**
  * @brief Debug function, print current sensors datas
  * @param [sensors_data] Struct that contain current sensors datas
*/
void print_sensors_data(t_sensors *sensors_data)
{
	t_sensors *sensors_tmp;

	sensors_tmp = sensors_data;

	printf("Struct print :\n");
	while (sensors_tmp)
	{
		printf("Time %d %d\n", sensors_tmp->date, sensors_tmp->time);
		printf("photodiode : %f\n", sensors_tmp->photodiode);
		printf("temperature : %f\n", sensors_tmp->temperature);
		printf("comsumption : %f\n", sensors_tmp->comsumption);
		printf("position : %f\n", sensors_tmp->position);
		printf("orgue : %f\n", sensors_tmp->orgue);
		sensors_tmp = sensors_tmp->next;
	}
}

/**
  * @brief Clear all sensors_data list
  * @param [sensors_data] Struct that contain current sensors datas
*/
void clear_sensors_data(t_sensors *sensors_data)
{
	t_sensors *sensors_tmp;

	while (sensors_data)
	{
		sensors_tmp = sensors_data->next;
		free(sensors_data);
		sensors_data = sensors_tmp;
	}
}

/**
  * @brief Deserialize json file to t_sensors chained list
  * @param [file_length] Length of json to parse (char number)
  * @param [file_content] String that contain json file
  * @return Chained list that contain all sensors datas
*/
t_sensors *json_deserialize(uint32_t file_length, char *file_content)
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
		return (NULL);
	}

	/* Assume the top-level element is an object */
	if (r < 1 || t[0].type != JSMN_OBJECT)
	{
		printf("Object expected\n");
		return (NULL);
	}
	uint32_t nu_of_measures = 0;// Usefull?
	uint32_t obj_size;

	t_sensors *sensors_data;
	t_sensors *current_sensors;

	current_sensors = (t_sensors *)malloc(sizeof(t_sensors));
	sensors_data = current_sensors;

	// printf("main addr : %p\n", sensors_data);
	for (i = 1; i < r; i++)
	{
		if (t[i].type == JSMN_OBJECT && t[i].size == 6)
		{
			// printf("\n");
			// printf("addr :%p\n", current_sensors);
			obj_size = t[i].size;
			i++;
			if (JSON_cmp(file_content, &t[i], "Time") == 0)
			{
				// printf("- Time: %.*s\n", t[i + 1].end - t[i + 1].start,
					//    file_content + t[i + 1].start);
				date_time_to_date_and_time(file_content + t[i + 1].start,
										   &current_sensors->date, &current_sensors->time);
				i += 2;
			}
			if (JSON_cmp(file_content, &t[i], "Photodiode") == 0)
			{
				// printf("- Photodiode: %.*s\n", t[i + 1].end - t[i + 1].start,
					//    file_content + t[i + 1].start);
				current_sensors->photodiode = atof(file_content + t[i + 1].start);
				i += 2;
			}
			if (JSON_cmp(file_content, &t[i], "Temperature") == 0)
			{
				// printf("- Temperature: %.*s\n", t[i + 1].end - t[i + 1].start,
					//    file_content + t[i + 1].start);
				current_sensors->temperature = atof(file_content + t[i + 1].start);
				i += 2;
			}
			if (JSON_cmp(file_content, &t[i], "Consumption") == 0)
			{
				// printf("- Consumption: %.*s\n", t[i + 1].end - t[i + 1].start,
					//    file_content + t[i + 1].start);
				current_sensors->comsumption = atof(file_content + t[i + 1].start);
				i += 2;
			}
			if (JSON_cmp(file_content, &t[i], "Position") == 0)
			{
				// printf("- Position: %.*s\n", t[i + 1].end - t[i + 1].start,
					//    file_content + t[i + 1].start);
				current_sensors->position = atof(file_content + t[i + 1].start);
				i += 2;
			}
			if (JSON_cmp(file_content, &t[i], "Orgue") == 0)
			{
				// printf("- Orgue: %.*s\n", t[i + 1].end - t[i + 1].start,
					//    file_content + t[i + 1].start);
				current_sensors->orgue = atof(file_content + t[i + 1].start);
				i += 1;
			}
			if (i + 12 < r)
			{
				(current_sensors->next) = (t_sensors *)malloc(sizeof(t_sensors));
				current_sensors = current_sensors->next;
			}
			else
			{
				current_sensors->next = NULL;
			}
			nu_of_measures++;
		}
	}
	printf("main addr : %p\n", sensors_data);
	printf("Number of measures : %d\n", nu_of_measures);
	// print_sensors_data(sensors_data);
	return (sensors_data);
	// clear_sensors_data(sensors_data);
}


/**
  * @brief Find the first file well formated alphabetically 
  * @param [directory] Directory to scan
  * @param [file_path] Futur path of file
  * @return 1 if succeed else 0
*/
int get_first_data_file_in_directory(char *directory, char *file_path)
{
	struct dirent **namelist;
	int numberOfFiles;

	numberOfFiles = scandir(directory, &namelist, 0, alphasort);
	if (numberOfFiles < 0)
	{
		printf("Error scandir\n");
	}
	else
	{
		for (int currentIndex = 0; currentIndex < numberOfFiles; currentIndex++)
		{
			printf("Le fichier lu s'appelle '%s'\n", namelist[currentIndex]->d_name);
			if (cmp_filename(namelist[currentIndex], NULL))
			{
				printf("--This is a data file\n");
				file_path = strcat(strcat(strcpy(file_path, directory), "/"), namelist[currentIndex]->d_name);
				printf("--This is the Path : %s\n", file_path);
				for (int rmIndex = 0; rmIndex < numberOfFiles; rmIndex++)
				{
					free(namelist[rmIndex]);
				}
				free(namelist);
				return (1);
				break;
			}
			else
			{
				printf("--This is not a data file\n");
			}
		}
	}
	for (int rmIndex = 0; rmIndex < numberOfFiles; rmIndex++)
	{
		free(namelist[rmIndex]);
	}
	free(namelist);
	return (0);
}

/**
  * @brief Open file, save it to string (char*) and close it
  * @param [file_length] Futur length of string
  * @param [filename] File to open
  * @return string (char*)
*/
char *load_file(uint32_t *file_length, char *fileName)
{
	FILE *file_ptr;
	if (!(file_ptr = fopen(fileName, "r")))
	{
		printf("Error while opening file\n");
		return (NULL);
	}
	char *file_content;
	// uint32_t file_length;
	fseek(file_ptr, 0, SEEK_END);
	*file_length = ftell(file_ptr);
	fseek(file_ptr, 0, SEEK_SET);
	file_content = malloc(*file_length);
	fread(file_content, 1, *file_length, file_ptr);
	fclose(file_ptr);
	return (file_content);
}


// TODO : change filename to current date in music data, not current time
/**
  * @brief Create midi file, name it with the date of current music data and open it 
  * @param [music_data] Midi struct of midi file
  * @param [output_directory] location of this midi file
*/
void	create_dated_midi_file(t_music_data *music_data, char *output_directory)
{
	time_t now;
	struct tm tm_now;
	char fileName[sizeof("AAAA_mm_JJ__HH_MM_SS.mid")];
	char filePath[sizeof(fileName) + strlen(output_directory) + 1];
	now = time(NULL);
	tm_now = *localtime(&now);
	strftime(fileName, sizeof(fileName), "%Y_%m_%d__%H_%M_%S.mid", &tm_now);
	printf("File Name : %s\n", fileName);
	// midi_setup_file("Test2.midi", music_data);
	sprintf(filePath, "%s/%s", output_directory, fileName);;
	printf("File Path : %s\n", filePath);
	midi_setup_file(filePath, music_data);
}

void print_time(char *beg,uint32_t time, char *end)
{
	printf("%s%02dh%02dm%02ds%s", beg, time/60/60, time/60%60, time%60, end);
}


int main(int argc, char **argv)
{

	char *filesDirectory = "data_files";
	char *outputDirectory = "midi_files";

	printf("In MIDI prgm\n");	
	if (argc == 3)
	{
		filesDirectory = argv[1];
		outputDirectory = argv[2];
	}
	

	signal(SIGTERM, (void (*)(int))terminate_session);
	create_dated_midi_file(&g_music_data, outputDirectory);

	char *currentDataFileName;
	t_sensors *sensorsData;
	sensorsData = NULL;
	currentDataFileName = (char *)malloc(sizeof(char) * 200);

	//Main loop
	for (;;)
	{
		if (!sensorsData || !sensorsData->next)
		{
			if (get_first_data_file_in_directory(filesDirectory, currentDataFileName))
			{
				printf("__Fichier trouvé : %s\n", currentDataFileName);
				char *file_content;
				uint32_t file_length;

				if (!(file_content = load_file(&file_length, currentDataFileName)))
				{
					printf("Error loading file\n");
					exit(-1);
				}

				if (!sensorsData)
				{
					sensorsData = json_deserialize(file_length, file_content);
				}
				else
				{
					//verifier si ca c'est legit
					sensorsData->next = json_deserialize(file_length, file_content);
				}
				free(file_content);
				print_sensors_data(sensorsData);

				//Remove? reouvrir le precedent si SIGTERM? LOGS?
				if (remove(currentDataFileName))
				{
					printf("Error while deleting file\n");
				}
				else
				{
					printf("File succefully deleted\n");
				}
			}
			else
			{
				printf("Pas de fichiers trouvés\n");
				sleep(5);
			}
		}
		// Si on a pas de fichier midi ouvert on en ouvre un nouveau
		if (!g_music_data.midi_file)
		{
			create_dated_midi_file(&g_music_data, outputDirectory);
			g_music_data.measures_writed = 0;
			g_music_data.data_time = 0;
		}
		// Tant que les datas ne sont pas finies et qu'il reste 
		// des mesures á ecrire dans le fichier midi, on ecrit les
		// datas dans le fichier midi
		while (g_music_data.midi_file && sensorsData)
		{
			if (g_music_data.data_time == 0)
			{
				g_music_data.data_time = sensorsData->time;// * 1000000;
				g_music_data.entry_data_time = sensorsData->time;
			}
			midi_write_measure(&g_music_data, sensorsData);
			g_music_data.measures_writed++;
			g_music_data.data_time += g_music_data.measure_value / 1000000;

			//DEBUG
			if (!sensorsData->next)
			{
				printf("***Pas de data next\n");
			}
			else
			{
				// printf("***Next time : %d\n", sensorsData->next->time);
				print_time("***Next time : ",sensorsData->next->time, "\n");

				// printf("***Music time : %d\n", g_music_data.data_time);
				print_time("***Music time : ",g_music_data.data_time, "\n");
			}


			if (g_music_data.measure_value * g_music_data.measures_writed
				>= g_music_data.partition_duration)
			{
				printf("Midi write end\n");
				midi_write_end(&g_music_data);
			}
			
			if (!sensorsData->next)
			{
				break;
				// go aller rechercher des données (et pas passer au next)
			}

			while (sensorsData->next && g_music_data.data_time > sensorsData->next->time)
			{
				t_sensors *sensors_tmp;
				sensors_tmp = sensorsData->next;
				free(sensorsData);
				sensorsData = sensors_tmp;
			}
		}
		printf("Fin de boucle \n");
	}

	// Signaux ->
	// Fini le midi, (lui donne un nom avec date debut/fin)
	// Note dans les LOGS le temps d'arret midi et log

	printf("MIDI prgrm EXIT\n");
	if (g_music_data.midi_file)
	{
		midi_write_end(&g_music_data);
	}

	return (0);
}
