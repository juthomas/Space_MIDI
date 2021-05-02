#include "../inc/midi.h"
#include "../inc/json_parser.h"
#include "../inc/midi_notes.h"
#include "../inc/midi_modes.h"

// 							//durée d'une partition 40 000 000us
// static t_music_data music_data = {.partition_duration = 40000000,
// 								//Measure value = quarter value * 4 (4/4) (4 noires par mesure)
// 							   .measure_value = 500000 * 4,
// 							   .measures_writed = 0,
// 							   // valeur d'une noire en us (pour le tempo)
// 							   .quarter_value = 500000 };

static bool g_exit_requested = false;

/**
  * @brief Map number from [in_min]-[in_max] to [out_min]-[out_max]
  * @param [x] Number to map
  * @param [in_min] Minimum of input number
  * @param [in_max] Maximum of input number
  * @param [out_min] Minimum of output number
  * @param [out_max] Maximum of ouput number
  * @return New number well mapped
*/
int32_t map_number(int32_t x, int32_t in_min, int32_t in_max, int32_t out_min, int32_t out_max)
{
	return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}

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
	// music_data->current_quarter_value / (music_data->quarter_value / 128)
	// MIDI_delta_time(music_data->midi_file, QUARTER);
	MIDI_delta_time(music_data->midi_file, music_data->current_quarter_value / (music_data->quarter_value / 128));
	MIDI_Note(music_data->midi_file, OFF, 1, 10, 0);
}


/**
  * @brief Simply wait for a heighth of measure
  * @param [music_data] Midi struct
*/
void midi_delay_heighth(t_music_data *music_data)
{
	MIDI_delta_time(music_data->midi_file, 0);
	MIDI_Note(music_data->midi_file, OFF, 1, 10, 64);
	// music_data->current_quarter_value / (music_data->quarter_value / 128)
	// MIDI_delta_time(music_data->midi_file, QUARTER);
	MIDI_delta_time(music_data->midi_file, music_data->current_quarter_value / (music_data->quarter_value / 256));
	MIDI_Note(music_data->midi_file, OFF, 1, 10, 0);
}


void get_music_mode(uint8_t gamme[7], uint8_t music_mode)
{
	// uint8_t *mode_phrygien = g_midi_mode[M_MODE_DORIEN_DIEZ4].mode_sequence;
	uint8_t tone_gap = 0;
	
	for (int i = 0; i < 7; i++)
	{
		gamme[i] = g_midi_mode[music_mode].starting_note + tone_gap;
		tone_gap += g_midi_mode[music_mode].mode_sequence[i];
	}
}


/**
  * @brief Update quarter value to match with quarter value goal 
  * (may take several calls)
  * @param [music_data] Midi struct
*/
void update_quarter_value(t_music_data *music_data)
{
	if (music_data->current_quarter_value != music_data->quarter_value_goal)
	{
		if (music_data->current_quarter_value < music_data->quarter_value_goal)
		{
			if (music_data->quarter_value_goal - music_data->current_quarter_value < music_data->quarter_value_step)
			{
				music_data->current_quarter_value = music_data->quarter_value_goal;
			}
			else
			{
				music_data->current_quarter_value += music_data->quarter_value_step;
			}
		}
		else
		{
			if (music_data->current_quarter_value - music_data->quarter_value_goal< music_data->quarter_value_step)
			{
				music_data->current_quarter_value = music_data->quarter_value_goal;
			}
			else
			{
				music_data->current_quarter_value -= music_data->quarter_value_step;
			}
		}
	}
}


