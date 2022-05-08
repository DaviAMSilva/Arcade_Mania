#ifndef FLASH_H
#define FLASH_H

#include <tonc.h>

void flash_erase_sector(u8 sector);
void flash_save_word(u32 value, uint index);
u32 flash_read_word(uint index);

#endif /* FLASH_H */