#include "../../inc/midi.h"
#include "../../inc/midi_notes.h"

/*
 * HOW TO BASICS :
 *
 * [x] = required
 * [o] = optional (melody part)
 * [...] part [...] = can be repeated
 * 
 * [x] FILE *file_file = fopen(filename, "wb") ;
 * [x] MIDI_write_file_header(midi_file, 1, 2, QUARTER) ;
 * [x] MIDI_write_metadata(midi_file, 500000); //120bpm
 *    [...]
 *    [x] uint64_t mark = MIDI_write_track_header(file);
 *    [o] MIDI_Instrument_Change(fichier, 0, 90);
 *    [o] MIDI_only_one_note_with_duration(file, 1, i, 64, QUARTER);
 *    [o] MIDI_Note(file, ON, 1, C3, 64);
 *    [o] MIDI_delta_time(file, QUARTER);
 *    [x] MIDI_write_end_of_track(file);
 *    [x] MIDI_write_track_lengh(file, mark);
 *    [...]
 * [x] fclose(midi_file);
*/

/**
  * @brief Write the Midi file header (1st function to call)
  * @param [file] Midi file pointer
  * @param [SMF] Midi file type (0-3)
  * @param [tracks] Number of midi tracks (0-128)
  * @param [nbdiv] Division number for quarter note (noire) (1-32767)
*/
void MIDI_write_file_header(FILE *file, unsigned char SMF, unsigned short tracks, unsigned short nbdiv)
{
	if ((SMF == 0) && (tracks > 1))
	{
		printf("ERROR : Only one track possible in SMF 0 ! \n");
		exit(EXIT_FAILURE);
	}
	// MThd
	unsigned char bytes[14] = {0x4d, 0x54, 0x68, 0x64, 0x00, 0x00, 0x00, 0x06};

	bytes[8]  = 0; //
	bytes[9]  = SMF; // Type de fichier midi (0, 1 ou 2)
	bytes[10] = tracks >> 8; // Nombre de pistes (j'usqu'a 0xFFFF)
	bytes[11] = tracks ;
	bytes[12] = nbdiv  >> 8; // 0x8000 pour SMPTE sinon nb de ticks pour une noire
	bytes[13] = nbdiv;     // Nombre de divisions de la noire
	fwrite(&bytes, 14, 1, file);
}

/**
  * @brief Write metadata Midi line
  * @param [file] Midi file pointer
  * @param [tempo] quarter note (noire) value in micro-seconds
  * (500000 = 0.5s = 120bpm)
*/
void MIDI_write_metadata(FILE *file, uint32_t tempo)
{
	uint32_t header_index;

	header_index = MIDI_write_track_header(file);
	MIDI_tempo(file, tempo);
	MIDI_write_end_of_track(file);
	MIDI_write_track_length(file, header_index);
}

/**
  * @brief Write the Midi track header
  * @param [file] Midi file pointer
  * @return Current position in file
*/
uint32_t MIDI_write_track_header(FILE *file)
{
	//MTrk
	unsigned char bytes[8] = {0x4d, 0x54, 0x72, 0x6b, 0x00, 0x00, 0x00, 0x00};

	fwrite(&bytes, 8, 1, file);
	return ftell(file);
}

/**
  * @brief Instrument change or instrument selection
  * @param [file] Midi file pointer
  * @param [channel] Selection of midi channel (0-16)
  * @param [instrument] Selection of midi instrument (1-128)
*/
void MIDI_Instrument_Change(FILE *fichier, unsigned char channel, unsigned char instrument)
{
	unsigned char bytes[2];

	MIDI_delta_time(fichier, 0);
	bytes[0] = 0xC0 + channel % 16;//16 canaux max
	bytes[1] = instrument % 128;//128 instruments max
	fwrite(&bytes, 2, 1, fichier);
}

/**
  * @brief One note pressed with duration
  * @param [file] Midi file pointer
  * @param [state] Logic state of note (ON/OFF)
  * @param [channel] Selection of midi channel (0-16)
  * @param [MIDI_note] Selection of midi note (1-127)
  * @param [velocity] Selection of velocity (power) (1-127)
*/
void MIDI_only_one_note_with_duration(FILE *file, unsigned char channel, unsigned char MIDI_note, unsigned char velocity, unsigned long duration)
{
	MIDI_delta_time(file, 0);
	MIDI_Note(file, ON, channel, MIDI_note, velocity);
	MIDI_delta_time(file, duration);
	MIDI_Note(file, OFF, channel, MIDI_note, 0);
}

