#ifndef MIDI_EUCLIDEAN_H
# define MIDI_EUCLIDEAN_H
# include <stdint.h>

typedef struct	s_euclidean
{
	uint8_t		euclidean_step_length;
	// size of euclideant_step_length
	int16_t		*euclidean_steps;
	uint8_t		octave_size;
	uint8_t		chord_list_length;
	uint8_t		*chord_list;
	uint8_t		mode;
	uint8_t		mode_beg_note;
	uint8_t		notes_per_cycle;
	uint8_t		step_gap;//??
	uint8_t		mess_chance;
	uint8_t		min_chord_size;
	uint8_t		max_chord_size;
}				t_euclidean


#endif
