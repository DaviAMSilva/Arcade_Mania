#ifndef GENERAL_H
#define GENERAL_H

#define NUM_GAMES 2

typedef enum GameIndex
{
	SNAKE_GAME,
	MEMORY_RAID_GAME,
} gameIndex_t;

void clear_video_mem(void);

#endif /* GENERAL_H */