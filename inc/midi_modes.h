#ifndef MIDI_MODES_H
#define MIDI_MODES_H
#include <stdint.h>
#include "./midi_notes.h"
///https://fr.wikipedia.org/wiki/Mode_(musique)
enum e_midi_modes
{
	// 7 note scales
	M_MODE_MAJOR,
	M_MODE_IONIAN,
	M_MODE_DORIAN,
	M_MODE_PHRYGIAN,
	M_MODE_LYDIAN,
	M_MODE_MIXOLYDIAN,
	M_MODE_AEOLIAN,
	M_MODE_MINOR,
	M_MODE_LOCRIAN,

	M_MODE_HARMONIC_MINOR,
	M_MODE_HARMONIC_MAJOR,

	M_MODE_MELODIC_MINOR,
	M_MODE_MELODIC_MINOR_DESC,
	M_MODE_MELODIC_MAJOR,

	M_MODE_BARTOK,
	M_MODE_HINDU,

	// raga modes
	M_MODE_TODI,
	M_MODE_PURVI,
	M_MODE_MARVA,
	M_MODE_BHAIRAVI,
	M_MODE_AHIRBHAIRAV,

	M_MODE_SUPER_LOCRIAN,
	M_MODE_ROMANIAN_MINOR,
	M_MODE_HUNGARIAN_MINOR,
	M_MODE_NEAPOLITAN_MINOR,
	M_MODE_ENIGMATIC,
	M_MODE_SPANISH,
	M_MODE_GYPSY,

	// modes of whole tones with added note
	M_MODE_LEADING_WHOLE_TONE,
	M_MODE_LYDIAN_MINOR,
	M_MODE_NEAPOLITAN_MAJOR,
	M_MODE_LOCRIAN_MAJOR,
	M_MODE_ALL,
};

typedef struct s_midi_modes
{
	uint8_t mode_name;
	uint8_t starting_note;	  //Note de depart du mode
	uint8_t mode_sequence[7]; //Interval en demi-tons// 12 au total
} t_midi_modes;

