#include <tonc.h>
/**
 * Essa funcionalidade só foi possível de ser implementada por causa da excelente informação
 * técnica fornecidade pelo site: problemkaputt.de/gbatek.htm criado por Martin Korth
 * disponível em: https://problemkaputt.de/gbatek.htm#gbacartbackupflashrom
 */









// Apaga todos os valores de um setor
// Isso é necessário pois só é possível escrever em memória que foi apagada
void flash_erase_sector(u8 s)
{
    // Comando de apagar
    *(sram_mem + (0x5555)) = 0xAA;
    *(sram_mem + (0x2AAA)) = 0x55;
    *(sram_mem + (0x5555)) = 0x80;

    // Comando de apagar setor s
    *(sram_mem + (0x5555)) = 0xAA;
    *(sram_mem + (0x2AAA)) = 0x55;
    *(sram_mem + (0x0000 + (s << 4))) = 0x30;

    // Esperar até que E00s000 == 0xFF
    while (*(sram_mem + (0x0000 + (s << 4))) != 0xFF);
}



// Salva uma palavra (4 bytes) para memória
// Isso é necessário pois essa memória só permite escritas de 1 byte
void flash_save_word(u32 value, uint index)
{
    for (u8 i = 0; i < 4; i++)
	{
   		// Comando de escrever byte
		*(sram_mem + (0x5555)) = 0xAA;
		*(sram_mem + (0x2AAA)) = 0x55;
		*(sram_mem + (0x5555)) = 0xA0;

		u8 byte = (value >> (i * 8)) & 0xFF;

    	// Escrever byte E00xxxx
    	*(sram_mem + i + index * 4) = byte;
		
    	// Esperar até que E00xxxx == 0xFF
		while (*(sram_mem + i + index * 4) != byte);
	}
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