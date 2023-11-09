#include "../inc/midi.h"
// #include "../inc/json_parser.h"
#include "../inc/midi_notes.h"
#include "../inc/midi_modes.h"
#include "../inc/midi_euclidean.h"

// Structure pour les opérations P() et V() sur les sémaphores
struct sembuf P = {0, -1, SEM_UNDO}; // Opération P (wait)
struct sembuf V = {0, 1, SEM_UNDO};	 // Opération V (signal)

#define LOG_ALL 0
#define EUCLIDEAN_DATAS_LENGTH 4

// 							//durée d'une partition 40 000 000us
// static t_music_data music_data = {.partition_duration = 40000000,
// 								//Measure value = quarter value * 4 (4/4) (4 noires par mesure)
// 							   .measure_value = 500000 * 4,
// 							   .measures_writed = 0,
// 							   // valeur d'une noire en us (pour le tempo)
// 							   .quarter_value = 500000 };

static uint8_t playing_notes_length = 24;
static uint8_t playing_notes[24];
static uint8_t playing_notes_duration[24];

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
	if (x < in_min)
		return out_min;
	if (x > in_max)
		return out_max;
	return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}

/**
 * @brief Create and open a new midi file
 * @param [filename] futur name of midi file
 * @param [filename_redundancy] futur name of midi file redundancy
 * @param [music_data] Midi struct
 */
void midi_setup_file(char *filename, char *filename_redundancy,
					 t_music_data *music_data)
{
	music_data->midi_file = fopen(filename, "wb");
	music_data->midi_file_redundancy = fopen(filename_redundancy, "wb");
	MIDI_write_file_header(music_data->midi_file, 1, 2, QUARTER);
	MIDI_write_file_header(music_data->midi_file_redundancy, 1, 2, QUARTER);
	// metadatas
	MIDI_write_metadata(music_data->midi_file, music_data->quarter_value);
	MIDI_write_metadata(music_data->midi_file_redundancy, music_data->quarter_value);
	music_data->midi_mark = MIDI_write_track_header(music_data->midi_file);
	music_data->midi_mark_redundancy = MIDI_write_track_header(music_data->midi_file_redundancy);
	MIDI_Instrument_Change(music_data->midi_file, 0, 90);
	MIDI_Instrument_Change(music_data->midi_file_redundancy, 0, 90);
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
	// printf("\033[1;35mwrite measure note : state=%s channel=%d note=%d velocity=%d\033[1;37m\n\n",
	// 	   (state == ON ? "ON" : "OFF"), channel, note, velocity);
	MIDI_delta_time(music_data->midi_file, 0);
	MIDI_delta_time(music_data->midi_file_redundancy, 0);
	MIDI_Note(music_data->midi_file, state, channel, note, velocity);
	MIDI_Note(music_data->midi_file_redundancy, state, channel, note, velocity);
}

/**
 * @brief Simply wait for a quarter of measure
 * @param [music_data] Midi struct
 */
void midi_delay_quarter(t_music_data *music_data)
{
	MIDI_delta_time(music_data->midi_file, 0);
	MIDI_delta_time(music_data->midi_file_redundancy, 0);
	MIDI_Note(music_data->midi_file, OFF, 1, 10, 64);
	MIDI_Note(music_data->midi_file_redundancy, OFF, 1, 10, 64);
	MIDI_delta_time(music_data->midi_file, music_data->current_quarter_value / (music_data->quarter_value / 128));
	MIDI_delta_time(music_data->midi_file_redundancy, music_data->current_quarter_value / (music_data->quarter_value / 128));
	MIDI_Note(music_data->midi_file, OFF, 1, 10, 0);
	MIDI_Note(music_data->midi_file_redundancy, OFF, 1, 10, 0);
}

/**
 * @brief Simply wait for a heighth of measure
 * @param [music_data] Midi struct
 */
void midi_delay_heighth(t_music_data *music_data)
{
	MIDI_delta_time(music_data->midi_file, 0);
	MIDI_delta_time(music_data->midi_file_redundancy, 0);
	MIDI_Note(music_data->midi_file, OFF, 1, 10, 64);
	MIDI_Note(music_data->midi_file_redundancy, OFF, 1, 10, 64);
	MIDI_delta_time(music_data->midi_file, music_data->current_quarter_value / (music_data->quarter_value / 64));
	MIDI_delta_time(music_data->midi_file_redundancy, music_data->current_quarter_value / (music_data->quarter_value / 64));
	MIDI_Note(music_data->midi_file, OFF, 1, 10, 0);
	MIDI_Note(music_data->midi_file_redundancy, OFF, 1, 10, 0);
}

/**
 * @brief Simply wait for a fraction of measure
 * @param [music_data] Midi struct
 * @param [fraction] Fraction delay
 */
void midi_delay_divs(t_music_data *music_data, uint16_t divs)
{
	MIDI_delta_time(music_data->midi_file, 0);
	MIDI_delta_time(music_data->midi_file_redundancy, 0);
	MIDI_Note(music_data->midi_file, OFF, 1, 10, 64);
	MIDI_Note(music_data->midi_file_redundancy, OFF, 1, 10, 64);
	MIDI_delta_time(music_data->midi_file, music_data->current_quarter_value / (music_data->quarter_value / divs));
	MIDI_delta_time(music_data->midi_file_redundancy, music_data->current_quarter_value / (music_data->quarter_value / divs));
	MIDI_Note(music_data->midi_file, OFF, 1, 10, 0);
	MIDI_Note(music_data->midi_file_redundancy, OFF, 1, 10, 0);
}

void get_music_mode(uint8_t gamme[7], uint8_t music_mode)
{
	for (int i = 0; i < 7; i++)
	{
		gamme[i] = g_midi_mode[music_mode].starting_note + g_midi_mode[music_mode].mode_sequence[i];
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
			if (music_data->quarter_value_goal < music_data->current_quarter_value + (uint32_t)((float)music_data->current_quarter_value * music_data->quarter_value_step_updating))
			{
				music_data->current_quarter_value = music_data->quarter_value_goal;
			}
			else
			{
				music_data->current_quarter_value += (uint32_t)((float)music_data->current_quarter_value * music_data->quarter_value_step_updating);
			}
		}
		else
		{
			if (music_data->quarter_value_goal > music_data->current_quarter_value - (uint32_t)((float)music_data->current_quarter_value * music_data->quarter_value_step_updating))
			{
				music_data->current_quarter_value = music_data->quarter_value_goal;
			}
			else
			{
				music_data->current_quarter_value -= (uint32_t)((float)music_data->current_quarter_value * music_data->quarter_value_step_updating);
			}
		}
	}
}

/**
 * @brief Try to play/stop selected note at correct timing
 * @param [music_data] Midi struct
 * @param [note] note to play/stop
 */
