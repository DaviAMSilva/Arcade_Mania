#include <tonc.h>










// Faz com que todas as cores passem a ser pretas
// mas com certos delays ao invés de preto instantâneo
void fade_to_black(void)
{
	VBlankIntrDelay(120);

	for (int i = 0; i < 5; i++)
	{
		for (int j = 0; j < 60; j++)
			VBlankIntrWait();

		clr_fade_fast(pal_bg_mem, CLR_BLACK, pal_bg_mem, 256, 15);
		clr_fade_fast(pal_obj_mem, CLR_BLACK, pal_obj_mem, 256, 15);
	}

	memset16(pal_bg_mem, CLR_BLACK, 256);
	memset16(pal_obj_mem, CLR_BLACK, 256);

	VBlankIntrDelay(60);
}