#ifdef MIDI_MODES_H
# define MIDI_MODES_H
///https://fr.wikipedia.org/wiki/Mode_(musique)
enum	e_midi_modes{
	M_MODE_IONIEN,
	M_MODE_DORIEN,
	M_MODE_PHRYGIEN,
	M_MODE_LYDIEN,
	M_MODE_MIXOLYDIEN,
	M_MODE_EOLIEN,
	M_MODE_LOCRIEN,
}

typedef struct	s_midi_modes
{
	uint8_t		mode_name;
	uint8_t		starting_note;
	uint8_t*	mode_sequence;
}				t_midi_modes;

static const t_midi_modes 	_midi_mode[] = {
	(t_midi_modes){}
}


#endif