void midi_write_measure_heighth_updater(t_music_data *music_data, t_note note, uint8_t current_heighth)
{
	if (note.active && note.beg_eighth == current_heighth)
	{
		midi_write_measure_note(music_data, ON, note.channel, note.note, note.velocity);
	}
	if (note.active && note.beg_eighth + note.eighth_duration == current_heighth)
	{
		midi_write_measure_note(music_data, OFF, note.channel, note.note, 0);
	}
}

/**
 * @brief Function to get a chords list indexes
 * @param [chords_list] chords list returned
 * @param [chords_size] Number of chords desired
 */
void get_chords_list(uint8_t *chords_list, uint8_t chords_size)
{
	for (uint8_t i = 0; i < chords_size; i++)
	{
		chords_list[i] = /* starting_note + */ (i * 2) % 7;
	}
}

/**
 * @brief Function to create chord for euclidean composing
 * @param [music_data] Midi struct
 * @param [playing_notes_duration] Tab of current playing notes durations
 * @param [playing_notes] Tab of current playing notes
 * @param [playing_notes_length] Size of 'playing_note_duration' & 'playing_notes'
 * @param [chords_list] Tab of selected mode notes
 * @param [note_i] Index of note in selected mode notes
 * @param [note_offset] Offset of note (starting note of mode)
 * @param [chord_size] Size of chord (simultaneous played notes)
 * @param [velocity] Velocity of chord/note
 * @param [steps_duration] Duration of note converted in euclidean steps
 */
void create_chord(t_music_data *music_data, uint8_t *playing_notes_duration, uint8_t *playing_notes,
				  uint8_t playing_notes_length, uint8_t mode, int16_t note_i,
				  uint8_t note_offset, uint8_t chord_size, uint8_t velocity, uint8_t steps_duration)
{
	bool current_note_done = false;

	for (uint8_t current_note = 0; current_note < chord_size; current_note++)
	{
		current_note_done = false;
		// If the note is currently played
		// Just add time to that note
		for (uint8_t playing_notes_i = 0; playing_notes_i < playing_notes_length; playing_notes_i++)
		{
			if (playing_notes[playing_notes_i] == note_offset + ((note_i & 0xFF00) >> 8) * 12 + g_midi_mode[mode].mode_sequence[((note_i & 0xFF) + 2 * current_note) % 7] + 12 * (((note_i & 0xFF) + 2 * current_note) / 7))
			{

				playing_notes_duration[playing_notes_i] = steps_duration + 1;
				current_note_done = true;
			}
		}
		// If the note isnt played
		// Create that note !
		for (uint8_t playing_notes_i = 0; playing_notes_i < playing_notes_length && !current_note_done; playing_notes_i++)
		{
			if (!playing_notes_duration[playing_notes_i])
			{

				playing_notes_duration[playing_notes_i] = steps_duration + 1;
				playing_notes[playing_notes_i] = note_offset + ((note_i & 0xFF00) >> 8) * 12 + g_midi_mode[mode].mode_sequence[((note_i & 0xFF) + 2 * current_note) % 7] + 12 * (((note_i & 0xFF) + 2 * current_note) / 7);
				// beg note
				midi_write_measure_note(music_data, ON, 1, playing_notes[playing_notes_i], velocity);
				break;
			}
		}
	}
}

/**
 * @brief Function to remove chord for euclidean composing
 * @param [music_data] Midi struct
 * @param [playing_notes_duration] Tab of current playing notes durations
 * @param [playing_notes] Tab of current playing notes
 * @param [playing_notes_length] Size of 'playing_note_duration' & 'playing_notes'
 */
void remove_chord(t_music_data *music_data, uint8_t *playing_notes_duration,
				  uint8_t *playing_notes, uint8_t playing_notes_length)
{
	for (uint8_t playing_notes_i = 0; playing_notes_i < playing_notes_length; playing_notes_i++)
	{
		if (playing_notes_duration[playing_notes_i])
		{
			if (playing_notes_duration[playing_notes_i] == 1)
			{
				// end note
				midi_write_measure_note(music_data, OFF, 1, playing_notes[playing_notes_i], 0);
				playing_notes[playing_notes_i] = 0;
			}
			playing_notes_duration[playing_notes_i]--;
		}
	}
}

/**
 * @brief Function to get a new note in allowed ones
 * @param [chords_list] List of allowed chords
 * @param [chord_list_length] Size of allowed chords list
 * @param [current_step] Current step in euclidean circle
 * @param [euclidean_steps] Euclidean steps (contain notes)
 * @return New Chord
 */
int16_t get_new_chord_from_list(uint8_t *chords_list, uint8_t chord_list_length, uint8_t current_step, int16_t *euclidean_steps)
{
	int16_t chord_to_test = 0;
	uint8_t steps = 0;

	int16_t available_chords_list[chord_list_length];
	uint8_t available_chords_list_len = 0;

	// Check for each chords indexes in mode
	for (uint8_t i = 0; i < chord_list_length; i++)
	{
		chord_to_test = chords_list[i];
		steps = 0;

		// While chords indexes doesnt exist in euclidean steps and dont check for steps not yet attributed
		while (euclidean_steps[steps] != chord_to_test && steps < current_step)
		{
			steps++;
		}
		// If chords indexes doesnt exist in euclidean steps, feed in a chord list
		if (euclidean_steps[steps] != chord_to_test)
		{
			available_chords_list[available_chords_list_len] = chord_to_test;
			available_chords_list_len++;
		}
	}
	// If all possible chords arent taken, take a random chord from the available chord list
	if (available_chords_list_len)
	{
		return (available_chords_list[rand() % available_chords_list_len]);
	}
	// If all chords allready exist in the euclidean cirle, simply get a random chord in basic chord list
	else
	{
		return (chords_list[rand() % chord_list_length]);
	}
}

/**
 * @brief Initializing euclidean struct
 * @param [euclidean] Euclidean Circle struct
 * @param [steps_length] Number of steps in euclidean circle
 * @param [octave_size] Range of playable notes
 * @param [chord_list_length] Number of different possible chords in mode
 * @param [mode] Music mode (e_midi_modes)
 * @param [mode_beg_note] Reference note for the mode (e_notes)
 * @param [notes_per_cycle] Number of notes playables in euclidean circle
 * @param [mess_chance] Chance to skip a note/chord in euclidean circle (0-100)%
 * @param [min_chord_size] Minimum length of chord/note played in euclidean step
 * @param [max_chord_size] Maximum length of chord/note played in euclidean step
 * @param [min_velocity] Minimum velocity for chord/note played
 * @param [max_velocity] Maximum velocity for chord/note played
 * @param [min_steps_duration] Minimum duration for chord/note played in euclidean step
 * @param [max_steps_duration] Maximum duration for chord/note played in euclidean step
 */
