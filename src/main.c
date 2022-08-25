#include "../inc/midi.h"
#include "../inc/json_parser.h"
#include "../inc/midi_notes.h"
#include "../inc/midi_modes.h"
#include "../inc/midi_euclidean.h"

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
	//metadatas
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
	printf("\033[1;35mwrite measure note : state=%s channel=%d note=%d velocity=%d\033[1;37m\n\n",
		   (state == ON ? "ON" : "OFF"), channel, note, velocity);
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
	// music_data->current_quarter_value / (music_data->quarter_value / 128)
	// MIDI_delta_time(music_data->midi_file, QUARTER);
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
	// music_data->current_quarter_value / (music_data->quarter_value / 128)
	// MIDI_delta_time(music_data->midi_file, QUARTER);
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
	// music_data->current_quarter_value / (music_data->quarter_value / 128)
	// MIDI_delta_time(music_data->midi_file, QUARTER);
	//  / 512 => base
	MIDI_delta_time(music_data->midi_file, music_data->current_quarter_value / (music_data->quarter_value / divs));
	MIDI_delta_time(music_data->midi_file_redundancy, music_data->current_quarter_value / (music_data->quarter_value / divs));
	MIDI_Note(music_data->midi_file, OFF, 1, 10, 0);
	MIDI_Note(music_data->midi_file_redundancy, OFF, 1, 10, 0);
}

