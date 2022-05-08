#include <tonc.h>










// Faz com que todas as cores passem a ser pretas
// mas com certos delays ao invés de preto instantâneo
void fade_to_black(void)
{
	VBlankIntrDelay(60);

	for (int i = 0; i < 5; i++)
	{
		for (int j = 0; j < 60; j++)
			VBlankIntrWait();

		clr_fade_fast(pal_bg_mem, CLR_BLACK, pal_bg_mem, 256, 15);
		clr_fade_fast(pal_obj_mem, CLR_BLACK, pal_obj_mem, 256, 15);
	}

	RegisterRamReset(RESET_PALETTE);

	VBlankIntrDelay(60);
}





// Salva uma palavra (4 bytes) para memória
// Isso é necessário pois essa memória só permite escritas de 1 byte
void flash_save_word(u32 value, uint index)
{
	/**
	 * AVISO: Eu tentei de tudo que eu podia para poder salvar
	 * informações no cartucho, mas o meu cartucho é da Atmel
	 * onde o processo de escrita é mais complicado e não estou
	 * conseguindo descobrir como pois há pouca informação
	 * 
	 * Entretando ainda funciona normal em emuladores em que não há
	 * essa restrição
	 */

	// *(u8 *)0xE005555 = 0xAA;
	// *(u8 *)0xE002AAA = 0x55;
	// *(u8 *)0xE005555 = 0xA0;
	// *(u8 *)(0xE000000 + 0 + index * 4) = (u8)(value >> 0 & 0xFF);
	// while (*(u8 *)(0xE000000 + 0 + index * 4) != (u8)(value >> 0 & 0xFF));

	// *(u8 *)0xE005555 = 0xAA;
	// *(u8 *)0xE002AAA = 0x55;
	// *(u8 *)0xE005555 = 0xA0;
	// *(u8 *)(0xE000000 + 1 + index * 4) = (u8)(value >> 8 & 0xFF);
	// while (*(u8 *)(0xE000000 + 1 + index * 4) != (u8)(value >> 8 & 0xFF));

	// *(u8 *)0xE005555 = 0xAA;
	// *(u8 *)0xE002AAA = 0x55;
	// *(u8 *)0xE005555 = 0xA0;
	// *(u8 *)(0xE000000 + 2 + index * 4) = (u8)(value >> 16 & 0xFF);
	// while (*(u8 *)(0xE000000 + 2 + index * 4) != (u8)(value >> 16 & 0xFF));

	// *(u8 *)0xE005555 = 0xAA;
	// *(u8 *)0xE002AAA = 0x55;
	// *(u8 *)0xE005555 = 0xA0;
	// *(u8 *)(0xE000000 + 3 + index * 4) = (u8)(value >> 24 & 0xFF);
	// while (*(u8 *)(0xE000000 + 3 + index * 4) != (u8)(value >> 24 & 0xFF));


    for (u8 i = 0; i < 4; i++)
	{
   		// write byte command
		*(u8 *)0xE005555 = 0xAA;
		*(u8 *)0xE002AAA = 0x55;
		*(u8 *)0xE005555 = 0xA0;

		u8 byte = (value >> (i * 8)) & 0xFF;

    	// write byte
    	*(u8 *)(0xE000000 + i + index * 4) = byte;
		
    	// wait until E000000
		while (*(u8 *)(0xE000000 + i + index * 4) != byte)
			;
	}

	// sram_mem[index * 4 + 0] = (u8)((value & 0x000000FF) >>  0);
	// sram_mem[index * 4 + 1] = (u8)((value & 0x0000FF00) >>  8);
	// sram_mem[index * 4 + 2] = (u8)((value & 0x00FF0000) >> 16);
	// sram_mem[index * 4 + 3] = (u8)((value & 0xFF000000) >> 24);
}



// Lê uma palavra (4 bytes) da memória
// Isso é necessário pois essa memória só permite leituras de 1 byte
u32 flash_read_word(uint index)
{
	return bytes2word(
		sram_mem[index * 4 + 0],
		sram_mem[index * 4 + 1],
		sram_mem[index * 4 + 2],
		sram_mem[index * 4 + 3]
	);
}