void init_euclidean_struct(t_euclidean *euclidean, uint8_t steps_length,
						   uint8_t octave_size, uint8_t chord_list_length,
						   uint8_t mode, uint8_t mode_beg_note,
						   uint8_t notes_per_cycle, uint8_t mess_chance,
						   uint8_t min_chord_size, uint8_t max_chord_size,
						   uint8_t min_velocity, uint8_t max_velocity,
						   uint8_t min_steps_duration, uint8_t max_steps_duration)
{
	euclidean->euclidean_steps_length = steps_length;
	euclidean->euclidean_steps = (int16_t *)malloc(sizeof(int16_t) * steps_length);
	euclidean->octaves_size = octave_size;
	euclidean->chords_list_length = chord_list_length;
	euclidean->chords_list = (uint8_t *)malloc(sizeof(uint8_t) * chord_list_length);
	euclidean->mode = mode;
	euclidean->mode_beg_note = mode_beg_note;
	euclidean->notes_per_cycle = notes_per_cycle;
	euclidean->step_gap = steps_length / notes_per_cycle;
	euclidean->mess_chance = mess_chance;
	euclidean->min_chord_size = min_chord_size;
	euclidean->max_chord_size = max_chord_size;
	get_chords_list(euclidean->chords_list, chord_list_length);
	euclidean->min_velocity = min_velocity;
	euclidean->max_velocity = max_velocity;
	euclidean->min_steps_duration = min_steps_duration;
	euclidean->max_steps_duration = max_steps_duration;
	euclidean->current_step = 0;
	euclidean->initialized = 1;
}

/**
 * @brief Removing euclidean struct without leaks
 * @param [euclidean] Euclidean Circle struct
 */
void remove_euclidean_struct(t_euclidean *euclidean)
{
	free(euclidean->euclidean_steps);
	free(euclidean->chords_list);
}

/**
 * @brief Getting a new chord from chord list (Randomly)
 *        and ajusting his pitch (Randomly)
 *        this new chords are passing to the "euclidean_steps[]" variable
 * @param [euclidean] Euclidean Circle struct
 */
void get_new_euclidean_chords(t_euclidean *euclidean)
{
	for (uint8_t steps = 0; steps < euclidean->euclidean_steps_length; steps++)
	{
		if (steps % euclidean->step_gap == 0)
		{
			euclidean->euclidean_steps[steps] = get_new_chord_from_list(euclidean->chords_list,
																		euclidean->chords_list_length, steps, euclidean->euclidean_steps);
			euclidean->euclidean_steps[steps] |= (rand() % euclidean->octaves_size) << 8; // add octave property
		}
		else
		{
			euclidean->euclidean_steps[steps] = -1;
		}
	}
}

/**
 * @brief Print the content of euclidean Steps (Variable euclidean_steps[])
 * @param [euclidean] Euclidean Circle struct
 */
void print_euclidean_steps(t_euclidean *euclidean)
{
	for (uint8_t steps = 0; steps < euclidean->euclidean_steps_length; steps++)
	{
		printf("Step value : %d, octave : %d\n", euclidean->euclidean_steps[steps] & 0xFF,
			   (euclidean->euclidean_steps[steps] & 0xFF00) >> 8);
	}
	printf("Chord list : ");
	for (uint8_t chord_list_i = 0; chord_list_i < euclidean->chords_list_length; chord_list_i++)
	{
		printf("%d,", euclidean->chords_list[chord_list_i]);
	}
	printf("\n");
}

/**
 * @brief Function to write an multiple Euclidean midi step
 * @param [music_data] Midi struct
 * @param [euclidean] Struct that contain current euclidean values
 */
void write_euclidean_step(t_music_data *music_data, t_euclidean *euclidean)
{
	// Create chord if the current euclidean step contain note and the mess chance dont mess
	if (euclidean->euclidean_steps[euclidean->current_step] != -1 && rand() % 100 >= euclidean->mess_chance)
	{
		create_chord(music_data, playing_notes_duration, playing_notes, playing_notes_length,
					 euclidean->mode, euclidean->euclidean_steps[euclidean->current_step], euclidean->mode_beg_note,
					 map_number(rand() % 100, 0, 100, euclidean->min_chord_size, euclidean->max_chord_size),		  /*chord size*/
					 map_number(rand() % 100, 0, 100, euclidean->min_velocity, euclidean->max_velocity),			  /*velocity*/
					 map_number(rand() % 100, 0, 100, euclidean->min_steps_duration, euclidean->max_steps_duration)); /*note duration in steps*/
	}
	// Update the current euclidean step
	euclidean->current_step = (euclidean->current_step + 1) % euclidean->euclidean_steps_length;
}

// TODO: TEST NOTE SHIFTING
/**
 * @brief Function to shift notes in euclidean step
 * @param [euclidean] Struct that contain current euclidean values
 * @param [shift_value] Value in semitone to shift notes
 */
void shift_euclidean_steps(t_euclidean *euclidean, int shift_value)
{
	for (uint8_t steps = 0; steps < euclidean->euclidean_steps_length; steps++)
	{
		if (euclidean->euclidean_steps[steps] != -1)
		{

			uint32_t tmp = 0;

			int tmp_shift = shift_value;
			if (((euclidean->euclidean_steps[steps] & 0xFF00) >> 8) * 7 + (((euclidean->euclidean_steps[steps] & 0x00FF) + tmp_shift)) < 0)
				tmp_shift = tmp_shift - (((euclidean->euclidean_steps[steps] & 0xFF00) >> 8) * 7 + (((euclidean->euclidean_steps[steps] & 0x00FF) + tmp_shift)));
			tmp = (7 + ((euclidean->euclidean_steps[steps] & 0x00FF) + tmp_shift)) % 7;
			tmp |= ((((euclidean->euclidean_steps[steps] & 0xFF00) >> 8) + ((7 + ((euclidean->euclidean_steps[steps] & 0x00FF) + tmp_shift)) / 7 - 1)) << 8);
			euclidean->euclidean_steps[steps] = tmp;
		}
	}
}

uint16_t get_maximum_value(uint16_t a, uint16_t b, uint16_t c, uint16_t d, uint16_t e, uint16_t f)
{
	uint16_t max = a;

	if (b > max)
		max = b;
	if (c > max)
		max = c;
	if (d > max)
		max = d;
	if (e > max)
		max = e;
	if (f > max)
		max = f;
	return (max);
}

// a : valeur de départ
// b : valeur d'arrivée
// t : facteur d'interpolation (doit être entre 0 et 1)
float lerp(float a, float b, float t)
{
	return a + t * (b - a);
}

/**
 * @brief Function to write an multiple Euclidean (4 tps / measure)
 * @param [music_data] Midi struct
 * @param [sensors_data] Struct that contain current sensors datas
 */
