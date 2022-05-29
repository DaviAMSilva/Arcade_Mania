#include <string.h>

#include <tonc.h>

#include <general.h>
#include <menu.h>
#include <flash.h>

#include <game_snake.h>
#include <game_memory_raid.h>
#include <game_wordle.h>










int snake_score = 0;
int snake_score_high = 0;
int memory_raid_score = 0;
int memory_raid_score_high = 0;
int wordle_scores[7] = {0};










int main()
{
	// Permite interrupções de software com chamadas
	// de funções presentes na BIOS
	irq_init(NULL);
	irq_add(II_VBLANK, NULL);

	// Inicialização do som, muito complicado
	REG_SNDSTAT		= SSTAT_ENABLE;
    REG_SNDDMGCNT 	= SDMG_BUILD_LR(SDMG_SQR1, 7);
    REG_SNDDSCNT	= SDS_DMG100;
    REG_SND1SWEEP	= SSW_OFF;
    REG_SND1CNT		= SSQR_ENV_BUILD(12, 0, 7) | SSQR_DUTY1_2;
    REG_SND1FREQ	= 0;



	// Verifica se o jogo já foi salvo antes
	if (flash_read_word(NULL_GAME) == SAVE_CODE)
	{
		snake_score_high		= flash_read_word(SNAKE_GAME);
		memory_raid_score_high	= flash_read_word(MEMORY_RAID_GAME);
		for (int i = 0; i < 7; i++)
			wordle_scores[i] = flash_read_word(WORDLE_GAME + i);
	}



	while (true)
	{
		switch(init_menu())
		{
			case NULL_GAME:
				continue;
			break;



			case SNAKE_GAME:
				RegisterRamReset(RESET_PALETTE | RESET_VRAM | RESET_OAM);
				snake_score = init_snake_game();
				snake_score_high = MAX(snake_score, snake_score_high);
			break;



			case MEMORY_RAID_GAME:
				RegisterRamReset(RESET_PALETTE | RESET_VRAM | RESET_OAM);
				memory_raid_score = init_memory_raid_game();
				memory_raid_score_high = MAX(memory_raid_score, memory_raid_score_high);
			break;



			case WORDLE_GAME:
				RegisterRamReset(RESET_PALETTE | RESET_VRAM | RESET_OAM);
				wordle_scores[init_wordle_game()]++;
			break;



			default:
				// Algo deu errado
				RegisterRamReset(RESET_PALETTE | RESET_VRAM | RESET_OAM);
				while (true);
			break;
		}

		// Salva a pontuação no cartucho
		flash_erase_sector(0);
		flash_save_word(SAVE_CODE, NULL_GAME);
		flash_save_word(snake_score_high, SNAKE_GAME);
		flash_save_word(memory_raid_score_high, MEMORY_RAID_GAME);
		for (int i = 0; i < 7; i++)
			flash_save_word(wordle_scores[i], WORDLE_GAME + i);
	}



	// Nunca deve chegar aqui
	RegisterRamReset(RESET_PALETTE | RESET_VRAM | RESET_OAM);
	while (true);
}