#ifndef GENERAL_H
#define GENERAL_H

#include <tonc.h>

#define NUM_GAMES 3

#define SAVE_CODE 0xDA710001 // Serve como identificador da versão do save

typedef enum GameIndex
{
	NULL_GAME = 0, // Usado para salvar o jogo
	SNAKE_GAME = 1,
	MEMORY_RAID_GAME = 2,
	WORDLE_GAME = 3,
} gameIndex_t;

void fade_to_black(void);

#endif /* GENERAL_H */