#include <string.h>

#include <tonc.h>

#include <general.h>
#include <menu.h>
#include <snake.h>
#include <memory_raid.h>










// Guarda todas as pontuações do jogo
// OBS: Sendo apenas 1 byte só dá para guardar 256 pontos
// O ideal era usar um inteiro e dividir suas partes na
// memória SRAM, mas assim é mais complicado
u8 snake_score				= 0;
u8 snake_score_high 		= 0;
u8 memory_raid_score 		= 0;
u8 memory_raid_score_high	= 0;










int main()
{
	// Permite interrupções de software com chamadas
	// de funções presentes na BIOS
	irq_init(NULL);
	irq_add(II_VBLANK, NULL);

	// Aparentemente melhor para acessar SRAM
	REG_WAITCNT = WS_SRAM_8;

	// Inicialização do som, muito complicado
	REG_SNDSTAT		= SSTAT_ENABLE;
    REG_SNDDMGCNT 	= SDMG_BUILD_LR(SDMG_SQR1, 7);
    REG_SNDDSCNT	= SDS_DMG100;
    REG_SND1SWEEP	= SSW_OFF;
    REG_SND1CNT		= SSQR_ENV_BUILD(12, 0, 7) | SSQR_DUTY1_2;
    REG_SND1FREQ	= 0;





	// Verifica se o jogo já foi salvo antes
	if (sram_mem[NULL_GAME] == 1)
	{
		snake_score_high		= sram_mem[SNAKE_GAME];
		memory_raid_score_high	= sram_mem[MEMORY_RAID_GAME];
	}





	while (1)
	{
		switch(init_menu())
		{
			case NULL_GAME:
				continue;
			break;



			case SNAKE_GAME:
				RegisterRamReset(RESET_PALETTE | RESET_VRAM);
				snake_score = init_snake_game();
				snake_score_high = MAX(snake_score, snake_score_high);
			break;



			case MEMORY_RAID_GAME:
				RegisterRamReset(RESET_PALETTE | RESET_VRAM);
				memory_raid_score = init_memory_raid_game();
				memory_raid_score_high = MAX(memory_raid_score, memory_raid_score_high);
			break;



			default:
				// Algo deu errado
				while (1);
			break;
		}





		// Salva a pontuação no cartucho
		sram_mem[NULL_GAME]			= 1;
		sram_mem[SNAKE_GAME]		= snake_score_high;
		sram_mem[MEMORY_RAID_GAME]	= memory_raid_score_high;
	}



	// Nunca deve chegar aqui
	while (1);
}