void midi_write_multiple_euclidean(t_music_data *music_data, t_sensors *sensors_data)
{
	static t_euclidean euclidean_datas[EUCLIDEAN_DATAS_LENGTH];
	// Start with an reatribution of midi notes in euclidean Circle
	static uint8_t reset_needed = 1;
	// Variable to check last reset time (to reset notes in euclidan circle)
	static uint32_t last_time = 0;
	static uint16_t measure_count_1 = 0;
	static uint16_t measure_count_2 = 0;
	static uint16_t measure_count_3 = 0;

	static int16_t delta_shift = 10;

	static int16_t circle_3_reset_ctdown = 0;

	static int16_t smooth_photodiodes = -1;

	// Initializing ast reset time with the current timestamp
	if (last_time == 0)
	{
		last_time = time(NULL);
	}
	// Get the maximum value between all photodiodes values
	uint16_t max_photodiodes = get_maximum_value(sensors_data->photodiode_1,
												 sensors_data->photodiode_2,
												 sensors_data->photodiode_3,
												 sensors_data->photodiode_4,
												 sensors_data->photodiode_5,
												 sensors_data->photodiode_6);

	if (smooth_photodiodes == -1)
	{
		smooth_photodiodes = get_maximum_value(sensors_data->photodiode_1,
											   sensors_data->photodiode_2,
											   sensors_data->photodiode_3,
											   sensors_data->photodiode_4,
											   sensors_data->photodiode_5,
											   sensors_data->photodiode_6);
	}
	else
	{

		smooth_photodiodes = (uint32_t)lerp((float)smooth_photodiodes, get_maximum_value(sensors_data->photodiode_1, sensors_data->photodiode_2, sensors_data->photodiode_3, sensors_data->photodiode_4, sensors_data->photodiode_5, sensors_data->photodiode_6), 0.03);
	}

	// music_data->current_quarter_value_goal = (uint32_t)map_number((uint32_t)sensors_data->photodiode_1, 0, 4096, 1000000, 3500); // CHANGE TO THAT
	music_data->current_quarter_value = (uint32_t)lerp((float)music_data->current_quarter_value, (float)map_number((uint32_t)max_photodiodes, 0, 4096, 500000, 70000), 0.03);
	// update_quarter_value(music_data); // CHANGE TO THAT

	// Iterate for each euclidean circle
	for (uint8_t current_euclidean_data = 0; current_euclidean_data < EUCLIDEAN_DATAS_LENGTH; current_euclidean_data++)
	{
		// Initialize euclidean datas with sensors current values
		if (!euclidean_datas[current_euclidean_data].initialized)
		{
			init_euclidean_struct(&euclidean_datas[current_euclidean_data],
								  100,																	  /* steps_length */
								  2,																	  /* octave_size */
								  7,																	  /* chord_list_length */
								  M_MODE_MAJOR,															  /* mode */
								  A2,																	  /* mode_beg_note */
								  4,																	  /* notes_per_cycle */
								  (uint8_t)map_number(sensors_data->carousel_state, 0, 54, 0, 80),		  /* mess_chance */
								  1,																	  /* min_chord_size */
								  1,																	  /* max_chord_size */
								//   (uint8_t)map_number(sensors_data->temperature_8, 1500, 1550, 30, 15),	  /* min_velocity */
								  (uint8_t)40,	  /* min_velocity */
								//   (uint8_t)map_number(sensors_data->smooth_photodiodes, 0, 4096, 15, 40),	  /* min_velocity */
								//   (uint8_t)map_number(sensors_data->temperature_8, 1500, 1550, 100, 110), /* max_velocity */
								  (uint8_t)70, /* max_velocity */
								//   (uint8_t)map_number(sensors_data->smooth_photodiodes, 0, 4096, 60, 110), /* max_velocity */
								  (uint32_t)map_number((uint32_t)max_photodiodes, 0, 4096, 10, 30),		  /* min_steps_duration */
								  (uint32_t)map_number((uint32_t)max_photodiodes, 0, 4096, 14, 52)		  /* max_steps_duration */
			);

			if (current_euclidean_data == 0)
			{
				euclidean_datas[current_euclidean_data].octaves_size = 3;
				euclidean_datas[current_euclidean_data].euclidean_steps_length = 48; // 24
				euclidean_datas[current_euclidean_data].notes_per_cycle = 4;
				euclidean_datas[current_euclidean_data].step_gap =
					euclidean_datas[current_euclidean_data].euclidean_steps_length / euclidean_datas[current_euclidean_data].notes_per_cycle;

				euclidean_datas[current_euclidean_data].mess_chance = 50;
			}
			else if (current_euclidean_data == 1)
			{
				euclidean_datas[current_euclidean_data].euclidean_steps_length = (uint8_t)map_number((uint32_t)sensors_data->temperature_1, 400, 480, 26, 79); // 13 39
				euclidean_datas[current_euclidean_data].mode_beg_note = A2 - 12;
				euclidean_datas[current_euclidean_data].octaves_size = 3;
				euclidean_datas[current_euclidean_data].notes_per_cycle = 2;
				euclidean_datas[current_euclidean_data].mess_chance = 100;
				euclidean_datas[current_euclidean_data].min_velocity = 50;
				euclidean_datas[current_euclidean_data].max_velocity = 80;
			}
			else if (current_euclidean_data == 2)
			{
				euclidean_datas[current_euclidean_data].octaves_size = 3;
				euclidean_datas[current_euclidean_data].euclidean_steps_length = (uint8_t)map_number((uint32_t)sensors_data->temperature_2, 400, 480, 30, 91); // 15 45
				euclidean_datas[current_euclidean_data].notes_per_cycle = 4;
				euclidean_datas[current_euclidean_data].step_gap =
					euclidean_datas[current_euclidean_data].euclidean_steps_length / euclidean_datas[current_euclidean_data].notes_per_cycle;
				euclidean_datas[current_euclidean_data].mess_chance = 100;
								euclidean_datas[current_euclidean_data].min_velocity = 90;
				euclidean_datas[current_euclidean_data].max_velocity = 110;
			}
			else if (current_euclidean_data == 3)
			{
				euclidean_datas[current_euclidean_data].octaves_size = 2;
				euclidean_datas[current_euclidean_data].euclidean_steps_length = 15;
				euclidean_datas[current_euclidean_data].mode_beg_note = A2 - 12;
				euclidean_datas[current_euclidean_data].notes_per_cycle = 10;
				euclidean_datas[current_euclidean_data].step_gap =
					euclidean_datas[current_euclidean_data].euclidean_steps_length / euclidean_datas[current_euclidean_data].notes_per_cycle;
				euclidean_datas[current_euclidean_data].mess_chance = 100;
				euclidean_datas[current_euclidean_data].min_steps_duration = 20;
				euclidean_datas[current_euclidean_data].max_steps_duration = 30;
				euclidean_datas[current_euclidean_data].min_velocity = 90;
				euclidean_datas[current_euclidean_data].max_velocity = 100;
			}
		}
	}
	// Changed spectro to motor
	if (delta_shift != (uint16_t)map_number((uint32_t)sensors_data->motor_current, 0, 12000, 0, 10))
	{

		int16_t tmp = (uint32_t)map_number((uint32_t)sensors_data->motor_current, 0, 12000, 0, 10) - delta_shift;
		shift_euclidean_steps(&euclidean_datas[3], tmp);
		delta_shift += tmp;
	}

	if (circle_3_reset_ctdown % 2 == 0)
	{
		euclidean_datas[3].euclidean_steps_length = rand() % 6 + 3;

		euclidean_datas[3].notes_per_cycle = rand() % (euclidean_datas[3].euclidean_steps_length * 3 / 4) + 1;
		euclidean_datas[3].step_gap =
			euclidean_datas[3].euclidean_steps_length / euclidean_datas[3].notes_per_cycle;

		get_new_euclidean_chords(&euclidean_datas[3]);
		shift_euclidean_steps(&euclidean_datas[3], 10);
		delta_shift = 10;
	}

	if (circle_3_reset_ctdown <= 0)
	{
		circle_3_reset_ctdown = 30;
	}

	int tmp = (5 + (rand() % 10));

	if (music_data->current_quarter_value < 100000 && circle_3_reset_ctdown < tmp)
	{
		euclidean_datas[3].mess_chance = 70;
	}
	else
	{
		euclidean_datas[3].mess_chance = 100;
	}

	circle_3_reset_ctdown--;

	euclidean_datas[0].min_chord_size = (sensors_data->vin_current % 4) + 1;
	euclidean_datas[0].max_chord_size = (sensors_data->vin_current % 4) + 1;

	static uint16_t mode_requested = A2;
	static uint16_t type_mode_requested = M_MODE_MAJOR;

	if (sensors_data->carousel_state < 20)
	{
		mode_requested = A2;
	}
	else if (sensors_data->carousel_state < 40)
	{
		mode_requested = B2;
	}
	else if (sensors_data->carousel_state < 60)
	{
		mode_requested = C2;
	}
	else if (sensors_data->carousel_state < 80)
	{
		mode_requested = D2;
	}
	else if (sensors_data->carousel_state < 100)
	{
		mode_requested = E2;
	}
	else if (sensors_data->carousel_state < 110)
	{
		mode_requested = F2;
	}
	else
	{
		mode_requested = G2;
	}

	type_mode_requested = sensors_data->vin_current % 10;

	if (smooth_photodiodes < 500)
	{
		if (euclidean_datas[0].mode != type_mode_requested)
			reset_needed = 1;
		if (euclidean_datas[0].mode_beg_note != mode_requested)
			reset_needed = 1;
		euclidean_datas[0].mode_beg_note = mode_requested;
		euclidean_datas[1].mode_beg_note = mode_requested;
		euclidean_datas[2].mode_beg_note = mode_requested;

		euclidean_datas[0].mode = type_mode_requested;
		euclidean_datas[1].mode = type_mode_requested;
		euclidean_datas[2].mode = type_mode_requested;
	}

	// Change euclidean datas with sensors values
	// \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/
	if (smooth_photodiodes > 1024)
	{
		euclidean_datas[1].mess_chance = 50;

		if (euclidean_datas[1].notes_per_cycle != (uint8_t)map_number(sensors_data->temperature_9, 1500, 1550, 2, 5))
		{
			reset_needed = 1;
		}
		euclidean_datas[1].notes_per_cycle = (uint8_t)map_number(sensors_data->temperature_9, 1500, 1550, 2, 5);
		euclidean_datas[1].step_gap =
			euclidean_datas[1].euclidean_steps_length / euclidean_datas[1].notes_per_cycle;

		if (euclidean_datas[0].notes_per_cycle != (uint8_t)map_number(sensors_data->temperature_9, 1500, 1550, 4, 9))
		{
			reset_needed = 1;
		}
		euclidean_datas[0].notes_per_cycle = (uint8_t)map_number(sensors_data->temperature_9, 1500, 1550, 4, 9);
		euclidean_datas[0].step_gap =
			euclidean_datas[0].euclidean_steps_length / euclidean_datas[0].notes_per_cycle;

		euclidean_datas[0].min_steps_duration = (uint32_t)map_number((uint32_t)max_photodiodes, 0, 4096, 10, 30);
		euclidean_datas[1].min_steps_duration = (uint32_t)map_number((uint32_t)max_photodiodes, 0, 4096, 10, 30);
		euclidean_datas[2].min_steps_duration = (uint32_t)map_number((uint32_t)max_photodiodes, 0, 4096, 10, 30);

		euclidean_datas[0].max_steps_duration = (uint32_t)map_number((uint32_t)max_photodiodes, 0, 4096, 20, 40);
		euclidean_datas[1].max_steps_duration = (uint32_t)map_number((uint32_t)max_photodiodes, 0, 4096, 20, 40);
		euclidean_datas[2].max_steps_duration = (uint32_t)map_number((uint32_t)max_photodiodes, 0, 4096, 20, 40);
	}
	else
	{
		euclidean_datas[1].mess_chance = 100;
	}

	if (smooth_photodiodes > 2048)
	{
		euclidean_datas[2].mess_chance = 70;
	}
	else
	{
		euclidean_datas[2].mess_chance = 100;
	}
	// /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\.

	// Every X time of measures, get new notes in corresponding euclidean circle

	if (measure_count_1 > 32)
	{
		get_new_euclidean_chords(&euclidean_datas[0]);
		measure_count_1 = 0;
	}

	if (measure_count_2 > 55)
	{
		get_new_euclidean_chords(&euclidean_datas[1]);
		measure_count_2 = 0;
	}

	if (measure_count_3 > 83)
	{
		get_new_euclidean_chords(&euclidean_datas[2]);
		measure_count_3 = 0;
	}

	// Initialize notes or if requested to get new note, pick new random notes from allowed ones
	if (reset_needed)
	{
		for (uint8_t current_euclidean_data = 0; current_euclidean_data < EUCLIDEAN_DATAS_LENGTH; current_euclidean_data++)
		{
			get_new_euclidean_chords(&euclidean_datas[current_euclidean_data]);
		}

		reset_needed = 0;
	}

	if (LOG_ALL)
	{
		// Print the current euclidean circle values (Step value = index of note in chord list, octave = offset of note)
		for (uint8_t current_euclidean_data = 0; current_euclidean_data < EUCLIDEAN_DATAS_LENGTH; current_euclidean_data++)
		{
			printf("\nEuclidean Cirle %d :\n", current_euclidean_data);
			print_euclidean_steps(&euclidean_datas[current_euclidean_data]);
		}
	}

	uint16_t div_counter = 0;
	uint16_t div_goal = 512; // Whole division (quarter * 4)
	uint16_t looseness = 42; // 40; // Humanization in divisions delta, cannot be superior of (divgoal / 3 - divgoal / 4)

	// Write a midi measure (iterate on each quarter)
	for (uint8_t current_quarter = 0; current_quarter < 4; current_quarter++)
	{
		uint16_t current_div_duration;

		// For each euclidean circle, create corresponding chord
		for (uint8_t current_euclidean_data = 0; current_euclidean_data < EUCLIDEAN_DATAS_LENGTH; current_euclidean_data++)
		{

			write_euclidean_step(music_data, &euclidean_datas[current_euclidean_data]);
		}
		// Remove chords that end this quarter division
		remove_chord(music_data, playing_notes_duration, playing_notes, playing_notes_length);
		if (current_quarter == 3)
		{
			current_div_duration = div_goal - div_counter;
		}
		else
		{
			current_div_duration = div_goal / 4 - looseness + rand() % (looseness * 2);
		}
		// Create a MIDI delay of one division quarter
		midi_delay_divs(music_data, current_div_duration);
		div_counter += current_div_duration;
	}
	measure_count_1++;
	measure_count_2++;
	measure_count_3++;
}

