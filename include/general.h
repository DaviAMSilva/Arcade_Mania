#ifndef GENERAL_H
#define GENERAL_H

#include <tonc.h>

#define NUM_GAMES 2

#define SAVE_CODE 0xDEADBEEF

typedef enum GameIndex
{
	NULL_GAME=0, // usado para salvar o jogo
	SNAKE_GAME=1,
	MEMORY_RAID_GAME=2,
} gameIndex_t;

void fade_to_black(void);

#endif /* GENERAL_H */