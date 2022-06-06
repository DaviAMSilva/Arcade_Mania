#include <tonc.h>
/**
 * Essa funcionalidade só foi possível de ser implementada por causa da excelente informação
 * técnica fornecida pelo site: problemkaputt.de/gbatek.htm criado por Martin Korth
 * disponível em: https://problemkaputt.de/gbatek.htm#gbacartbackupflashrom
 */









// Isso é necessário quando que se utiliza um nível de otimização
// acima de O1, pois essa memória é muito sensível
static volatile u8 *const flash_mem = (volatile u8 *)sram_mem;



// Apaga todos os valores de um setor
// Isso é necessário pois só é possível escrever em memória que foi apagada
void flash_erase_sector(u8 s)
{
    // Comando de apagar
    *(flash_mem + (0x5555)) = 0xAA;
    *(flash_mem + (0x2AAA)) = 0x55;
    *(flash_mem + (0x5555)) = 0x80;

    // Comando de apagar setor s
    *(flash_mem + (0x5555)) = 0xAA;
    *(flash_mem + (0x2AAA)) = 0x55;
    *(flash_mem + (0x0000 + (s << 4))) = 0x30;

    // Esperar até que E00s000 == 0xFF
    while (*(flash_mem + (0x0000 + (s << 4))) != 0xFF);
}



// Salva uma palavra (4 bytes) para memória
// Isso é necessário pois essa memória só permite escritas de 1 byte
void flash_save_word(u32 value, uint index)
{
    for (u8 i = 0; i < 4; i++)
	{
   		// Comando de escrever byte
		*(flash_mem + (0x5555)) = 0xAA;
		*(flash_mem + (0x2AAA)) = 0x55;
		*(flash_mem + (0x5555)) = 0xA0;

		u8 byte = (value >> (i * 8)) & 0xFF;

    	// Escrever byte E00xxxx
    	*(flash_mem + i + index * 4) = byte;
		
    	// Esperar até que E00xxxx == 0xFF
		while (*(flash_mem + i + index * 4) != byte);
	}
}



// Lê uma palavra (4 bytes) da memória
// Isso é necessário pois essa memória só permite leituras de 1 byte
u32 flash_read_word(uint index)
{
	return bytes2word(
		flash_mem[index * 4 + 0],
		flash_mem[index * 4 + 1],
		flash_mem[index * 4 + 2],
		flash_mem[index * 4 + 3]
	);
}