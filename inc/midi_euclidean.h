#ifndef MIDI_EUCLIDEAN_H
# define MIDI_EUCLIDEAN_H
# include <stdint.h>

typedef struct	s_euclidean
{
	uint8_t		euclidean_steps_length;
	// size of euclideant_step_length
	int16_t		*euclidean_steps;
	uint8_t		octaves_size;
	uint8_t		chords_list_length;
	uint8_t		*chords_list;
	uint8_t		mode;
	uint8_t		mode_beg_note;
	uint8_t		notes_per_cycle;
	uint8_t		step_gap;//??
	uint8_t		mess_chance;
	uint8_t		min_chord_size;
	uint8_t		max_chord_size;
	uint8_t		min_velocity;
	uint8_t		max_velocity;
	uint8_t		min_steps_duration;
	uint8_t		max_steps_duration;
	uint8_t		current_step;
	uint8_t		initialized;
}				t_euclidean;


#endif
