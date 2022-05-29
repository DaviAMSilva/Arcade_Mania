#include <stdlib.h>

#include <tonc.h>

#include <general.h>
#include <flash.h>

#include <data/BG_Menu_Games.h>
#include <data/BG_Menu_Overlay.h>










// 300 = 5s
#define RESET_TIMER 300










extern int snake_score;
extern int snake_score_high;
extern int memory_raid_score;
extern int memory_raid_score_high;
extern int wordle_scores[7];










// Tratamos o mapa de memória como um array de SBBs para facilitar a cópia
SCREENBLOCK *bg_menu_games = (SCREENBLOCK *)BG_Menu_GamesMap;










gameIndex_t init_menu(void)
{
	REG_DISPCNT = DCNT_MODE0;



	// Definindo os mapas
	REG_BG0CNT = BG_CBB(0) | BG_SBB(24)	| BG_REG_64x64 | BG_8BPP | BG_PRIO(1);
	REG_BG1CNT = BG_CBB(2) | BG_SBB(28)	| BG_REG_32x32 | BG_8BPP | BG_PRIO(0);

	// Copiando os tiles
	memcpy32(tile8_mem[0], BG_Menu_GamesTiles, BG_Menu_GamesTilesLen / 4);
	memcpy32(tile8_mem[2], BG_Menu_OverlayTiles, BG_Menu_OverlayTilesLen / 4);

	// Copiando os mapas
	// memcpy32(se_mem[24], BG_Menu_GamesMap, BG_Menu_GamesMapLen / 4);
	memcpy32(se_mem[28], BG_Menu_OverlayMap, BG_Menu_OverlayMapLen / 4);

	// A paleta de cores é igual para os dois
	memcpy32(pal_bg_mem, BG_Menu_OverlayPal, BG_Menu_OverlayPalLen / 4);
	pal_bg_mem[0] = CLR_BLACK;





	// Toda vez que volta para o menu continua no mesmo lugar
	static int game_index = 0;
	static int xoffset = 0;

	// Usado para definir qual o índice do SBB a ser usado
	static int sbb_index = 0;

	// Usado para reiniciar a pontuação mais alta
	int reset_timer = 0;





	// Copiando o mapa do primeiro jogo
	memcpy32(se_mem[24 + sbb_index], bg_menu_games + game_index, sizeof(bg_menu_games[0]) / 4);
	





	// FIXME: Fazer a impressão das pontuações voltar a funcionar
	// As pontuações ainda estão sendo salvas corretamente
	// Guarda as strings das pontuações
	// char scores_buffer[4][9] = {0};

	// snprintf(scores_buffer[0], 9, "%8.8d", 100 * snake_score_high);
	// snprintf(scores_buffer[1], 9, "%8.8d", 100 * snake_score);
	// snprintf(scores_buffer[2], 9, "%8.8d", 100 * memory_raid_score_high);
	// snprintf(scores_buffer[3], 9, "%8.8d", 100 * memory_raid_score);



	// Muito complicado para explicar, apenas inicia o texto para as pontuações
	// tte_init_se(
    //     0,
    //     BG_CBB(0) | BG_SBB(24) | BG_REG_64x64 | BG_8BPP | BG_PRIO(1),
    //     BG_Menu_GamesTilesLen / sizeof(TILE8),
    //     CLR_WHITE,
    //     14,
    //     NULL,
    //     NULL
	// );


	// Imprime as pontuações atuais e as maiores pontuações
	// tte_set_pos(64, 64);
	// tte_write(scores_buffer[0]);
	// tte_set_pos(64, 72);
	// tte_write(scores_buffer[1]);
	// tte_set_pos(64, 64 + 256);
	// tte_write(scores_buffer[2]);
	// tte_set_pos(64, 72 + 256);
	// tte_write(scores_buffer[3]);



	// Ativando os backgrounds
	REG_DISPCNT = DCNT_MODE0 | DCNT_BG0 | DCNT_BG1;
	REG_BG0HOFS = xoffset;










	while (true)
	{
		key_poll();



		// Se o jogador segurar SELECT as pontuações são resetadas
		if (key_is_down(KEY_SELECT))
		{
			if (reset_timer > RESET_TIMER)
			{
				snake_score				= 0;
				snake_score_high		= 0;
				memory_raid_score		= 0;
				memory_raid_score_high	= 0;

				// Apaga as pontuações
				flash_erase_sector(0);

				return NULL_GAME;
			}

			reset_timer++;
			VBlankIntrWait();
		}
		else
		{
			// O botão SELECT foi soltado e o timer reiniciou
			reset_timer = 0;
		}
		
		

		// Retorna para main e executa o jogo especificado
		if (key_hit(KEY_START))
		{
			return game_index + 1; // +1 pois os jogos começam em 1 
		}




		// Move esquerda/direita 64 * 4 = 256 pixels
		int direction = 0;
		if (key_is_down(KEY_R) || key_is_down(KEY_RIGHT))
			direction = 1;
		else if (key_is_down(KEY_L) || key_is_down(KEY_LEFT))
			direction = -1;

		if (direction)
		{
			game_index = (game_index + NUM_GAMES + direction) % NUM_GAMES;

			// Isso realiza a troca entre o primeiro e o segundo SBB
			sbb_index = (sbb_index + 1) % 2;

			// Copiando o mapa do próximo jogo dinamicamente
			memcpy32(se_mem[24 + sbb_index], bg_menu_games + game_index, sizeof(bg_menu_games[0]) / 4);

			// Move o background para a esquerda/direita 64 * 4 = 256 pixels
			for (int i = 0; i < 64; i++)
			{
				xoffset += 4 * direction;
				VBlankIntrWait();
				REG_BG0HOFS = xoffset;
			}
		}



		// Torna o jogo um pouco mais aleatório no início, dessa maneira a seed é dependente da
		// quantidade de frames que o jogo ficou ligado antes de ser iniciado 
		srand(rand() + game_index);
	}
}