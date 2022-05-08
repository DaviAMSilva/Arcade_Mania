#ifndef GENERAL_H
#define GENERAL_H

#include <tonc.h>

#define NUM_GAMES 2

#define SAVE_CODE 0xDEADBEEF

typedef enum GameIndex
{
	SAVE_GAME=0, // usado para salvar o jogo
	SNAKE_GAME,
	MEMORY_RAID_GAME,
} gameIndex_t;

void fade_to_black(void);

#endif /* GENERAL_H */