/**
  * @brief Instrument change or instrument selection
  * @param [file] Midi file pointer
  * @param [state] Logic state of note (ON/OFF)
  * @param [channel] Selection of midi channel (0-16)
  * @param [MIDI_note] Selection of midi note (1-127)
  * @param [velocity] Selection of velocity (power) (1-127)
*/
void MIDI_Note(FILE *file, unsigned char state, unsigned char channel, unsigned char MIDI_note, unsigned char velocity)
{
	unsigned char bytes[3];

	bytes[0] = state + channel % 16;
	bytes[1] = MIDI_note % 128;
	bytes[2] = velocity % 128;//Volume
	fwrite(&bytes, 3, 1, file);
}

/**
  * @brief Write Delta time
  * @param [file] Midi file pointer
  * @param [duration] Waiting duration in micro-seconds
*/
void MIDI_delta_time(FILE *file, uint32_t duration)
{
	MIDI_write_variable_length_quantity(file, duration);
}

/**
  * @brief Midi control parameters (pitch, attack, release, etc..)
  * @param [file] Midi file pointer
  * @param [channel] Selection of midi channel (0-16)
  * @param [type] Effect type (0-128)
  * @param [value] Effect value (1-127)
*/
void MIDI_Control_Change(FILE *file, unsigned char channel, unsigned char type, unsigned char value)
{
	unsigned char bytes[3];

	MIDI_delta_time(file, 0);
	bytes[0] = 0xB0 + channel % 16;
	bytes[1] = type % 128;
	bytes[2] = value % 128;
	fwrite(&bytes, 3, 1, file);
}

/**
  * @brief Write the End of track symbol
  * @param [file] Midi file pointer
*/
void MIDI_write_end_of_track(FILE *file)
{
	unsigned char bytes[3] = {0xFF, 0x2F, 0x00};

	MIDI_delta_time(file, 0);
	fwrite(&bytes, 3, 1, file);
}

/**
  * @brief Write track length
  * @param [file] Midi file pointer
  * @param [mark] Track header position
*/
void MIDI_write_track_length(FILE *file, uint32_t mark)
{
	unsigned char bytes[4] ;
	uint32_t size;

	size = ftell(file) - mark ;
	fseek(file, mark-4, SEEK_SET);
	bytes[0] = size >> 24;
	bytes[1] = size >> 16;
	bytes[2] = size >> 8;
	bytes[3] = size;
	fwrite(&bytes, 4, 1, file);
	fseek(file, 0, SEEK_END);
}

/**
  * @brief Write tempo in Midi line
  * @param [file] Midi file pointer
  * @param [tempo] quarter note (noire) value in micro-seconds
*/
void MIDI_tempo(FILE *file, uint32_t tempo)
{
	unsigned char bytes[6] = {0xFF, 0x51, 0x03};

	MIDI_delta_time(file, 0);
	bytes[3] = tempo >> 16;
	bytes[4] = tempo >> 8;
	bytes[5] = tempo;
	fwrite(&bytes, 6, 1, file);
}

/**
  * @brief Write variable lenght quantity
  * @param [file] Midi file pointer
  * @param [duration] Waiting duration in micro-seconds
*/
void MIDI_write_variable_length_quantity(FILE *file, uint32_t duration)
{
	bool pass;
	
	if (duration > 0x0FFFFFFF)
	{
		printf("ERROR : delay > 0x0FFFFFFF ! \n");
		exit(EXIT_FAILURE);
	}
	
	uint32_t filo = duration & 0x7F;

	duration = duration >> 7;
	while (duration != 0)
	{
		filo = (filo << 8) + ((duration & 0x7F) | 0x80);
		duration = duration >> 7;
	}

	do
	{
		fwrite(&filo, 1, 1, file);
		pass = filo & 0x80;
		if (pass)
		{
			filo = filo >> 8;
		}
	} while (pass);
}
