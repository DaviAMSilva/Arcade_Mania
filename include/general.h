#ifndef GENERAL_H
#define GENERAL_H

#include <stdlib.h>
#include <tonc.h>



#define NUM_GAMES 2

typedef enum GameIndex
{
	SNAKE_GAME,
	MEMORY_RAID_GAME,
	NULL_GAME,
} gameIndex_t;

void flash_save_word(u32 value, uint index);
u32 flash_read_word(uint index);

#endif /* GENERAL_H */