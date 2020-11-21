#include <data/BG_Entrada.h>
#include <data/BG_EntradaOverlay.h>

#include <tonc.h>
#include <tonc_libgba.h>
#include <string.h>

#include <snake.h>
#include <memory_raid.h>

int snake_score				= 0;
int snake_score_high 		= 0;
int memory_raid_score 		= 0;
int memory_raid_score_high	= 0;

int main()
{
	// init_snake_game();
	init_memory_raid_game();

	while (1);
}