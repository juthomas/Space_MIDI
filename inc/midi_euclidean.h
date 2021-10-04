#ifndef MIDI_EUCLIDEAN_H
# define MIDI_EUCLIDEAN_H
# include <stdint.h>

typedef struct	s_euclidean
{
	// Number of steps in euclidean circle
	uint8_t		euclidean_steps_length;
	// Tab containing the notes in euclidean circle
	// 0xXX00 = octave height
	// 0x00XX = note index in chord list
	int16_t		*euclidean_steps;
	// Number of octaves were the note can be played
	uint8_t		octaves_size;
	// Number of chord possibles (range of notes)
	uint8_t		chords_list_length;
	// Tab containing notes of modes
	uint8_t		*chords_list;
	// Current music mode e_midi_modes
	uint8_t		mode;
	// Offset of the first note in music mode
	uint8_t		mode_beg_note;
	// Number of note can be played in euclidean cycle
	uint8_t		notes_per_cycle;
	// Useless ???
	uint8_t		step_gap;//??
	// Number of chance to miss note (note not played)
	uint8_t		mess_chance;
	// Minimum of chord size (between single note and a lot)
	uint8_t		min_chord_size;
	// Maximum of chord size (between single note and a lot)
	uint8_t		max_chord_size;
	// Minumum of note velocity (between 0 and 127)
	uint8_t		min_velocity;
	// Maximum of note velocity (between 0 and 127)
	uint8_t		max_velocity;
	// Minimum of note steps duration (between 1 and a lot)
	uint8_t		min_steps_duration;
	// Maximum of note steps duration (between 1 and a lot)
	uint8_t		max_steps_duration;
	// Current step in euclidean circle (between 0 and euclidean_steps_length)
	uint8_t		current_step;
	// Var to check if euclidean data is initialized
	uint8_t		initialized;
}				t_euclidean;


#endif