void get_music_mode(uint8_t gamme[7], uint8_t music_mode)
{
	// uint8_t *mode_phrygien = g_midi_mode[M_MODE_DORIEN_DIEZ4].mode_sequence;

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
	// printf("\n\n\n\nIn update quarter value func\n\n\n\n\n\n\n");

	if (music_data->current_quarter_value != music_data->quarter_value_goal)
	{
		if (music_data->current_quarter_value < music_data->quarter_value_goal)
		{
			// if (music_data->quarter_value_goal - music_data->current_quarter_value < music_data->quarter_value_step)
			if (music_data->quarter_value_goal < music_data->current_quarter_value + (uint32_t)((float)music_data->current_quarter_value * music_data->quarter_value_step_updating))
			{
				// printf("++Updated current quarter value :%d\n", music_data->current_quarter_value);
				// printf("++Updated NOT added :%d\n", (uint32_t)((float)music_data->current_quarter_value * music_data->quarter_value_step_updating));

				// printf("--Updated add :", (uint32_t)((float)music_data->current_quarter_value * music_data->quarter_value_step_updating));
				music_data->current_quarter_value = music_data->quarter_value_goal;
			}
			else
			{
				// printf("++Updated current quarter value :%d\n", music_data->current_quarter_value);
				// printf("++Updated add :%d\n", (uint32_t)((float)music_data->current_quarter_value * music_data->quarter_value_step_updating));
				music_data->current_quarter_value += (uint32_t)((float)music_data->current_quarter_value * music_data->quarter_value_step_updating);
			}
		}
		else
		{
			// if (music_data->current_quarter_value - music_data->quarter_value_goal < music_data->quarter_value_step)
			if (music_data->quarter_value_goal > music_data->current_quarter_value - (uint32_t)((float)music_data->current_quarter_value * music_data->quarter_value_step_updating))
			{
				// printf("--Updated current quarter value :%d\n", music_data->current_quarter_value);
				// printf("--Updated goal quarter value :%d\n", music_data->quarter_value_goal);
				// printf("--Updated NOT add :%d\n", (uint32_t)((float)music_data->current_quarter_value * music_data->quarter_value_step_updating));
				music_data->current_quarter_value = music_data->quarter_value_goal;

				// printf("fhew fhewiu fheiwuhf uiewhf iuewhf iewhifh ewiuf hiuewh fiuewhiuf ewiuh fhiufew");
			}
			else
			{
				// printf("--Updated current quarter value :%d\n", music_data->current_quarter_value);
				// printf("--Updated add :%d\n", (uint32_t)((float)music_data->current_quarter_value * music_data->quarter_value_step_updating));
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
	// uint8_t chords_list[chords_size];

	for (uint8_t i = 0; i < chords_size; i++)
	{ //i * 2
		// chords_list[i] = /* starting_note + */ g_midi_mode[mode].mode_sequence[(i * 2) % 7];
		chords_list[i] = /* starting_note + */ (i * 2) % 7;
	}
	// return (chords_list);
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

	printf("\033[1;32mChord to play\n");
	for (int i = 0; i < chord_size; i++)
	{
		printf("Note Chord[%d] : %d\n", i, note_offset + ((note_i & 0xFF00) >> 8) * 12 + g_midi_mode[mode].mode_sequence[((note_i & 0xFF) + 2 * i) % 7] + 12 * (((note_i & 0xFF) + 2 * i) / 7));
	}
	printf("\033[1;37m\n");

	for (uint8_t current_note = 0; current_note < chord_size; current_note++)
	{
		current_note_done = false;
		//If the note is currently played
		//Just add time to that note
		for (uint8_t playing_notes_i = 0; playing_notes_i < playing_notes_length; playing_notes_i++)
		{
			if (playing_notes[playing_notes_i] == note_offset + ((note_i & 0xFF00) >> 8) * 12 + g_midi_mode[mode].mode_sequence[((note_i & 0xFF) + 2 * current_note) % 7] + 12 * (((note_i & 0xFF) + 2 * current_note) / 7))
			{
				// printf("Note_i P1: %d, current_note : %d, calcul : %d, calcul_tab : %d\n", (note_i & 0b1111), current_note,((note_i & 0b1111) + 2 * current_note)% 7,
				// g_midi_mode[mode].mode_sequence[((note_i & 0b1111) + 2 * current_note)% 7] );

				playing_notes_duration[playing_notes_i] = steps_duration;
				current_note_done = true;
			}
		}
		//If the note isnt played
		//Create that note !
		for (uint8_t playing_notes_i = 0; playing_notes_i < playing_notes_length && !current_note_done; playing_notes_i++)
		{
			if (!playing_notes_duration[playing_notes_i])
			{
				// printf("Note_i : %d, current_note : %d, calcul : %d, calcul_tab : %d\n", (note_i & 0b1111), current_note,((note_i & 0b1111) + 2 * current_note)% 7,
				// g_midi_mode[mode].mode_sequence[((note_i & 0b1111) + 2 * current_note)% 7] );

				playing_notes_duration[playing_notes_i] = steps_duration;
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

	printf("\033[1;96mRM func\n");
	for (uint8_t i = 0; i < playing_notes_length; i++)
	{
		printf("Playing notes[%d] : N = %d, D = %d\n", i, playing_notes[i], playing_notes_duration[i]);
	}
	printf("\033[1;37m\n");

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
void init_euclidean_struct(t_euclidean *euclidean, uint8_t steps_length, \
						   uint8_t octave_size, uint8_t chord_list_length, \
						   uint8_t mode, uint8_t mode_beg_note, \
						   uint8_t notes_per_cycle, uint8_t mess_chance, \
						   uint8_t min_chord_size, uint8_t max_chord_size, \
						   uint8_t min_velocity, uint8_t max_velocity, \
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
void	remove_euclidean_struct(t_euclidean *euclidean)
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
			euclidean->euclidean_steps[steps] = get_new_chord_from_list(euclidean->chords_list, \
								 euclidean->chords_list_length, steps, euclidean->euclidean_steps);
			euclidean->euclidean_steps[steps] |= (rand() % euclidean->octaves_size) << 8; //add octave property
			printf("New step : %d\n", euclidean->euclidean_steps[steps]);
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
void	print_euclidean_steps(t_euclidean *euclidean)
{
	for (uint8_t steps = 0; steps < euclidean->euclidean_steps_length; steps++)
	{
		printf("Step value : %d, octave : %d\n", euclidean->euclidean_steps[steps] & 0xFF, \
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
void	write_euclidean_step(t_music_data *music_data, t_euclidean *euclidean)
{
	// Create chord if the current euclidean step contain note and the mess chance dont mess
	if (euclidean->euclidean_steps[euclidean->current_step] != -1 && rand() % 100 >= euclidean->mess_chance)
	{
		create_chord(music_data, playing_notes_duration, playing_notes, playing_notes_length,
						euclidean->mode, euclidean->euclidean_steps[euclidean->current_step], euclidean->mode_beg_note,
						map_number(rand() % 100, 0, 100, euclidean->min_chord_size, euclidean->max_chord_size),			/*chord size*/
						map_number(rand() % 100, 0, 100, euclidean->min_velocity, euclidean->max_velocity),				/*velocity*/
						map_number(rand() % 100, 0, 100, euclidean->min_steps_duration, euclidean->max_steps_duration)); /*note duration in steps*/
	}
	// Update the current euclidean step
	euclidean->current_step = (euclidean->current_step + 1) % euclidean->euclidean_steps_length;
}

// TODO: TESTING NOTE SHIFTING AND FIX THIS FCKING SHIT + TROUVER UN FIX POUR LE RESET ETC...
void shift_euclidean_steps(t_euclidean *euclidean, int shift_value)
{
	for (uint8_t steps = 0; steps < euclidean->euclidean_steps_length; steps++)
	{
		if (euclidean->euclidean_steps[steps] != -1)
		{

			uint32_t tmp = 0;

			int tmp_shift = shift_value;
			// printf("Bidule : %d\n", ((euclidean->euclidean_steps[steps] & 0xFF00) >> 8) * 7 + (((euclidean->euclidean_steps[steps] & 0x00FF) + tmp_shift)));
			if (((euclidean->euclidean_steps[steps] & 0xFF00) >> 8) * 7 + (((euclidean->euclidean_steps[steps] & 0x00FF) + tmp_shift)) < 0)
				tmp_shift = tmp_shift - (((euclidean->euclidean_steps[steps] & 0xFF00) >> 8) * 7 + (((euclidean->euclidean_steps[steps] & 0x00FF) + tmp_shift)));
			// printf("New shift : %d\n", tmp_shift);
			tmp = (7 + ((euclidean->euclidean_steps[steps] & 0x00FF) + tmp_shift)) % 7;
			tmp |= ((((euclidean->euclidean_steps[steps] & 0xFF00) >> 8) + ((7 + ((euclidean->euclidean_steps[steps] & 0x00FF) + tmp_shift)) / 7 - 1)) << 8);
			euclidean->euclidean_steps[steps] = tmp;


		}
	}
}


/**
  * @brief Function to write an multiple Euclidean (4 tps / measure)
  * @param [music_data] Midi struct
  * @param [sensors_data] Struct that contain current sensors datas
*/
void midi_write_multiple_euclidean(t_music_data *music_data, t_sensors *sensors_data)
{
	// Number of euclidean "Circles"
	const uint8_t euclidean_datas_length = 3;
	// Initializing euclidean "Circles" datas with NULL
	static t_euclidean euclidean_datas[euclidean_datas_length] = {0};
	// Start with an reatribution of midi notes in euclidean Circle
	static uint8_t reset_needed = 1;
	// Variable to check last reset time (to reset notes in euclidan circle)
	static uint32_t last_time = 0;
	// Initializing ast reset time with the current timestamp
	if (last_time == 0)
	{
		last_time = time(NULL);
	}


	music_data->quarter_value_goal = (uint32_t)map_number((uint32_t)sensors_data->photodiode_1, 0, 4096, 1000000, 100000);
	// Update Midi quarter value to move towards the quarter goal value
	update_quarter_value(music_data);
	// Iterate for each euclidean circle
	for (uint8_t current_euclidean_data = 0; current_euclidean_data < euclidean_datas_length; current_euclidean_data++)
	{
		// Initialize euclidean datas with sensors current values
		if (!euclidean_datas[current_euclidean_data].initialized)
		{
			init_euclidean_struct(&euclidean_datas[current_euclidean_data],
									20, /* steps_length */
									2, /* octave_size */
									7, /* chord_list_length */
									M_MODE_HARMONIC_MINOR, /* mode */
									A2, /* mode_beg_note */
									4, /* notes_per_cycle */
									(uint8_t)map_number(sensors_data->carousel_state, 0, 180, 80, 0), /* mess_chance */
									1, /* min_chord_size */
									1, /* max_chord_size */
									(uint8_t)map_number(sensors_data->organ_1, 0, 1024, 48, 35), /* min_velocity */
									(uint8_t)map_number(sensors_data->organ_1, 0, 1024, 70, 74), /* max_velocity */
									10, /* min_steps_duration */
									14 /* max_steps_duration */
									);
			if (current_euclidean_data == 0)
			{
				euclidean_datas[current_euclidean_data].octaves_size = 3;
				euclidean_datas[current_euclidean_data].euclidean_steps_length = 24;
				euclidean_datas[current_euclidean_data].notes_per_cycle = 4;
				euclidean_datas[current_euclidean_data].step_gap = \
					euclidean_datas[current_euclidean_data].euclidean_steps_length \
						/ euclidean_datas[current_euclidean_data].notes_per_cycle;

				euclidean_datas[current_euclidean_data].mess_chance = 30;
			}
			else if (current_euclidean_data == 1)
			{
				euclidean_datas[current_euclidean_data].euclidean_steps_length = 12;
				euclidean_datas[current_euclidean_data].notes_per_cycle = 2;
				euclidean_datas[current_euclidean_data].mess_chance = 100;
			}
			else if (current_euclidean_data == 2)
			{
				euclidean_datas[current_euclidean_data].octaves_size = 3;
				euclidean_datas[current_euclidean_data].euclidean_steps_length = 14;
				euclidean_datas[current_euclidean_data].notes_per_cycle = 4;
				euclidean_datas[current_euclidean_data].step_gap = \
					euclidean_datas[current_euclidean_data].euclidean_steps_length \
						/ euclidean_datas[current_euclidean_data].notes_per_cycle;
				euclidean_datas[current_euclidean_data].mess_chance = 100;
			}
		}
	}

	// Change euclidean datas with sensors values
	// \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/
	if ((uint32_t)sensors_data->photodiode_1 > 1024)
	{
		euclidean_datas[1].mess_chance = (uint32_t)map_number((uint32_t)sensors_data->photodiode_1, 0, 4096, 60, 20);
		
		if (euclidean_datas[1].notes_per_cycle != (uint8_t)map_number(sensors_data->organ_1, 0, 1024, 2, 5))
		{
			reset_needed = 1;
		}
		euclidean_datas[1].notes_per_cycle = (uint8_t)map_number(sensors_data->organ_1, 0, 1024, 2, 8);
				euclidean_datas[1].step_gap = \
					euclidean_datas[1].euclidean_steps_length \
						/ euclidean_datas[1].notes_per_cycle;

		euclidean_datas[2].notes_per_cycle = (uint8_t)map_number(sensors_data->organ_1, 0, 1024, 2, 8);
			euclidean_datas[2].step_gap = \
				euclidean_datas[2].euclidean_steps_length \
					/ euclidean_datas[2].notes_per_cycle;



		if (euclidean_datas[0].notes_per_cycle != (uint8_t)map_number(sensors_data->organ_1, 0, 1024, 4, 9))
		{
			reset_needed = 1;
		}
				euclidean_datas[0].notes_per_cycle = (uint8_t)map_number(sensors_data->organ_1, 0, 1024, 4, 9);
				euclidean_datas[0].step_gap = \
					euclidean_datas[0].euclidean_steps_length \
						/ euclidean_datas[0].notes_per_cycle;

		euclidean_datas[0].min_steps_duration = (uint8_t)map_number(sensors_data->organ_1, 0, 1024, 10, 2);
		euclidean_datas[1].min_steps_duration = (uint8_t)map_number(sensors_data->organ_1, 0, 1024, 10, 2);
		euclidean_datas[2].min_steps_duration = (uint8_t)map_number(sensors_data->organ_1, 0, 1024, 10, 2);

		euclidean_datas[0].max_steps_duration = (uint8_t)map_number(sensors_data->organ_1, 0, 1024, 14, 3);
		euclidean_datas[1].max_steps_duration = (uint8_t)map_number(sensors_data->organ_1, 0, 1024, 14, 3);
		euclidean_datas[2].max_steps_duration = (uint8_t)map_number(sensors_data->organ_1, 0, 1024, 14, 3);
		// (uint32_t)map_number((uint32_t)sensors_data->photodiode_1, 0, 4096, 60, 0);
		
	}

	if ((uint32_t)sensors_data->photodiode_1 > 2048)
	{
		euclidean_datas[2].mess_chance = (uint32_t)map_number((uint32_t)sensors_data->photodiode_1, 2048, 4096, 80, 20);

	}
	// /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\.


	// Each 30-60 seconds, request to get new notes in euclidean circles
	printf("Time : %ld", time(NULL));
	printf("Last Time : %d", last_time);
	if (time(NULL) - last_time > 30 + rand() % 30)
	{
		reset_needed = 1;
		last_time = time(NULL);
		printf("\n\n\n\n\n! RESETING !\n\n\n\n\n\n");
	}

	// If request to get new note, pick new random notes from allowed ones
	if (reset_needed)
	{
		for (uint8_t current_euclidean_data = 0; current_euclidean_data < euclidean_datas_length; current_euclidean_data++)
		{
			get_new_euclidean_chords(&euclidean_datas[current_euclidean_data]);
		}
		reset_needed = 0;
	}

	// Print the current euclidean circle values (Step value = index of note in chord list, octave = offset of note)
	for (uint8_t current_euclidean_data = 0; current_euclidean_data < euclidean_datas_length; current_euclidean_data++)
	{
		printf("\nEuclidean Cirle %d :\n", current_euclidean_data);
		print_euclidean_steps(&euclidean_datas[current_euclidean_data]);
	}


	uint16_t div_counter = 0;
	uint16_t div_goal = 512; // Whole division (quarter * 4)
	uint16_t looseness = 40; // Humanization in divisions delta, cannot be superior of divgoal / 8

	// Write a midi measure (iterate on each quarter) 
	for (uint8_t current_quarter = 0; current_quarter < 4; current_quarter++)
	{
		uint16_t current_div_duration;
		// For each euclidean circle, create corresponding chord
		for (uint8_t current_euclidean_data = 0; current_euclidean_data < euclidean_datas_length; current_euclidean_data++)
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

}

/**
  * @brief Function to write an Euclidean measure
  * @param [music_data] Midi struct
  * @param [sensors_data] Struct that contain current sensors datas
*/
void midi_write_euclidean_measure(t_music_data *music_data, t_sensors *sensors_data)
{
// Number of euclidean "Circles"
	// const uint8_t EUCLIDEAN_DATAS_LENGTH = 3;
	// Initializing euclidean "Circles" datas with NULL
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

	// Initializing ast reset time with the current timestamp
	if (last_time == 0)
	{
		last_time = time(NULL);
	}

	// print_sensors_data(sensors_data);

	music_data->quarter_value_goal = (uint32_t)map_number((uint32_t)sensors_data->photodiode_1, 0, 4096, 100000000, 35000000);
	// Update Midi quarter value to move towards the quarter goal value
	// printf("\033[1;32mmusic data current quarter value : %d\033[1;37m\n", music_data->current_quarter_value);
	// 5000000
	music_data->current_quarter_value = (uint32_t)map_number((uint32_t)sensors_data->photodiode_1, 0, 4096, 50000000, 1800000); // RM THAT !!
	// music_data->current_quarter_value = (uint32_t)map_number((uint32_t)sensors_data->photodiode_1, 0, FIX_4096, 100000000, 3500000); // RM THAT !!


	// printf("\033[1;32mmusic data current quarter value after  : %d\033[1;37m\n", music_data->current_quarter_value);

	// update_quarter_value(music_data); RM TO FIX
	// Iterate for each euclidean circle
	for (uint8_t current_euclidean_data = 0; current_euclidean_data < EUCLIDEAN_DATAS_LENGTH; current_euclidean_data++)
	{
		// Initialize euclidean datas with sensors current values
		if (!euclidean_datas[current_euclidean_data].initialized)
		{
			init_euclidean_struct(&euclidean_datas[current_euclidean_data],
								  20,																/* steps_length */
								  2,																/* octave_size */
								  7,																/* chord_list_length */
								  M_MODE_MAJOR,														/* mode */
								  A2,																/* mode_beg_note */
								  4,																/* notes_per_cycle */
								  (uint8_t)map_number(sensors_data->carousel_state, 0, 119, 80, 0), /* mess_chance */
								  1,																/* min_chord_size */
								  1,																/* max_chord_size */
								  (uint8_t)map_number(sensors_data->organ_1, 0, 1024, 48, 35),		/* min_velocity */
								  (uint8_t)map_number(sensors_data->organ_1, 0, 1024, 70, 74),		/* max_velocity */
								  10,																/* min_steps_duration */
								  14																/* max_steps_duration */
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
				euclidean_datas[current_euclidean_data].euclidean_steps_length = (uint8_t)map_number((uint32_t)sensors_data->temperature_1, 0, 4096, 26, 79); // 13 39
				euclidean_datas[current_euclidean_data].mode_beg_note = A2 - 12;
				euclidean_datas[current_euclidean_data].octaves_size = 3;
				// euclidean_datas[current_euclidean_data].euclidean_steps_length = 13;
				euclidean_datas[current_euclidean_data].notes_per_cycle = 2;
				euclidean_datas[current_euclidean_data].mess_chance = 100;
			}
			else if (current_euclidean_data == 2)
			{
				euclidean_datas[current_euclidean_data].octaves_size = 3;
				euclidean_datas[current_euclidean_data].euclidean_steps_length = (uint8_t)map_number((uint32_t)sensors_data->temperature_2, 0, 4096, 30, 91); // 15 45
				// euclidean_datas[current_euclidean_data].euclidean_steps_length = 15;
				euclidean_datas[current_euclidean_data].notes_per_cycle = 4;
				euclidean_datas[current_euclidean_data].step_gap =
					euclidean_datas[current_euclidean_data].euclidean_steps_length / euclidean_datas[current_euclidean_data].notes_per_cycle;
				euclidean_datas[current_euclidean_data].mess_chance = 100;
			}
			else if (current_euclidean_data == 3)
			{
				euclidean_datas[current_euclidean_data].octaves_size = 2;
				euclidean_datas[current_euclidean_data].euclidean_steps_length = 5;
				// euclidean_datas[current_euclidean_data].euclidean_steps_length = 15;
				euclidean_datas[current_euclidean_data].mode_beg_note = A2 - 12;
				euclidean_datas[current_euclidean_data].notes_per_cycle = 2;
				euclidean_datas[current_euclidean_data].step_gap =
					euclidean_datas[current_euclidean_data].euclidean_steps_length / euclidean_datas[current_euclidean_data].notes_per_cycle;
				euclidean_datas[current_euclidean_data].mess_chance = 100;
				euclidean_datas[current_euclidean_data].min_steps_duration = 1;
				euclidean_datas[current_euclidean_data].max_steps_duration = 2;
			}
		}
	}


	if (delta_shift != (int16_t)map_number((uint32_t)sensors_data->spectro_current, 0, 33535, 0, 10))
	{

		int16_t tmp = (uint32_t)map_number((uint32_t)sensors_data->spectro_current, 0, 33535, 0, 10) - (uint32_t)delta_shift;
		shift_euclidean_steps(&euclidean_datas[3], tmp);
		delta_shift += tmp;
		// reset_needed = 1;
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
		// circle_3_reset_ctdown = 1;
	}
	// 15000000
	// 30000000
	if (circle_3_reset_ctdown <= 0)
	{
		circle_3_reset_ctdown = 30;
	}

	int tmp = (1 + (rand() % 5));
	if (music_data->current_quarter_value < 15000000 && circle_3_reset_ctdown % (tmp + 1 + (rand() % 4)) < tmp)
	{
		euclidean_datas[3].mess_chance = 70;
	}
	else
	{
		euclidean_datas[3].mess_chance = 100;
	}

	circle_3_reset_ctdown--;

	euclidean_datas[0].min_chord_size = (sensors_data->vin_current % 4) + 1; //(uint8_t)map_number((uint32_t)sensors_data->temperature_3, 0, FIX_4096 - 400, 1, 7);	//temperature_3
	euclidean_datas[0].max_chord_size = (sensors_data->vin_current % 4) + 1; // temperature_3


	static uint16_t mode_requested = A2;
	static uint16_t type_mode_requested = M_MODE_MAJOR;

	if (sensors_data->carousel_state < 20)
	{
		mode_requested = A2;
		// if (euclidean_datas[0].mode_beg_note != A2)
		// 	reset_needed = 1;
		// euclidean_datas[0].mode_beg_note = A2;
		// euclidean_datas[1].mode_beg_note = A2;
		// euclidean_datas[2].mode_beg_note = A2;
	}
	else if (sensors_data->carousel_state < 40)
	{
		mode_requested = B2;

		// if (euclidean_datas[0].mode_beg_note != B2)
		// 	reset_needed = 1;
		// euclidean_datas[0].mode_beg_note = B2;
		// euclidean_datas[1].mode_beg_note = B2;
		// euclidean_datas[2].mode_beg_note = B2;
	}
	else if (sensors_data->carousel_state < 60)
	{
		mode_requested = C2;

		// if (euclidean_datas[0].mode_beg_note != C2)
		// 	reset_needed = 1;
		// euclidean_datas[0].mode_beg_note = C2;
		// euclidean_datas[1].mode_beg_note = C2;
		// euclidean_datas[2].mode_beg_note = C2;
	}
	else if (sensors_data->carousel_state < 80)
	{
		mode_requested = D2;

		// if (euclidean_datas[0].mode_beg_note != D2)
		// 	reset_needed = 1;
		// euclidean_datas[0].mode_beg_note = D2;
		// euclidean_datas[1].mode_beg_note = D2;
		// euclidean_datas[2].mode_beg_note = D2;
	}
	else if (sensors_data->carousel_state < 100)
	{
		mode_requested = E2;

		// if (euclidean_datas[0].mode_beg_note != E2)
		// 	reset_needed = 1;
		// euclidean_datas[0].mode_beg_note = E2;
		// euclidean_datas[1].mode_beg_note = E2;
		// euclidean_datas[2].mode_beg_note = E2;
	}
	else if (sensors_data->carousel_state < 110)
	{
		mode_requested = F2;

		// if (euclidean_datas[0].mode_beg_note != F2)
		// 	reset_needed = 1;
		// euclidean_datas[0].mode_beg_note = F2;
		// euclidean_datas[1].mode_beg_note = F2;
		// euclidean_datas[2].mode_beg_note = F2;
	}
	else
	{
		mode_requested = G2;

		// if (euclidean_datas[0].mode_beg_note != G2)
		// 	reset_needed = 1;
		// euclidean_datas[0].mode_beg_note = G2;
		// euclidean_datas[1].mode_beg_note = G2;
		// euclidean_datas[2].mode_beg_note = G2;
	}

	type_mode_requested = sensors_data->lid_state / 5;

	if (sensors_data->photodiode_1 < 500)
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

	// write_mode(&curses_env, g_midi_mode[euclidean_datas[0].mode].name , g_notes_definitions[1].name);
	// write_mode(&curses_env, g_midi_mode[euclidean_datas[0].mode].name , "bonjour");

	// Change euclidean datas with sensors values
	// \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/ \/
	if ((uint32_t)sensors_data->photodiode_2 > 1024)
	{
		euclidean_datas[1].mess_chance = 40; //(uint32_t)map_number((uint32_t)sensors_data->photodiode_2, 0, 4096, 60, 20);

		if (euclidean_datas[1].notes_per_cycle != (uint8_t)map_number(sensors_data->organ_1, 0, 1024, 2, 5))
		{
			reset_needed = 1;
		}
		euclidean_datas[1].notes_per_cycle = (uint8_t)map_number(sensors_data->organ_1, 0, 1024, 2, 5);
		euclidean_datas[1].step_gap =
			euclidean_datas[1].euclidean_steps_length / euclidean_datas[1].notes_per_cycle;

		if (euclidean_datas[0].notes_per_cycle != (uint8_t)map_number(sensors_data->organ_1, 0, 1024, 4, 9))
		{
			reset_needed = 1;
		}
		euclidean_datas[0].notes_per_cycle = (uint8_t)map_number(sensors_data->organ_1, 0, 1024, 4, 9);
		euclidean_datas[0].step_gap =
			euclidean_datas[0].euclidean_steps_length / euclidean_datas[0].notes_per_cycle;

		euclidean_datas[0].min_steps_duration = (uint8_t)map_number(sensors_data->organ_1, 0, 1024, 10, 2);
		euclidean_datas[1].min_steps_duration = (uint8_t)map_number(sensors_data->organ_1, 0, 1024, 10, 2);
		euclidean_datas[2].min_steps_duration = (uint8_t)map_number(sensors_data->organ_1, 0, 1024, 10, 2);

		euclidean_datas[0].max_steps_duration = (uint8_t)map_number(sensors_data->organ_1, 0, 1024, 14, 3);
		euclidean_datas[1].max_steps_duration = (uint8_t)map_number(sensors_data->organ_1, 0, 1024, 14, 3);
		euclidean_datas[2].max_steps_duration = (uint8_t)map_number(sensors_data->organ_1, 0, 1024, 14, 3);
		// (uint32_t)map_number((uint32_t)sensors_data->photodiode_1, 0, 4096, 60, 0);
	}
	else
	{
		euclidean_datas[1].mess_chance = 100;
	}

	if ((uint32_t)sensors_data->photodiode_2 > 2048)
	{
		euclidean_datas[2].mess_chance = 40; //(uint32_t)map_number((uint32_t)sensors_data->photodiode_2, 2048, 4096, 80, 20);
	}
	else
	{
		euclidean_datas[2].mess_chance = 100;
	}
	// /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\ /\.

	// Each 30-60 seconds, request to get new notes in euclidean circles
	if (LOG_ALL)
	{

		printf("Time : %ld", time(NULL));
		printf("Last Time : %d", last_time);
	}
	// if (time(NULL) - last_time > 30 + rand() % 30)
	// {
	// 	reset_needed = 1;
	// 	last_time = time(NULL);
	// 	printf("\n\n\n\n\n! RESETING !\n\n\n\n\n\n");
	// }

	if (measure_count_1 > 32)
	{
		get_new_euclidean_chords(&euclidean_datas[0]);
		measure_count_1 = 0;
		// printf("\n\n\n\n\n! RESETING 0 !\n\n\n\n\n\n");

	}

	if (measure_count_2 > 55)
	{
		get_new_euclidean_chords(&euclidean_datas[1]);
		measure_count_2 = 0;
		// printf("\n\n\n\n\n! RESETING 1 !\n\n\n\n\n\n");
	}

	if (measure_count_3 > 83)
	{
		get_new_euclidean_chords(&euclidean_datas[2]);
		measure_count_3 = 0;
		// printf("\n\n\n\n\n! RESETING 2 !\n\n\n\n\n\n");

	}

	// Initialize notes or if requested to get new note, pick new random notes from allowed ones
	if (reset_needed)
	{
		for (uint8_t current_euclidean_data = 0; current_euclidean_data < EUCLIDEAN_DATAS_LENGTH; current_euclidean_data++)
		{
			get_new_euclidean_chords(&euclidean_datas[current_euclidean_data]);
		}

		// printf("\n\n\n\n\n! RESETING !\n\n\n\n\n\n");

		// delta_shift = 0;
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

	// show_euclidean_circle(&curses_env, 0, &euclidean_datas[0]);
	// show_euclidean_circle(&curses_env, 1, &euclidean_datas[1]);
	// show_euclidean_circle(&curses_env, 2, &euclidean_datas[2]);
	// show_euclidean_circle(&curses_env, 3, &euclidean_datas[3]);



	uint16_t div_counter = 0;
	uint16_t div_goal = 512; // Whole division (quarter * 4)
	uint16_t looseness = 42; //40; // Humanization in divisions delta, cannot be superior of (divgoal / 3 - divgoal / 4)

	// Write a midi measure (iterate on each quarter)
	for (uint8_t current_quarter = 0; current_quarter < 4; current_quarter++)
	{
		uint16_t current_div_duration;


		// For each euclidean circle, create corresponding chord
		for (uint8_t current_euclidean_data = 0; current_euclidean_data < EUCLIDEAN_DATAS_LENGTH; current_euclidean_data++)
		{
			// if (current_euclidean_data == 3)
			// {

			// char printf_hack[64];
			// snprintf(printf_hack, 64,"BEG WRITING CHORD %d\n", current_euclidean_data);

			// write_value(&curses_env, printf_hack);
			write_euclidean_step(music_data, &euclidean_datas[current_euclidean_data]);
			// snprintf(printf_hack, 64,"END WRITING CHORD %d\n", current_euclidean_data);
			// write_value(&curses_env, printf_hack);
			// }
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
  * @brief Function to write an 4 stroke measure
  * @param [music_data] Midi struct
  * @param [sensors_data] Struct that contain current sensors datas
*/
void midi_write_measure(t_music_data *music_data, t_sensors *sensors_data)
{
	uint8_t gamme[7];
	int8_t octave_offset = 0;

	octave_offset = (int8_t)map_number(sensors_data->carousel_state, 0, 160, -3, 3) * 12;
	music_data->quarter_value_goal = (uint32_t)map_number((uint32_t)sensors_data->photodiode_1, 0, 4096, 1000000, 100000);
	update_quarter_value(music_data);
	// printf("Quarter current value :", );
	t_note first_note = {.active = 1,
						 .beg_eighth = 1,
						 .eighth_duration = (rand() % 6) + 1,
						 .velocity = (rand() % 64) + 64,
						 .channel = 1,
						 .note = gamme[(uint8_t)sensors_data->temperature_1 % 7] + octave_offset};
	t_note second_note = {.active = sensors_data->organ_2 > 2 ? 1 : 0,
						  .beg_eighth = rand() % 6,
						  .eighth_duration = (rand() % 2) + 1,
						  .velocity = (rand() % 64) + 64,
						  .channel = 1,
						  .note = gamme[rand() % 7] + octave_offset};
	t_note third_note = {.active = sensors_data->organ_3 > 2.3 ? 1 : 0,
						 .beg_eighth = rand() % 6,
						 .eighth_duration = (rand() % 2) + 1,
						 .velocity = (rand() % 64) + 64,
						 .channel = 1,
						 .note = gamme[rand() % 7] + octave_offset};
	t_note fourth_note = {.active = sensors_data->organ_4 > 2.6 ? 1 : 0,
						  .beg_eighth = rand() % 6,
						  .eighth_duration = (rand() % 2) + 1,
						  .velocity = (rand() % 64) + 64,
						  .channel = 1,
						  .note = gamme[rand() % 7] + octave_offset};

	t_note sixth_note = {.active = sensors_data->organ_4 > 3 ? 1 : 0,
						 .beg_eighth = rand() % 6,
						 .eighth_duration = (rand() % 2) + 1,
						 .velocity = (rand() % 64) + 64,
						 .channel = 1,
						 .note = gamme[rand() % 7] + octave_offset};

	t_note seventh_note = {.active = sensors_data->organ_4 > 3.3 ? 1 : 0,
						   .beg_eighth = rand() % 6,
						   .eighth_duration = (rand() % 2) + 1,
						   .velocity = (rand() % 64) + 64,
						   .channel = 1,
						   .note = gamme[rand() % 7] + octave_offset};

	printf("Organ value : %d; %d; %d; %d; %d\n",
		   sensors_data->organ_2, sensors_data->organ_3, sensors_data->organ_4,
		   sensors_data->organ_5, sensors_data->organ_6);

	//    ====================================================
	//  ||                                                    ||
	//  ||                      T = 1/8                       ||
	//  ||                                                    ||
	// midi_write_measure_note(music_data, ON, 1, gamme[0], 64);

	midi_write_measure_heighth_updater(music_data, first_note, 1);
	midi_write_measure_heighth_updater(music_data, second_note, 1);
	midi_write_measure_heighth_updater(music_data, third_note, 1);
	midi_write_measure_heighth_updater(music_data, fourth_note, 1);
	midi_write_measure_heighth_updater(music_data, sixth_note, 1);
	midi_write_measure_heighth_updater(music_data, seventh_note, 1);

	//  ||                                                    ||
	//  ||                      T = 1/8                       ||
	//  ||                                                    ||
	//    ====================================================

	midi_delay_heighth(music_data);

	//    ====================================================
	//  ||                                                    ||
	//  ||                      T = 2/8                       ||
	//  ||                                                    ||

	midi_write_measure_heighth_updater(music_data, first_note, 2);
	midi_write_measure_heighth_updater(music_data, second_note, 2);
	midi_write_measure_heighth_updater(music_data, third_note, 2);
	midi_write_measure_heighth_updater(music_data, fourth_note, 2);
	midi_write_measure_heighth_updater(music_data, sixth_note, 2);
	midi_write_measure_heighth_updater(music_data, seventh_note, 2);

	// midi_write_measure_note(music_data, OFF, 1, gamme[0], 0);

	// midi_write_measure_note(music_data, ON, 1,
	// 						gamme[(uint32_t)sensors_data->photodiode_1 % 7], 64);
	//  ||                                                    ||
	//  ||                      T = 2/8                       ||
	//  ||                                                    ||
	//    ====================================================

	midi_delay_heighth(music_data);

	//    ====================================================
	//  ||                                                    ||
	//  ||                      T = 3/8                       ||
	//  ||                                                    ||
	midi_write_measure_heighth_updater(music_data, first_note, 3);
	midi_write_measure_heighth_updater(music_data, second_note, 3);
	midi_write_measure_heighth_updater(music_data, third_note, 3);
	midi_write_measure_heighth_updater(music_data, fourth_note, 3);
	midi_write_measure_heighth_updater(music_data, sixth_note, 3);
	midi_write_measure_heighth_updater(music_data, seventh_note, 3);

	// midi_write_measure_note(music_data, OFF, 1,
	// 						gamme[(uint32_t)sensors_data->photodiode_1 % 7], 0);

	//  ||                                                    ||
	//  ||                      T = 3/8                       ||
	//  ||                                                    ||
	//    ====================================================

	midi_delay_heighth(music_data);

	//    ====================================================
	//  ||                                                    ||
	//  ||                      T = 4/8                       ||
	//  ||                                                    ||

	midi_write_measure_heighth_updater(music_data, first_note, 4);
	midi_write_measure_heighth_updater(music_data, second_note, 4);
	midi_write_measure_heighth_updater(music_data, third_note, 4);
	midi_write_measure_heighth_updater(music_data, fourth_note, 4);
	midi_write_measure_heighth_updater(music_data, sixth_note, 4);
	midi_write_measure_heighth_updater(music_data, seventh_note, 4);

	//  ||                                                    ||
	//  ||                      T = 4/8                       ||
	//  ||                                                    ||
	//    ====================================================

	midi_delay_heighth(music_data);

	//    ====================================================
	//  ||                                                    ||
	//  ||                      T = 5/8                       ||
	//  ||                                                    ||

	midi_write_measure_heighth_updater(music_data, first_note, 5);
	midi_write_measure_heighth_updater(music_data, second_note, 5);
	midi_write_measure_heighth_updater(music_data, third_note, 5);
	midi_write_measure_heighth_updater(music_data, fourth_note, 5);
	midi_write_measure_heighth_updater(music_data, sixth_note, 5);
	midi_write_measure_heighth_updater(music_data, seventh_note, 5);

	//  ||                                                    ||
	//  ||                      T = 5/8                       ||
	//  ||                                                    ||
	//    ====================================================

	midi_delay_heighth(music_data);

	//    ====================================================
	//  ||                                                    ||
	//  ||                      T = 6/8                       ||
	//  ||                                                    ||

	midi_write_measure_heighth_updater(music_data, first_note, 6);
	midi_write_measure_heighth_updater(music_data, second_note, 6);
	midi_write_measure_heighth_updater(music_data, third_note, 6);
	midi_write_measure_heighth_updater(music_data, fourth_note, 6);
	midi_write_measure_heighth_updater(music_data, sixth_note, 6);
	midi_write_measure_heighth_updater(music_data, seventh_note, 6);

	//  ||                                                    ||
	//  ||                      T = 6/8                       ||
	//  ||                                                    ||
	//    ====================================================

	midi_delay_heighth(music_data);

	//    ====================================================
	//  ||                                                    ||
	//  ||                      T = 7/8                       ||
	//  ||                                                    ||

	midi_write_measure_heighth_updater(music_data, first_note, 7);
	midi_write_measure_heighth_updater(music_data, second_note, 7);
	midi_write_measure_heighth_updater(music_data, third_note, 7);
	midi_write_measure_heighth_updater(music_data, fourth_note, 7);
	midi_write_measure_heighth_updater(music_data, sixth_note, 7);
	midi_write_measure_heighth_updater(music_data, seventh_note, 7);

	//  ||                                                    ||
	//  ||                      T = 7/8                       ||
	//  ||                                                    ||
	//    ====================================================

	midi_delay_heighth(music_data);

	//    ====================================================
	//  ||                                                    ||
	//  ||                      T = 8/8                       ||
	//  ||                                                    ||
	midi_write_measure_heighth_updater(music_data, first_note, 8);
	midi_write_measure_heighth_updater(music_data, second_note, 8);
	midi_write_measure_heighth_updater(music_data, third_note, 8);
	midi_write_measure_heighth_updater(music_data, fourth_note, 8);
	midi_write_measure_heighth_updater(music_data, sixth_note, 8);
	midi_write_measure_heighth_updater(music_data, seventh_note, 8);

	//  ||                                                    ||
	//  ||                      T = 8/8                       ||
	//  ||                                                    ||
	//    ====================================================

	midi_delay_heighth(music_data);

	//    ====================================================
	//  ||                                                    ||
	//  ||                      T = END                       ||
	//  ||                                                    ||

	midi_write_measure_heighth_updater(music_data, first_note, 9);
	midi_write_measure_heighth_updater(music_data, second_note, 9);
	midi_write_measure_heighth_updater(music_data, third_note, 9);
	midi_write_measure_heighth_updater(music_data, fourth_note, 9);
	midi_write_measure_heighth_updater(music_data, sixth_note, 9);
	midi_write_measure_heighth_updater(music_data, seventh_note, 9);

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

/**
  * @brief Print Time from seconds to DDhMMmSSs(Time)
  * @param [beg] String to print before the time
  * @param [end] String to print after the time
*/
void print_time(char *beg, uint32_t time, char *end)
{
	printf("%s%02dh%02dm%02ds(%d)%s", beg, time / 60 / 60, time / 60 % 60, time % 60, time, end);
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
		printf("%scurrent data : %d\n", "\033[1;35m", current_data);
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
	uint32_t nu_of_measures = 0; // Usefull?

	t_sensors *sensors_data;
	t_sensors *current_sensors;

	current_sensors = (t_sensors *)malloc(sizeof(t_sensors));
	sensors_data = current_sensors;

	// printf("main addr : %p\n", sensors_data);
	//TODO : fix le i et le r genre WTF je comprends pas
	for (i = 3; i < r; i++)
	{

		printf("TEST 0123 : %d - %d - %d - %d\n", t[i].type, t[i].size, i, r);
		if (t[i].type == JSMN_OBJECT && t[i].size == (32 + 1))
		{
			// printf("\n");
			// printf("addr :%p\n", current_sensors);
			//obj_size = t[i].size;
			i++;
			if (JSON_cmp(file_content, &t[i], "Time") == 0)
			{
				char tmp[t[i + 1].end - t[i + 1].start + 1];
				strncpy(tmp,file_content + t[i + 1].start, t[i + 1].end - t[i + 1].start);
				tmp[t[i + 1].end - t[i + 1].start] = '\0';

				printf("- Time: %.*s\n", t[i + 1].end - t[i + 1].start,
				   file_content + t[i + 1].start);
				printf("tmp value omg %s\n", tmp);

				date_time_to_date_and_time(tmp,
										   &current_sensors->date, &current_sensors->time);
				i += 2;
			}
			if (JSON_cmp(file_content, &t[i], "Photodiode_1") == 0)
			{
				current_sensors->photodiode_1 = atoi(file_content + t[i + 1].start);
				i += 2;
			}
			if (JSON_cmp(file_content, &t[i], "Photodiode_2") == 0)
			{
				current_sensors->photodiode_2 = atoi(file_content + t[i + 1].start);
				i += 2;
			}
			if (JSON_cmp(file_content, &t[i], "Photodiode_3") == 0)
			{
				current_sensors->photodiode_3 = atoi(file_content + t[i + 1].start);
				i += 2;
			}
			if (JSON_cmp(file_content, &t[i], "Photodiode_4") == 0)
			{
				current_sensors->photodiode_4 = atoi(file_content + t[i + 1].start);
				i += 2;
			}
			if (JSON_cmp(file_content, &t[i], "Photodiode_5") == 0)
			{
				current_sensors->photodiode_5 = atoi(file_content + t[i + 1].start);
				i += 2;
			}
			if (JSON_cmp(file_content, &t[i], "Photodiode_6") == 0)
			{
				current_sensors->photodiode_6 = atoi(file_content + t[i + 1].start);
				i += 2;
			}

			if (JSON_cmp(file_content, &t[i], "Temperature_1") == 0)
			{
				current_sensors->temperature_1 = atoi(file_content + t[i + 1].start);
				i += 2;
			}
			if (JSON_cmp(file_content, &t[i], "Temperature_2") == 0)
			{
				current_sensors->temperature_2 = atoi(file_content + t[i + 1].start);
				i += 2;
			}
			if (JSON_cmp(file_content, &t[i], "Temperature_3") == 0)
			{
				current_sensors->temperature_3 = atoi(file_content + t[i + 1].start);
				i += 2;
			}
			if (JSON_cmp(file_content, &t[i], "Temperature_4") == 0)
			{
				current_sensors->temperature_4 = atoi(file_content + t[i + 1].start);
				i += 2;
			}
			if (JSON_cmp(file_content, &t[i], "Temperature_5") == 0)
			{
				current_sensors->temperature_5 = atoi(file_content + t[i + 1].start);
				i += 2;
			}
			if (JSON_cmp(file_content, &t[i], "Temperature_6") == 0)
			{
				current_sensors->temperature_6 = atoi(file_content + t[i + 1].start);
				i += 2;
			}
			if (JSON_cmp(file_content, &t[i], "Temperature_7") == 0)
			{
				current_sensors->temperature_7 = atoi(file_content + t[i + 1].start);
				i += 2;
			}
			if (JSON_cmp(file_content, &t[i], "Temperature_8") == 0)
			{
				current_sensors->temperature_8 = atoi(file_content + t[i + 1].start);
				i += 2;
			}
			if (JSON_cmp(file_content, &t[i], "Temperature_9") == 0)
			{
				current_sensors->temperature_9 = atoi(file_content + t[i + 1].start);
				i += 2;
			}
			if (JSON_cmp(file_content, &t[i], "Temperature_10") == 0)
			{
				current_sensors->temperature_10 = atoi(file_content + t[i + 1].start);
				i += 2;
			}

			if (JSON_cmp(file_content, &t[i], "Microphone") == 0)
			{
				current_sensors->microphone = atoi(file_content + t[i + 1].start);
				i += 2;
			}

			if (JSON_cmp(file_content, &t[i], "Spectro_current") == 0)
			{
				current_sensors->spectro_current = atoi(file_content + t[i + 1].start);
				i += 2;
			}

			if (JSON_cmp(file_content, &t[i], "Organ_current") == 0)
			{
				current_sensors->organ_current = atoi(file_content + t[i + 1].start);
				i += 2;
			}
			if (JSON_cmp(file_content, &t[i], "Vin_current") == 0)
			{
				current_sensors->vin_current = atoi(file_content + t[i + 1].start);
				i += 2;
			}
			if (JSON_cmp(file_content, &t[i], "Q7_current") == 0)
			{
				current_sensors->q7_current = atoi(file_content + t[i + 1].start);
				i += 2;
			}
			if (JSON_cmp(file_content, &t[i], "5v_current") == 0)
			{
				current_sensors->t5v_current = atoi(file_content + t[i + 1].start);
				i += 2;
			}
			if (JSON_cmp(file_content, &t[i], "3.3v_current") == 0)
			{
				current_sensors->t3_3v_current = atoi(file_content + t[i + 1].start);
				i += 2;
			}
			if (JSON_cmp(file_content, &t[i], "Motor_current") == 0)
			{
				current_sensors->motor_current = atoi(file_content + t[i + 1].start);
				i += 2;
			}

			if (JSON_cmp(file_content, &t[i], "Carousel_state") == 0)
			{
				current_sensors->carousel_state = atoi(file_content + t[i + 1].start);
				i += 2;
			}
			if (JSON_cmp(file_content, &t[i], "Lid_state") == 0)
			{
				current_sensors->lid_state = atoi(file_content + t[i + 1].start);
				i += 2;
			}

			// if (JSON_cmp(file_content, &t[i], "Spectrum") == 0)
			// {
			// 	current_sensors->spectrum = atoi(file_content + t[i + 1].start);
			// 	i += 2;
			// }

			if (JSON_cmp(file_content, &t[i], "Organ_1") == 0)
			{
				current_sensors->organ_1 = atoi(file_content + t[i + 1].start);
				i += 2;
			}
			if (JSON_cmp(file_content, &t[i], "Organ_2") == 0)
			{
				current_sensors->organ_2 = atoi(file_content + t[i + 1].start);
				i += 2;
			}
			if (JSON_cmp(file_content, &t[i], "Organ_3") == 0)
			{
				current_sensors->organ_3 = atoi(file_content + t[i + 1].start);
				i += 2;
			}
			if (JSON_cmp(file_content, &t[i], "Organ_4") == 0)
			{
				current_sensors->organ_4 = atoi(file_content + t[i + 1].start);
				i += 2;
			}
			if (JSON_cmp(file_content, &t[i], "Organ_5") == 0)
			{
				current_sensors->organ_5 = atoi(file_content + t[i + 1].start);
				i += 2;
			}
			if (JSON_cmp(file_content, &t[i], "Organ_6") == 0)
			{
				current_sensors->organ_6 = atoi(file_content + t[i + 1].start);
				i += 1;
			}

			if (i + (33 * 2 + 2) < r)
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
		sprintf(fileName, "%04d_%02d_%02d__%02d_%02d_%02d.mid",
				sensors_data->date / 10000, sensors_data->date / 100 % 100, sensors_data->date % 100,
				sensors_data->time / 60 / 60, sensors_data->time / 60 % 60, sensors_data->time % 60);
		sprintf(fileNameRedundancy, "%04d_%02d_%02d__%02d_%02d_%02d_.mid",
				sensors_data->date / 10000, sensors_data->date / 100 % 100, sensors_data->date % 100,
				sensors_data->time / 60 / 60, sensors_data->time / 60 % 60, sensors_data->time % 60);
	}
	else //Ne passe normalement jamais ICI
	{
		now = time(NULL);
		tm_now = *localtime(&now);
		strftime(fileName, sizeof(fileName), "%Y_%m_%d__%H_%M_%S.mid", &tm_now);
		strftime(fileNameRedundancy, sizeof(fileNameRedundancy), "%Y_%m_%d__%H_%M_%S_.mid", &tm_now);
	}

	// printf("File Name : %s\n", fileName);
	sprintf(filePath, "%s/%s", output_directory, fileName);
	sprintf(filePathRedundancy, "%s/%s", output_directory_redundancy, fileNameRedundancy);
	// printf("File Path : %s\n", filePath);
	make_path(filePath, 0755);
	make_path(filePathRedundancy, 0755);
	printf("file 1 : \"%s\", file 2 : \"%s\"\n", filePath, filePathRedundancy);
	midi_setup_file(filePath, filePathRedundancy, music_data);
}

/**
  * @brief Terminate properly midi notes (for exiting file/program)
  * @param [music_data] Midi struct of midi file
*/
void terminate_notes(t_music_data *music_data)
{
	// remove_chord(music_data, playing_notes_duration, playing_notes, playing_notes_length);
	printf("\033[1;96mTerminate func\n");
	for (uint8_t i = 0; i < playing_notes_length; i++)
	{
		printf("Playing notes[%d] : N = %d, D = %d\n", i, playing_notes[i], playing_notes_duration[i]);
	}
	printf("\033[1;37m\n");

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
void init_music_data(t_music_data *music_data, uint32_t partition_duration, \
					 uint32_t quarter_value, uint32_t quarter_value_goal, \
					 float tempo_acceleration)
{
	music_data->partition_duration = 60000000 * partition_duration;//10 minutes
	music_data->measure_value = 500000 * 4;//useless
	music_data->measures_writed = 0;
	music_data->delta_time = 0;
	music_data->quarter_value_step = 100000;
	//music_data->quarter_value_goal = quarter_value;
	music_data->quarter_value_goal = quarter_value_goal;
	//                          100000
	music_data->quarter_value = 500000;//define metadata 500000=120bpm
	music_data->current_quarter_value = quarter_value;
	music_data->quarter_value_step_updating = tempo_acceleration;//Acceleration per measure in percentage (1.0=100%, 0.05=5%)

}

int main(int argc, char **argv)
{
	//durée d'une partition 40 000 000us
	t_music_data music_data = {0};
	init_music_data(&music_data, 10, 1000000, 250000, 0.03);

	char *filesDirectory = "data_files";
	char *outputDirectory = "midi_files";
	char *outputDirectoryRedundancy = "midi_files";

	if (argc == 3)
	{
		filesDirectory = argv[1];
		outputDirectory = argv[2];
		outputDirectoryRedundancy = argv[2];
	}
	else if (argc == 4)
	{
		filesDirectory = argv[1];
		outputDirectory = argv[2];
		outputDirectoryRedundancy = argv[3];
	}

	signal(SIGTERM, (void (*)(int))terminate_session);

	char *currentDataFileName;
	t_sensors *sensorsData;
	sensorsData = NULL;
	currentDataFileName = (char *)malloc(sizeof(char) * 200);

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
				else // Normalement ne sert plus a R
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
		if (!music_data.midi_file && sensorsData) // && !music_data.midi_file_redundancy?
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
				music_data.data_time = sensorsData->time; // * 1000000;
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
				// midi_write_measure(&music_data, sensorsData);
				midi_write_multiple_euclidean(&music_data, sensorsData);

				// midi_write_euclidean_measure(&music_data, sensorsData);
				music_data.measures_writed++; //
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

			// printf("datatime : %d, sensor time : %d\n",  music_data.data_time,  sensorsData->time);
			// while (sensorsData->next && music_data.data_time > sensorsData->next->time)
			while (sensorsData && music_data.data_time >= sensorsData->time) // >= ???
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
	if (music_data.midi_file) //&& music_data.midi_file_redundancy?
	{
		printf("Midi write end 2\n");
		terminate_notes(&music_data);

		midi_write_end(&music_data);
	}
	return (0);
}