/**
 * @brief Write end of midi file (and close it)
 * @param [music_data] Midi struct of an opened midi file
 */
void midi_write_end(t_music_data *music_data)
{
	MIDI_write_end_of_track(music_data->midi_file);
	MIDI_write_end_of_track(music_data->midi_file_redundancy);
	MIDI_write_track_length(music_data->midi_file, music_data->midi_mark);
	MIDI_write_track_length(music_data->midi_file_redundancy, music_data->midi_mark_redundancy);
	fclose(music_data->midi_file);
	fclose(music_data->midi_file_redundancy);
	music_data->midi_file = NULL;			 // new
	music_data->midi_file_redundancy = NULL; // new
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

	printf("OMG WTF : %s\n", date_time);

	if (sscanf(date_time, "%d/%d/%d %d:%d:%d", &YY, &mm, &DD, &HH, &MM, &SS) != 6)
	{
		return (0);
	}
	*date = YY * 10000 + mm * 100 + DD;
	*time = HH * 60 * 60 + MM * 60 + SS;
	return (1);
}

/**
 * @brief Print Time from seconds to DDhMMmSSs(Time)
 * @param [beg] String to print before the time
 * @param [end] String to print after the time
 */
void print_time(char *beg, uint32_t time, char *end)
{
	printf("%s%02dh%02dm%02ds(%d)%s", beg, time / 60 / 60, time / 60 % 60, time % 60, time, end);
}