/**
  * @brief Function to write an 4 stroke measure
  * @param [music_data] Midi struct
  * @param [sensors_data] Struct that contain current sensors datas
*/
void midi_write_measure(t_music_data *music_data, t_sensors *sensors_data)
{
	uint8_t gamme[7];
	get_music_mode(gamme, M_MODE_PHRYGIEN);
	// 0 - 5 => 500000 -> 50000
	music_data->quarter_value_goal = (uint32_t)map_number((uint32_t)sensors_data->photodiode_1, 0, 5, 500000, 50000);
	update_quarter_value(music_data);
	// //  = g_midi_mode[M_MODE_DORIEN_DIEZ4].mode_sequence;
	// for (int i = 0; i < 7; i++)
	// {
	// 	// mode_phrygien[i] += g_midi_mode[M_MODE_DORIEN_DIEZ4].starting_note;
	// 	printf("Mode value : %d\n", gamme[i]);
	// }
	

//    ====================================================
//  ||                                                    ||
//  ||                      T = 1/4                       ||
//  ||                                                    ||
	midi_write_measure_note(music_data, ON, 1, gamme[0], 64);
//  ||                                                    ||
//  ||                      T = 1/4                       ||
//  ||                                                    ||
//    ====================================================

	midi_delay_quarter(music_data);
	
//    ====================================================
//  ||                                                    ||
//  ||                      T = 2/4                       ||
//  ||                                                    ||
	midi_write_measure_note(music_data, OFF, 1, gamme[0], 0);

	midi_write_measure_note(music_data, ON, 1,
							gamme[(uint32_t)sensors_data->photodiode_1 % 7], 64);
//  ||                                                    ||
//  ||                      T = 2/4                       ||
//  ||                                                    ||
//    ====================================================

	midi_delay_quarter(music_data);

//    ====================================================
//  ||                                                    ||
//  ||                      T = 3/4                       ||
//  ||                                                    ||
	midi_write_measure_note(music_data, OFF, 1,
							gamme[(uint32_t)sensors_data->photodiode_1 % 7], 0);
	midi_write_measure_note(music_data, ON, 1,
							gamme[(uint32_t)sensors_data->temperature_1 % 7], 64);
//  ||                                                    ||
//  ||                      T = 3/4                       ||
//  ||                                                    ||
//    ====================================================

	midi_delay_quarter(music_data);

//    ====================================================
//  ||                                                    ||
//  ||                      T = 4/4                       ||
//  ||                                                    ||
	midi_write_measure_note(music_data, OFF, 1,
							gamme[(uint32_t)sensors_data->temperature_1 % 7], 0);
	midi_write_measure_note(music_data, ON, 1,
							gamme[(uint32_t)sensors_data->vin_current % 7], 64);
//  ||                                                    ||
//  ||                      T = 4/4                       ||
//  ||                                                    ||
//    ====================================================

	midi_delay_quarter(music_data);

//    ====================================================
//  ||                                                    ||
//  ||                      T = END                       ||
//  ||                                                    ||
	midi_write_measure_note(music_data, OFF, 1,
							gamme[(uint32_t)sensors_data->vin_current % 7], 0);
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

// TODO : Create new data file with sensors data
/**
  * @brief Called when signal "SIGTERM" is sended
  * @param [signal] Signal Number (Probably 15)
*/
void terminate_session(int signal)
{
	(void)signal;
	g_exit_requested = true;
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

/**
  * @brief Check if filename syntax is "YYYY_MM_DD__HH_mm_SS.json"
  * @param [file] file to check
  * @return 6 if filename seems correct else 0
*/
int8_t cmp_filename(struct dirent *file)
{
	char *name1tmp = file->d_name;

	uint32_t DD = 0;

	uint32_t mm = 0;

	uint32_t YY = 0;

	uint32_t HH = 0;

	uint32_t MM = 0;

	uint32_t SS = 0;

	uint32_t ret = 0;

	ret = sscanf(name1tmp, "%d_%d_%d__%d_%d_%d.json", &YY, &mm, &DD, &HH, &MM, &SS);

	// printf("ret : %d\n", ret);
	// printf("Time : %d/%d/%d %d:%d:%d\n", DD, mm, YY, HH, MM, SS);
	return (ret == 6);
}

void print_time(char *beg,uint32_t time, char *end)
{
	printf("%s%02dh%02dm%02ds(%d)%s", beg, time/60/60, time/60%60, time%60, time, end);
}


/**
  * @brief Debug function, print current sensors datas
  * @param [sensors_data] Struct that contain current sensors datas
*/
void print_sensors_data(t_sensors *sensors_data)
{
	t_sensors *sensors_tmp;

	sensors_tmp = sensors_data;
	int current_data = 0;
	printf("Struct print :\n");
	while (sensors_tmp)
	{
		// printf("Time %d %d\n", sensors_tmp->date, sensors_tmp->time);
		printf("%scurrent data : %d\n","\033[1;35m", current_data);
		print_time("> Time : ", sensors_tmp->time, "\n\033[1;37m");
		current_data++;
		// printf("Photodiodes\n");
		// printf("          1 : %f\n", sensors_tmp->photodiode_1);
		// printf("          2 : %f\n", sensors_tmp->photodiode_2);
		// printf("          3 : %f\n", sensors_tmp->photodiode_3);
		// printf("          4 : %f\n", sensors_tmp->photodiode_4);
		// printf("          5 : %f\n", sensors_tmp->photodiode_5);
		// printf("          6 : %f\n", sensors_tmp->photodiode_6);
		// printf("Temperatures\n");
		// printf("vin current : %f\n", sensors_tmp->vin_current);
		// printf("          1 : %f\n", sensors_tmp->temperature_1);
		// printf("          2 : %f\n", sensors_tmp->temperature_2);
		// printf("          3 : %f\n", sensors_tmp->temperature_3);
		// printf("          4 : %f\n", sensors_tmp->temperature_4);
		// printf("          5 : %f\n", sensors_tmp->temperature_5);
		// printf("          6 : %f\n", sensors_tmp->temperature_6);
		// printf("          7 : %f\n", sensors_tmp->temperature_7);
		// printf("          8 : %f\n", sensors_tmp->temperature_8);
		// printf("          9 : %f\n", sensors_tmp->temperature_9);
		// printf("          10 : %f\n", sensors_tmp->temperature_10);
		// printf("lid state : %d\n", sensors_tmp->lid_state);
		// printf("first sample : %s\n", sensors_tmp->first_sample ? "true" : "false");
		// printf("spectro current : %f\n", sensors_tmp->spectro_current);
		// printf("electro current : %f\n", sensors_tmp->electro_current);
		// printf("organ current : %f\n", sensors_tmp->organ_current);
		// printf("q7 current : %f\n", sensors_tmp->q7_current);
		// printf("5v current : %f\n", sensors_tmp->t5v_current);
		// printf("3.3v current : %f\n", sensors_tmp->t3_3v_current);
		// printf("motor current : %f\n", sensors_tmp->motor_current);
		// printf("position 360 : %d\n", sensors_tmp->position_360);
		// printf("spectrum : %d\n", sensors_tmp->spectrum);
		// printf("organ : %f\n", sensors_tmp->organ);
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

	t_sensors *sensors_data;
	t_sensors *current_sensors;

	current_sensors = (t_sensors *)malloc(sizeof(t_sensors));
	sensors_data = current_sensors;

	// printf("main addr : %p\n", sensors_data);
	for (i = 3; i < r; i++)
	{
		if (t[i].type == JSMN_OBJECT && t[i].size == (29 + 1))
		{
			// printf("\n");
			// printf("addr :%p\n", current_sensors);
			//obj_size = t[i].size;
			i++;
			if (JSON_cmp(file_content, &t[i], "Time") == 0)
			{
				// printf("- Time: %.*s\n", t[i + 1].end - t[i + 1].start,
					//    file_content + t[i + 1].start);
				date_time_to_date_and_time(file_content + t[i + 1].start,
										   &current_sensors->date, &current_sensors->time);
				i += 2;
			}
			if (JSON_cmp(file_content, &t[i], "Photodiode_1") == 0)
			{
				current_sensors->photodiode_1 = atof(file_content + t[i + 1].start);
				i += 2;
			}
			if (JSON_cmp(file_content, &t[i], "Photodiode_2") == 0)
			{
				current_sensors->photodiode_2 = atof(file_content + t[i + 1].start);
				i += 2;
			}
			if (JSON_cmp(file_content, &t[i], "Photodiode_3") == 0)
			{
				current_sensors->photodiode_3 = atof(file_content + t[i + 1].start);
				i += 2;
			}
			if (JSON_cmp(file_content, &t[i], "Photodiode_4") == 0)
			{
				current_sensors->photodiode_4 = atof(file_content + t[i + 1].start);
				i += 2;
			}
			if (JSON_cmp(file_content, &t[i], "Photodiode_5") == 0)
			{
				current_sensors->photodiode_5 = atof(file_content + t[i + 1].start);
				i += 2;
			}
			if (JSON_cmp(file_content, &t[i], "Photodiode_6") == 0)
			{
				current_sensors->photodiode_6 = atof(file_content + t[i + 1].start);
				i += 2;
			}

			if (JSON_cmp(file_content, &t[i], "Temperature_1") == 0)
			{
				current_sensors->temperature_1 = atof(file_content + t[i + 1].start);
				i += 2;
			}
			if (JSON_cmp(file_content, &t[i], "Temperature_2") == 0)
			{
				current_sensors->temperature_2 = atof(file_content + t[i + 1].start);
				i += 2;
			}
			if (JSON_cmp(file_content, &t[i], "Temperature_3") == 0)
			{
				current_sensors->temperature_3 = atof(file_content + t[i + 1].start);
				i += 2;
			}
			if (JSON_cmp(file_content, &t[i], "Temperature_4") == 0)
			{
				current_sensors->temperature_4 = atof(file_content + t[i + 1].start);
				i += 2;
			}
			if (JSON_cmp(file_content, &t[i], "Temperature_5") == 0)
			{
				current_sensors->temperature_5 = atof(file_content + t[i + 1].start);
				i += 2;
			}
			if (JSON_cmp(file_content, &t[i], "Temperature_6") == 0)
			{
				current_sensors->temperature_6 = atof(file_content + t[i + 1].start);
				i += 2;
			}
			if (JSON_cmp(file_content, &t[i], "Temperature_7") == 0)
			{
				current_sensors->temperature_7 = atof(file_content + t[i + 1].start);
				i += 2;
			}
			if (JSON_cmp(file_content, &t[i], "Temperature_8") == 0)
			{
				current_sensors->temperature_8 = atof(file_content + t[i + 1].start);
				i += 2;
			}
			if (JSON_cmp(file_content, &t[i], "Temperature_9") == 0)
			{
				current_sensors->temperature_9 = atof(file_content + t[i + 1].start);
				i += 2;
			}
			if (JSON_cmp(file_content, &t[i], "Temperature_10") == 0)
			{
				current_sensors->temperature_10 = atof(file_content + t[i + 1].start);
				i += 2;
			}

			if (JSON_cmp(file_content, &t[i], "Lid_state") == 0)
			{
				current_sensors->lid_state = atoi(file_content + t[i + 1].start);
				i += 2;
			}
			if (JSON_cmp(file_content, &t[i], "First_sample") == 0)
			{
				current_sensors->first_sample = atoi(file_content + t[i + 1].start);
				i += 2;
			}

			if (JSON_cmp(file_content, &t[i], "Spectro_current") == 0)
			{
				current_sensors->spectro_current = atof(file_content + t[i + 1].start);
				i += 2;
			}
			if (JSON_cmp(file_content, &t[i], "Electro_current") == 0)
			{
				current_sensors->electro_current = atof(file_content + t[i + 1].start);
				i += 2;
			}
			if (JSON_cmp(file_content, &t[i], "Organ_current") == 0)
			{
				current_sensors->organ_current = atof(file_content + t[i + 1].start);
				i += 2;
			}
			if (JSON_cmp(file_content, &t[i], "Vin_current") == 0)
			{
				current_sensors->vin_current = atof(file_content + t[i + 1].start);
				i += 2;
			}
			if (JSON_cmp(file_content, &t[i], "Q7_current") == 0)
			{
				current_sensors->q7_current = atof(file_content + t[i + 1].start);
				i += 2;
			}
			if (JSON_cmp(file_content, &t[i], "5v_current") == 0)
			{
				current_sensors->t5v_current = atof(file_content + t[i + 1].start);
				i += 2;
			}
			if (JSON_cmp(file_content, &t[i], "3.3v_current") == 0)
			{
				current_sensors->t3_3v_current = atof(file_content + t[i + 1].start);
				i += 2;
			}
			if (JSON_cmp(file_content, &t[i], "Motor_current") == 0)
			{
				current_sensors->motor_current = atof(file_content + t[i + 1].start);
				i += 2;
			}

			if (JSON_cmp(file_content, &t[i], "Position_360") == 0)
			{
				current_sensors->position_360 = atoi(file_content + t[i + 1].start);
				i += 2;
			}
			if (JSON_cmp(file_content, &t[i], "Spectrum") == 0)
			{
				current_sensors->spectrum = atoi(file_content + t[i + 1].start);
				i += 2;
			}

			if (JSON_cmp(file_content, &t[i], "Organ") == 0)
			{
				current_sensors->organ = atof(file_content + t[i + 1].start);
				i += 1;
			}
			if (i + (29 * 2 + 2) < r)
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
	// printf("main addr : %p\n", sensors_data);
	// printf("Number of measures : %d\n", nu_of_measures);
	return (sensors_data);
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
			// printf("Le fichier lu s'appelle '%s'\n", namelist[currentIndex]->d_name);
			if (cmp_filename(namelist[currentIndex]))
			{
				// printf("--This is a data file\n");
				file_path = strcat(strcat(strcpy(file_path, directory), "/"), namelist[currentIndex]->d_name);
				// printf("--This is the Path : %s\n", file_path);
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
				// printf("--This is not a data file\n");
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
	//Delay to wait write file end
	usleep(50000);
	FILE *file_ptr;
	if (!(file_ptr = fopen(fileName, "r")))
	{
		printf("Error while opening file\n");
		return (NULL);
	}
	char *file_content;
	fseek(file_ptr, 0, SEEK_END);
	*file_length = ftell(file_ptr);
	fseek(file_ptr, 0, SEEK_SET);
	file_content = malloc(*file_length);
	fread(file_content, 1, *file_length, file_ptr);
	fclose(file_ptr);
	return (file_content);
}

/**
  * @brief Create file path if doesnt exists
  * @param [file_path] File path, /!\ cant be const /!\
  * @param [mode] Permission bit masks for mode
*/
int make_path(char* file_path, mode_t mode) {
	for (char* p = strchr(file_path + 1, '/'); p; p = strchr(p + 1, '/')) {
		*p = '\0';
		if (mkdir(file_path, mode) == -1) {
			if (errno != EEXIST) {
				*p = '/';
				return -1;
			}
		}
		*p = '/';
	}
	return 0;
}

// TODO : Supress else
/**
  * @brief Create midi file, name it with the date of current music data and open it 
  * @param [music_data] Midi struct of midi file
  * @param [output_directory] location of this midi file
*/
void	create_dated_midi_file(t_music_data *music_data, char *output_directory, t_sensors *sensors_data)
{
	time_t now;
	struct tm tm_now;
	char fileName[sizeof("AAAA_mm_JJ__HH_MM_SS.mid")];
	char filePath[sizeof(fileName) + strlen(output_directory) + 1];
	
	if (sensors_data)
	{
		sprintf(fileName, "%04d_%02d_%02d__%02d_%02d_%02d.mid", \
			sensors_data->date / 10000, sensors_data->date / 100 % 100, sensors_data->date % 100,\
			sensors_data->time / 60 / 60, sensors_data->time / 60 % 60, sensors_data->time % 60);
	}
	else //Ne passe normalement jamais ICI
	{
		now = time(NULL);
		tm_now = *localtime(&now);
		strftime(fileName, sizeof(fileName), "%Y_%m_%d__%H_%M_%S.mid", &tm_now);
	}
	
	// printf("File Name : %s\n", fileName);
	sprintf(filePath, "%s/%s", output_directory, fileName);
	// printf("File Path : %s\n", filePath);
	make_path(filePath, 0755);
	midi_setup_file(filePath, music_data);
}



int main(int argc, char **argv)
{
								//durée d'une partition 40 000 000us
	 t_music_data music_data = {.partition_duration = 40000000,
								//Measure value = quarter value * 4 (4/4) (4 noires par mesure)
							   .measure_value = 500000 * 4,
							   .measures_writed = 0,//
							   .delta_time = 0,
							   .quarter_value_step = 10000,// 
							   .quarter_value_goal = 500000,//equal to value
							   // valeur d'une noire en us (pour le tempo)
							   .quarter_value = 500000,
							   .current_quarter_value = 500000};
	char *filesDirectory = "data_files";
	char *outputDirectory = "midi_files";

	if (argc == 3)
	{
		filesDirectory = argv[1];
		outputDirectory = argv[2];
	}
	

	signal(SIGTERM, (void (*)(int))terminate_session);

	char *currentDataFileName;
	t_sensors *sensorsData;
	sensorsData = NULL;
	currentDataFileName = (char*)malloc(sizeof(char) * 200);

	//Main loop
	while (!g_exit_requested)
	{
		if (!sensorsData || !sensorsData->next)
		{
			if (get_first_data_file_in_directory(filesDirectory, currentDataFileName))
			{
				// printf("__Fichier trouvé : %s\n", currentDataFileName);
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
				else// Normalement ne sert plus a R
				{
					//verifier si ca c'est legit
					sensorsData->next = json_deserialize(file_length, file_content);
				}
				free(file_content);
				// print_sensors_data(sensorsData);

				//Remove? reouvrir le precedent si SIGTERM? LOGS?
				if (remove(currentDataFileName))
				{
					printf("Error while deleting file\n");
				}
				else
				{
					// printf("File succefully deleted\n");
				}
			}
			else
			{
				// printf("Pas de fichiers trouvés\n");
				sleep(5);
			}
		}
		// Si on a pas de fichier midi ouvert on en ouvre un nouveau
		if (!music_data.midi_file && sensorsData)
		{
			create_dated_midi_file(&music_data, outputDirectory, sensorsData);
			music_data.measures_writed = 0;//
			music_data.data_time = 0;//
			music_data.delta_time = 0;
		}
		// Tant que les datas ne sont pas finies et qu'il reste 
		// des mesures á ecrire dans le fichier midi, on ecrit les
		// datas dans le fichier midi
		while (music_data.midi_file && sensorsData)
		{
			if (music_data.data_time == 0)
			{
				music_data.data_time = sensorsData->time;// * 1000000;
				music_data.entry_data_time = sensorsData->time;
				// print_time("-_- new time : ",sensorsData->time, "\n");
				
				
				// t_sensors *sensors_tmp;
				// sensors_tmp = sensorsData->next;
				// free(sensorsData);
				// sensorsData = sensors_tmp;
			}
			if (music_data.data_time <= sensorsData->time)
			{
				// if (music_data.current_quarter_value >= 50000)
				// {
				// 	music_data.current_quarter_value -= 5000;
				// }
				midi_write_measure(&music_data, sensorsData);
				music_data.measures_writed++;//
				music_data.delta_time += (music_data.current_quarter_value * 4);
				music_data.data_time = music_data.entry_data_time + ((music_data.delta_time) / 1000000);
			}
			//DEBUG
			// if (!sensorsData->next)
			// {
			// 	printf("***Pas de data next\n");
			// }
			// else
			// {
			// 	// printf("***Next time : %d\n", sensorsData->next->time);
			// 	print_time("***Next time : ",sensorsData->next->time, "\n");

			// 	// printf("***Music time : %d\n", music_data.data_time);
			// 	print_time("***Music time : ",music_data.data_time, "\n");
			// }
			// print_sensors_data(sensorsData);

			if (music_data.delta_time
				>= music_data.partition_duration)
			{
				printf("Midi write end\n");
				midi_write_end(&music_data);
			}
			
			if (!sensorsData->next)
			{
				break;
				// Aller rechercher des données (et pas passer au next)
			}

			// printf("datatime : %d, sensor time : %d\n",  music_data.data_time,  sensorsData->time);
			// while (sensorsData->next && music_data.data_time > sensorsData->next->time)
			while (sensorsData && music_data.data_time >= sensorsData->time)// >= ???
			{
				if (sensorsData->next)
				{
					t_sensors *sensors_tmp;
					sensors_tmp = sensorsData->next;
					free(sensorsData);
					sensorsData = sensors_tmp;
				}
				else
				{
					sensorsData = NULL;
				}
			}
			// printf("left : datatime : %d, sensor time : %d\n",  music_data.data_time,  sensorsData->time);
		}
	}

	// Signaux ->
	// Fini le midi, (lui donne un nom avec date debut/fin)
	// Note dans les LOGS le temps d'arret midi et log

	// printf("MIDI prgrm EXIT\n");
	if (music_data.midi_file)
	{
		midi_write_end(&music_data);
	}
	return (0);
}
