#include <string.h>

#include <tonc.h>

#include <general.h>
#include <menu.h>
#include <snake.h>
#include <memory_raid.h>

int snake_score				= 0;
int snake_score_high 		= 0;
int memory_raid_score 		= 0;
int memory_raid_score_high	= 0;

int main()
{
	while (1)
	{
		switch(init_menu())
		{
			case SNAKE_GAME:
				clear_video_mem();
				snake_score = init_snake_game();
				snake_score_high = MAX(snake_score, snake_score_high);
			break;

			case MEMORY_RAID_GAME:
				clear_video_mem();
				memory_raid_score = init_memory_raid_game();
				memory_raid_score_high = MAX(memory_raid_score, memory_raid_score_high);
			break;

			default:
				while (1);
			break;
		}
	}

	while (1);
}