void print_colored_value(char *name, int32_t value)
{
	printf("\033[1;34m%s\033[1;37m : \033[1;33m%d\033[0m\n", name, value);
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
		printf("\033[1;32mcurrent data\033[1;37m : \033[1;33m%d\033[0m\n", current_data);
		current_data++;
		print_colored_value("Photodiodes 1  ", sensors_tmp->photodiode_1);
		print_colored_value("Photodiodes 2  ", sensors_tmp->photodiode_2);
		print_colored_value("Photodiodes 3  ", sensors_tmp->photodiode_3);
		print_colored_value("Photodiodes 4  ", sensors_tmp->photodiode_4);
		print_colored_value("Photodiodes 5  ", sensors_tmp->photodiode_5);
		print_colored_value("Photodiodes 6  ", sensors_tmp->photodiode_6);

		print_colored_value("vin current    ", sensors_tmp->vin_current);
		print_colored_value("Temperature 1  ", sensors_tmp->temperature_1);
		print_colored_value("Temperature 2  ", sensors_tmp->temperature_2);
		print_colored_value("Temperature 3  ", sensors_tmp->temperature_3);
		print_colored_value("Temperature 4  ", sensors_tmp->temperature_4);
		print_colored_value("Temperature 5  ", sensors_tmp->temperature_5);
		print_colored_value("Temperature 6  ", sensors_tmp->temperature_6);
		print_colored_value("Temperature 7  ", sensors_tmp->temperature_7);
		print_colored_value("Temperature 8  ", sensors_tmp->temperature_8);
		print_colored_value("Temperature 9  ", sensors_tmp->temperature_9);
		print_colored_value("Temperature 10 ", sensors_tmp->temperature_10);

		print_colored_value("lid state      ", sensors_tmp->lid_state);
		print_colored_value("spectro current", sensors_tmp->spectro_current);
		print_colored_value("organ current  ", sensors_tmp->organ_current);
		print_colored_value("q7 current     ", sensors_tmp->q7_current);
		print_colored_value("5v current     ", sensors_tmp->t5v_current);
		print_colored_value("3.3v current   ", sensors_tmp->t3_3v_current);
		print_colored_value("motor current  ", sensors_tmp->motor_current);
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

t_sensors *get_next_buffer_values(t_circular_buffer circular_buffer, uint64_t *latest_timestamp)
{
	t_sensors *sensors = (t_sensors *)malloc(sizeof(t_sensors));
	for (uint16_t i = 0; i < BUFFER_ROUNDS; i++)
	{
		if (circular_buffer.data[(i + circular_buffer.older_block) % BUFFER_ROUNDS].timestamp > *latest_timestamp)
		{
			sensors->photodiode_1 = circular_buffer.data[(i + circular_buffer.older_block) % BUFFER_ROUNDS].photodiode_1;		// 0 - 4095  PD0
			sensors->photodiode_2 = circular_buffer.data[(i + circular_buffer.older_block) % BUFFER_ROUNDS].photodiode_2;		// 0 - 4095  PD1
			sensors->photodiode_3 = circular_buffer.data[(i + circular_buffer.older_block) % BUFFER_ROUNDS].photodiode_3;		// 0 - 4095  PD2
			sensors->photodiode_4 = circular_buffer.data[(i + circular_buffer.older_block) % BUFFER_ROUNDS].photodiode_4;		// 0 - 4095  PD3
			sensors->photodiode_5 = circular_buffer.data[(i + circular_buffer.older_block) % BUFFER_ROUNDS].photodiode_5;		// 0 - 4095  PD4
			sensors->photodiode_6 = circular_buffer.data[(i + circular_buffer.older_block) % BUFFER_ROUNDS].photodiode_6;		// 0 - 4095  PD5
			sensors->temperature_1 = circular_buffer.data[(i + circular_buffer.older_block) % BUFFER_ROUNDS].temperature_1;		// 0 - 4095  PT0
			sensors->temperature_2 = circular_buffer.data[(i + circular_buffer.older_block) % BUFFER_ROUNDS].temperature_2;		// 0 - 4095  PT1
			sensors->temperature_3 = circular_buffer.data[(i + circular_buffer.older_block) % BUFFER_ROUNDS].temperature_3;		// 0 - 4095  PT2
			sensors->temperature_4 = circular_buffer.data[(i + circular_buffer.older_block) % BUFFER_ROUNDS].temperature_4;		// 0 - 4095  PT3
			sensors->temperature_5 = circular_buffer.data[(i + circular_buffer.older_block) % BUFFER_ROUNDS].temperature_5;		// 0 - 4095  PT4
			sensors->temperature_6 = circular_buffer.data[(i + circular_buffer.older_block) % BUFFER_ROUNDS].temperature_6;		// 0 - 4095  PT5
			sensors->temperature_7 = circular_buffer.data[(i + circular_buffer.older_block) % BUFFER_ROUNDS].temperature_7;		// 0 - 4095  PT6
			sensors->temperature_8 = circular_buffer.data[(i + circular_buffer.older_block) % BUFFER_ROUNDS].temperature_8;		// 0 - 4095  PT1 OSCAR
			sensors->temperature_9 = circular_buffer.data[(i + circular_buffer.older_block) % BUFFER_ROUNDS].temperature_9;		// 0 - 4095  PT2 OSCAR
			sensors->temperature_10 = circular_buffer.data[(i + circular_buffer.older_block) % BUFFER_ROUNDS].temperature_10;	// 0 - 4095  PT3 OSCAR
			sensors->microphone = circular_buffer.data[(i + circular_buffer.older_block) % BUFFER_ROUNDS].microphone;			// 0 - 1     Mostly 0
			sensors->spectro_current = circular_buffer.data[(i + circular_buffer.older_block) % BUFFER_ROUNDS].spectro_current; // -32768 - 32767 I INA SPECTRO
			sensors->organ_current = circular_buffer.data[(i + circular_buffer.older_block) % BUFFER_ROUNDS].organ_current;		// -128 - 127     I INA OSCAR
			sensors->vin_current = circular_buffer.data[(i + circular_buffer.older_block) % BUFFER_ROUNDS].vin_current;			// -32768 - 32767 I INA DC_BUS
			sensors->q7_current = circular_buffer.data[(i + circular_buffer.older_block) % BUFFER_ROUNDS].q7_current;			// -128 - 127     I INA Q7
			sensors->t5v_current = circular_buffer.data[(i + circular_buffer.older_block) % BUFFER_ROUNDS].t5v_current;			// -128 - 127     I INA 5V
			sensors->t3_3v_current = circular_buffer.data[(i + circular_buffer.older_block) % BUFFER_ROUNDS].t3_3v_current;		// -128 - 127     I INA 3V3
			sensors->motor_current = circular_buffer.data[(i + circular_buffer.older_block) % BUFFER_ROUNDS].motor_current;		// -32768 - 32767 I INA MOTORS
			sensors->carousel_state = circular_buffer.data[(i + circular_buffer.older_block) % BUFFER_ROUNDS].carousel_state;	// 0 - 119  Lid Open = 0, Lid Closed 0 - 119
			sensors->lid_state = circular_buffer.data[(i + circular_buffer.older_block) % BUFFER_ROUNDS].lid_state;				// 0 - 53   Lid Open = 0, Lid Closed 53/2
			sensors->organ_1 = circular_buffer.data[(i + circular_buffer.older_block) % BUFFER_ROUNDS].organ_1;					// 0 - 4096  POT1 OSCAR
			sensors->organ_2 = circular_buffer.data[(i + circular_buffer.older_block) % BUFFER_ROUNDS].organ_2;					// 0 - 4096  POT2 OSCAR
			sensors->organ_3 = circular_buffer.data[(i + circular_buffer.older_block) % BUFFER_ROUNDS].organ_3;					// 0 - 4096  POT3 OSCAR
			sensors->organ_4 = circular_buffer.data[(i + circular_buffer.older_block) % BUFFER_ROUNDS].organ_4;					// 0 - 4096  POT4 OSCAR
			sensors->organ_5 = circular_buffer.data[(i + circular_buffer.older_block) % BUFFER_ROUNDS].organ_5;					// 0 - 4096  POT5 OSCAR
			sensors->organ_6 = circular_buffer.data[(i + circular_buffer.older_block) % BUFFER_ROUNDS].organ_6;					// 0 - 4096  POT6 OSCAR
			sensors->timestamp = circular_buffer.data[(i + circular_buffer.older_block) % BUFFER_ROUNDS].timestamp;				// 0 - oo
			sensors->next = NULL;
			*latest_timestamp = circular_buffer.data[(i + circular_buffer.older_block) % BUFFER_ROUNDS].timestamp;
			return (sensors);
		}
	}
	free(sensors);
	return (NULL);
}

t_sensors *get_new_buffer_values(t_circular_buffer circular_buffer, uint64_t *latest_timestamp, t_sensors *sensorsData)
{
	if (!sensorsData)
	{
		sensorsData = get_next_buffer_values(circular_buffer, latest_timestamp);
	}
	else
	{
		t_sensors *tmp = sensorsData;
		while ((tmp->next = get_next_buffer_values(circular_buffer, latest_timestamp)) != 0)
		{
			tmp = tmp->next;
		}
		if (tmp == sensorsData)
		{
			sleep(3);
		}
	}
	return (sensorsData);
}

/**
 * @brief Create file path if doesnt exists
 * @param [file_path] File path, /!\ cant be const /!\
 * @param [mode] Permission bit masks for mode
 */
int make_path(char *file_path, mode_t mode)
{
	for (char *p = strchr(file_path + 1, '/'); p; p = strchr(p + 1, '/'))
	{
		*p = '\0';
		if (mkdir(file_path, mode) == -1)
		{
			if (errno != EEXIST)
			{
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
 * @param [output_directory_redundancy] location of redundancy midi file
 * @param [sensors_data] Struct that contain current sensors datas
 */
void create_dated_midi_file(t_music_data *music_data, char *output_directory,
							char *output_directory_redundancy, t_sensors *sensors_data)
{
	time_t now;
	struct tm tm_now;
	char fileName[sizeof("AAAA_mm_JJ__HH_MM_SS.mid")];
	char filePath[sizeof(fileName) + strlen(output_directory) + 1];

	char fileNameRedundancy[sizeof("AAAA_mm_JJ__HH_MM_SS_.mid")];
	char filePathRedundancy[sizeof(fileNameRedundancy) + strlen(output_directory_redundancy) + 1];

	if (sensors_data)
	{

		time_t test = sensors_data->timestamp;

		tm_now = *localtime(&test); //
		char s_now[sizeof("AAAA_mm_JJ__HH_MM_SS")];

		strftime(s_now, sizeof("AAAA_mm_JJ__HH_MM_SS"), "%Y_%m_%d__%H_%M_%S", &tm_now); //

		sprintf(fileName, "%s.mid", s_now);
		sprintf(fileNameRedundancy, "%s_.mid", s_now);
	}
	else // Ne passe normalement jamais ICI
	{
		now = time(NULL);
		tm_now = *localtime(&now);
		strftime(fileName, sizeof(fileName), "%Y_%m_%d__%H_%M_%S.mid", &tm_now);
		strftime(fileNameRedundancy, sizeof(fileNameRedundancy), "%Y_%m_%d__%H_%M_%S_.mid", &tm_now);
	}

	sprintf(filePath, "%s/%s", output_directory, fileName);
	sprintf(filePathRedundancy, "%s/%s", output_directory_redundancy, fileNameRedundancy);
	make_path(filePath, 0755);
	make_path(filePathRedundancy, 0755);
	midi_setup_file(filePath, filePathRedundancy, music_data);
}

/**
 * @brief Terminate properly midi notes (for exiting file/program)
 * @param [music_data] Midi struct of midi file
 */
void terminate_notes(t_music_data *music_data)
{
	for (uint8_t playing_notes_i = 0; playing_notes_i < playing_notes_length; playing_notes_i++)
	{
		if (playing_notes_duration[playing_notes_i])
		{
			// end note
			midi_write_measure_note(music_data, OFF, 1, playing_notes[playing_notes_i], 0);
			playing_notes[playing_notes_i] = 0;
		}
	}
}

/**
 * @brief Initialize music_data
 * @param [music_data] Midi struct of midi file
 * @param [partition_duration] Partition duration in minutes
 * @param [quarter_value] Quarter value in micro-seconds
 * @param [tempo_acceleration] Acceleration per measure in percentage (1.0=100%, 0.05=5%)
 *
 */
void init_music_data(t_music_data *music_data, uint32_t partition_duration,
					 uint32_t quarter_value, uint32_t quarter_value_goal,
					 float tempo_acceleration)
{
	music_data->partition_duration = 60000000 * partition_duration; // 10 minutes
	music_data->measure_value = 500000 * 4;							// useless
	music_data->measures_writed = 0;
	music_data->delta_time = 0;
	music_data->quarter_value_step = 100000;
	music_data->quarter_value_goal = quarter_value_goal;
	music_data->quarter_value = 500000; // define metadata 500000=120bpm
	music_data->current_quarter_value = quarter_value;
	music_data->quarter_value_step_updating = tempo_acceleration; // Acceleration per measure in percentage (1.0=100%, 0.05=5%)
}

int main(int argc, char **argv)
{
	// durée d'une partition 40 000 000us
	t_music_data music_data = {0};
	init_music_data(&music_data, 10, 1000000, 250000, 0.03);

	uint64_t latest_timestamp = 0;

	char *outputDirectory = "midi_files";
	char *outputDirectoryRedundancy = "midi_files";

	if (argc == 2)
	{
		outputDirectory = argv[1];
		outputDirectoryRedundancy = argv[1];
	}
	else if (argc == 3)
	{
		outputDirectory = argv[1];
		outputDirectoryRedundancy = argv[2];
	}

	signal(SIGTERM, (void (*)(int))terminate_session);
	signal(SIGINT, (void (*)(int))terminate_session);
	signal(SIGSTOP, (void (*)(int))terminate_session);

	char *currentDataFileName;
	t_sensors *sensorsData;
	sensorsData = NULL;
	currentDataFileName = (char *)malloc(sizeof(char) * 200);

	int semid = semget(SEM_KEY, 1, 0);
	if (semid == -1)
	{
		perror("semget");
		return 1;
	}

	int shmid;
	struct shmseg *shmp;
	shmid = shmget(SHM_KEY, sizeof(struct shmseg), 0644 | IPC_CREAT);

	if (shmid == -1)
	{
		perror("Shared memory");
		return 1;
	}

	// Attach to the segment to get a pointer to it.
	shmp = shmat(shmid, NULL, 0);
	if (shmp == (void *)-1)
	{
		perror("Shared memory attach");
		return 1;
	}

	// Main loop
	while (!g_exit_requested && shmp->complete != 1)
	{
		if (!sensorsData || !sensorsData->next)
		{
			semop(semid, &P, 1); // Verrouiller le sémaphore

			sensorsData = get_new_buffer_values(*(t_circular_buffer *)shmp->buf, &latest_timestamp, sensorsData);
			semop(semid, &V, 1); // Déverrouiller le sémaphore

			if (sensorsData != 0)
			{
				printf("--New values from sender taken--\n");
				print_sensors_data(sensorsData);
			}
			else
			{
				printf("--Waiting for new values from sender--\n");
				sleep(5);
			}
			if (shmp->cnt == -1)
			{
				perror("read");
				return 1;
			}
		}
		// Si on a pas de fichier midi ouvert on en ouvre un nouveau
		if (!music_data.midi_file && sensorsData)
		{
			create_dated_midi_file(&music_data, outputDirectory,
								   outputDirectoryRedundancy, sensorsData);
			music_data.measures_writed = 0; //
			music_data.data_time = 0;		//
			music_data.delta_time = 0;
		}
		// Tant que les datas ne sont pas finies et qu'il reste
		// des mesures á ecrire dans le fichier midi, on ecrit les
		// datas dans le fichier midi
		while (music_data.midi_file && sensorsData) // && music_data.midi_file_redundancy?
		{
			if (music_data.data_time == 0)
			{
				music_data.data_time = sensorsData->timestamp; // * 1000000;
				music_data.entry_data_time = sensorsData->timestamp;
			}

			if (music_data.data_time <= sensorsData->timestamp)
			{

				midi_write_multiple_euclidean(&music_data, sensorsData);

				music_data.measures_writed++; //
				music_data.delta_time += (music_data.current_quarter_value * 4);
				music_data.data_time = music_data.entry_data_time + ((music_data.delta_time) / 1000000);
			}

			if (music_data.delta_time >= music_data.partition_duration)
			{
				printf("Midi write end\n");
				terminate_notes(&music_data);
				midi_write_end(&music_data);
			}

			if (!sensorsData->next)
			{
				break;
				// Aller rechercher des données (et pas passer au next)
			}

			while (sensorsData && music_data.data_time >= sensorsData->timestamp)
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
					sensorsData = NULL; // free?
				}
			}
		}
	}

	// Signaux ->
	// Fini le midi, (lui donne un nom avec date debut/fin)
	// Note dans les LOGS le temps d'arret midi et log

	if (shmdt(shmp) == -1)
	{
		perror("shmdt");
		return 1;
	}
	if (music_data.midi_file)
	{
		printf("Midi write end 2\n");
		terminate_notes(&music_data);

		midi_write_end(&music_data);
	}
	return (0);
}
