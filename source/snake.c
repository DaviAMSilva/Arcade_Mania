#include <snake.h>

/**
 * TODO: Criar uma lista encadeada com funções
 * para controlar o movimento da cobra e
 * implementar as outras funcionalidades
 */

void init_snake(void)
{
	REG_DISPCNT = DCNT_MODE0;

	// Iniciando os backgrounds
	REG_BG0CNT = BG_CBB(0) | BG_SBB(4) | BG_REG_32x32 | BG_8BPP;
	REG_BG1CNT = BG_CBB(0) | BG_SBB(5) | BG_REG_32x32 | BG_8BPP;

	// Copiando os tiles
	memcpy(tile8_mem[0], TL_SnakeTiles, TL_SnakeTilesLen);
	se_fill(&se_mat[5][0][0], 1);

	// Copiando a paleta de cores
	memcpy(pal_bg_mem, TL_SnakePal, TL_SnakePalLen);

	// Ativando os backgrounds
	REG_DISPCNT |= DCNT_BG0 | DCNT_BG1;

	int x = 0, y = 0, dx = 0, dy = 0;

	while (1)
	{
		// Passa 16 frames (0,2666s)
		for (u32 i = 0; i < 16; i++)
		{
			// Movimenta dependendo das teclas
			// é preciso testar em cada passagem
			key_poll();
			if (key_tri_vert())
				dx = 0, dy = key_tri_vert();
			else if (key_tri_horz())
				dx = key_tri_horz(), dy = 0;

			// Espera até o próximo frame
			vid_vsync();
		}

		// Apaga o quadrado anterior
		se_mat[4][y][x] = 0;

		// Moveo quadrado
		x = (x + dx + SCREEN_WIDTH_T) % SCREEN_WIDTH_T;
		y = (y + dy + SCREEN_HEIGHT_T) % SCREEN_HEIGHT_T;

		// Desenha o quadrado
		se_mat[4][y][x] = 4;
	}
}