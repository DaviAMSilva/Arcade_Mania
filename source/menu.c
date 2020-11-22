#include <stdlib.h>

#include <tonc.h>

#include <general.h>

#include <data/BG_Menu.h>
#include <data/BG_MenuOverlay.h>





extern int snake_score;
extern int snake_score_high;
extern int memory_raid_score;
extern int memory_raid_score_high;










gameIndex_t init_menu(void)
{
	// Toda vez que volta para o menu continua no mesmo lugar
	static int game_index = SNAKE_GAME;
	static int xoffset = 0;










	REG_DISPCNT = DCNT_MODE0;
	
	REG_BG0CNT = BG_CBB(0) | BG_SBB(16)	| BG_REG_64x32 | BG_8BPP | BG_PRIO(1);
	REG_BG1CNT = BG_CBB(1) | BG_SBB(18)	| BG_REG_32x32 | BG_8BPP | BG_PRIO(0);

	// Copiando os tiles
	memcpy32(tile8_mem[0], BG_MenuTiles, BG_MenuTilesLen / 4);
	memcpy32(tile8_mem[1], BG_MenuOverlayTiles, BG_MenuOverlayTilesLen / 4);

	// Copiando os mapas
	memcpy32(se_mem[16], BG_MenuMap, BG_MenuMapLen / 4);
	memcpy32(se_mem[18], BG_MenuOverlayMap, BG_MenuOverlayMapLen / 4);

	// A paleta de cores é igual para os dois
	memcpy32(pal_bg_mem, BG_MenuPal, BG_MenuPalLen / 4);

	// Ativando os backgrounds
	REG_DISPCNT |= DCNT_BG0 | DCNT_BG1;
	REG_BG0HOFS = xoffset;










	while (1)
	{
		key_poll();



		if (key_hit(KEY_START))
		{
			return game_index;
		}




		// Move esquerda/direita
		if (key_is_down(KEY_R) || key_is_down(KEY_RIGHT))
		{
			for (int i = 0; i < 64; i++)
			{
				xoffset += 4;
				vid_vsync();
				REG_BG0HOFS = xoffset;
			}

			game_index = (game_index + NUM_GAMES + 1) % NUM_GAMES;
		}
		else if (key_is_down(KEY_L) || key_is_down(KEY_LEFT))
		{
			for (int i = 0; i < 64; i++)
			{
				xoffset -= 4;
				vid_vsync();
				REG_BG0HOFS = xoffset;
			}

			game_index = (game_index + NUM_GAMES - 1) % NUM_GAMES;
		}



		// Deixa um pouco mais aleatório no início
		srand(rand() + game_index);
	}
}