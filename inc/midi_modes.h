#ifndef MIDI_MODES_H
# define MIDI_MODES_H
# include <stdint.h>
# include "./midi_notes.h"
///https://fr.wikipedia.org/wiki/Mode_(musique)
enum	e_midi_modes{
	M_MODE_IONIEN,
	M_MODE_DORIEN,
	M_MODE_PHRYGIEN,
	M_MODE_LYDIEN,
	M_MODE_MIXOLYDIEN,
	M_MODE_EOLIEN,
	M_MODE_LOCRIEN,
	M_MODE_DORIEN_DIEZ4,
	M_MODE_ALL,
};

typedef struct	s_midi_modes
{
	uint8_t		mode_name;
	uint8_t		starting_note;//Note de depart du mode
	uint8_t		mode_sequence[7];//Interval en demi-tons// 12 au total
}				t_midi_modes;

static const t_midi_modes 	g_midi_mode[] = {
	(t_midi_modes){.mode_name = M_MODE_IONIEN, .starting_note = C4,
	.mode_sequence = {2, 2, 1, 2, 2, 2, 1}},
	(t_midi_modes){.mode_name = M_MODE_DORIEN, .starting_note = D4,
	.mode_sequence = {2, 1, 2, 2, 2, 1, 2}},
	(t_midi_modes){.mode_name = M_MODE_PHRYGIEN, .starting_note = E4,
	.mode_sequence = {1, 2, 2, 2, 1, 2, 2}},
	(t_midi_modes){.mode_name = M_MODE_LYDIEN, .starting_note = F4,
	.mode_sequence = {2, 2, 2, 1, 2, 2, 1}},
	(t_midi_modes){.mode_name = M_MODE_MIXOLYDIEN, .starting_note = G4,
	.mode_sequence = {2, 2, 1, 2, 2, 1, 2}},
	(t_midi_modes){.mode_name = M_MODE_EOLIEN, .starting_note = A4,
	.mode_sequence = {2, 1, 2, 2, 1, 2, 2}},
	(t_midi_modes){.mode_name = M_MODE_LOCRIEN, .starting_note = B4,
	.mode_sequence = {1, 2, 2, 1, 2, 2, 2}},

	(t_midi_modes){.mode_name = M_MODE_DORIEN_DIEZ4, .starting_note = D4,
	.mode_sequence = {2, 1, 3, 1, 2, 1, 2}},



};


#endif