static const t_midi_modes g_midi_mode[] = {
	// 7 note scales
	(t_midi_modes){.mode_name = M_MODE_MAJOR, .starting_note = C4, .mode_sequence = {0, 2, 4, 5, 7, 9, 11}},
	(t_midi_modes){.mode_name = M_MODE_IONIAN, .starting_note = C4, .mode_sequence = {0, 2, 4, 5, 7, 9, 11}},
	(t_midi_modes){.mode_name = M_MODE_DORIAN, .starting_note = C4, .mode_sequence = {0, 2, 4, 5, 7, 9, 10}},
	(t_midi_modes){.mode_name = M_MODE_PHRYGIAN, .starting_note = C4, .mode_sequence = {0, 1, 3, 5, 7, 8, 10}},
	(t_midi_modes){.mode_name = M_MODE_LYDIAN, .starting_note = C4, .mode_sequence = {0, 2, 4, 6, 7, 9, 11}},
	(t_midi_modes){.mode_name = M_MODE_MIXOLYDIAN, .starting_note = C4, .mode_sequence = {0, 2, 4, 5, 7, 9, 10}},
	(t_midi_modes){.mode_name = M_MODE_AEOLIAN, .starting_note = C4, .mode_sequence = {0, 2, 3, 5, 7, 8, 10}},
	(t_midi_modes){.mode_name = M_MODE_MINOR, .starting_note = C4, .mode_sequence = {0, 2, 3, 5, 7, 8, 10}},
	(t_midi_modes){.mode_name = M_MODE_LOCRIAN, .starting_note = C4, .mode_sequence = {0, 1, 3, 5, 6, 8, 10}},

	(t_midi_modes){.mode_name = M_MODE_HARMONIC_MINOR, .starting_note = C4, .mode_sequence = {0, 2, 3, 5, 7, 8, 11}},
	(t_midi_modes){.mode_name = M_MODE_HARMONIC_MAJOR, .starting_note = C4, .mode_sequence = {0, 2, 4, 5, 7, 8, 11}},

	(t_midi_modes){.mode_name = M_MODE_MELODIC_MINOR, .starting_note = C4, .mode_sequence = {0, 2, 3, 5, 7, 9, 11}},
	(t_midi_modes){.mode_name = M_MODE_MELODIC_MINOR_DESC, .starting_note = C4, .mode_sequence = {0, 2, 3, 5, 7, 8, 10}},
	(t_midi_modes){.mode_name = M_MODE_MELODIC_MAJOR, .starting_note = C4, .mode_sequence = {0, 2, 4, 5, 7, 8, 10}},

	(t_midi_modes){.mode_name = M_MODE_BARTOK, .starting_note = C4, .mode_sequence = {0, 2, 4, 5, 7, 8, 10}},
	(t_midi_modes){.mode_name = M_MODE_HINDU, .starting_note = C4, .mode_sequence = {0, 2, 4, 5, 7, 8, 10}},

	// raga modes
	(t_midi_modes){.mode_name = M_MODE_TODI, .starting_note = C4, .mode_sequence = {0, 1, 3, 6, 7, 8, 11}},
	(t_midi_modes){.mode_name = M_MODE_PURVI, .starting_note = C4, .mode_sequence = {0, 1, 4, 6, 7, 8, 11}},
	(t_midi_modes){.mode_name = M_MODE_MARVA, .starting_note = C4, .mode_sequence = {0, 1, 4, 6, 7, 9, 11}},
	(t_midi_modes){.mode_name = M_MODE_BHAIRAVI, .starting_note = C4, .mode_sequence = {0, 1, 4, 5, 7, 8, 11}},
	(t_midi_modes){.mode_name = M_MODE_AHIRBHAIRAV, .starting_note = C4, .mode_sequence = {0, 1, 4, 5, 7, 9, 10}},

	(t_midi_modes){.mode_name = M_MODE_SUPER_LOCRIAN, .starting_note = C4, .mode_sequence = {0, 1, 3, 4, 6, 8, 10}},
	(t_midi_modes){.mode_name = M_MODE_ROMANIAN_MINOR, .starting_note = C4, .mode_sequence = {0, 2, 3, 6, 7, 9, 10}},
	(t_midi_modes){.mode_name = M_MODE_HUNGARIAN_MINOR, .starting_note = C4, .mode_sequence = {0, 2, 3, 6, 7, 8, 11}},
	(t_midi_modes){.mode_name = M_MODE_NEAPOLITAN_MINOR, .starting_note = C4, .mode_sequence = {0, 1, 3, 5, 7, 8, 11}},
	(t_midi_modes){.mode_name = M_MODE_ENIGMATIC, .starting_note = C4, .mode_sequence = {0, 1, 4, 6, 8, 10, 11}},
	(t_midi_modes){.mode_name = M_MODE_SPANISH, .starting_note = C4, .mode_sequence = {0, 1, 4, 5, 7, 8, 10}},
	(t_midi_modes){.mode_name = M_MODE_GYPSY, .starting_note = C4, .mode_sequence = {0, 2, 3, 6, 7, 8, 10}},

	// modes of whole tones with added note
	(t_midi_modes){.mode_name = M_MODE_LEADING_WHOLE_TONE, .starting_note = C4, .mode_sequence = {0, 2, 4, 6, 8, 10, 11}},
	(t_midi_modes){.mode_name = M_MODE_LYDIAN_MINOR, .starting_note = C4, .mode_sequence = {0, 2, 4, 6, 7, 8, 10}},
	(t_midi_modes){.mode_name = M_MODE_NEAPOLITAN_MAJOR, .starting_note = C4, .mode_sequence = {0, 1, 3, 5, 7, 9, 11}},
	(t_midi_modes){.mode_name = M_MODE_LOCRIAN_MAJOR, .starting_note = C4, .mode_sequence = {0, 2, 4, 5, 6, 8, 10}},
};

#endif