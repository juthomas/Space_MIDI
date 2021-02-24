#include "../inc/midi.h"

int main(int argc, char **argv)
{
	midi_test(argc == 2 ? argv[1] : "output.mid");
}