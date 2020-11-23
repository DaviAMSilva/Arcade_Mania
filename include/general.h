#ifndef GENERAL_H
#define GENERAL_H

#define NUM_GAMES 2

typedef enum GameIndex
{
	SNAKE_GAME,
	MEMORY_RAID_GAME,
	NULL_GAME,
} gameIndex_t;

void fade_to_black(void);

#endif /* GENERAL_H */