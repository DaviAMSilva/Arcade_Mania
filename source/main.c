#include <BG_Entrada.h>
#include <BG_EntradaOverlay.h>
#include <gba.h>

#include <string.h>

/**
 * Essa é uma demonstração do como exibir imagens no GBA
 * sendo que este exemplo consegue exibir duas imagens
 * diferentes ao mesmo tempo utilizando-se de dois backgrounds
 */
int main()
{
    SetMode(MODE_0);

    // Iniciando os backgrounds 0 e 1
    REG_BG0CNT = BG_256_COLOR | BG_MAP_BASE(16) | BG_TILE_BASE(0) | TEXTBG_SIZE_256x256 | BG_PRIORITY(1);
    REG_BG1CNT = BG_256_COLOR | BG_MAP_BASE(17) | BG_TILE_BASE(1) | TEXTBG_SIZE_256x256;

    // Copiando os tiles
    memcpy(CHAR_BASE_BLOCK(0), BG_EntradaTiles, BG_EntradaTilesLen);
    memcpy(CHAR_BASE_BLOCK(1), BG_EntradaOverlayTiles, BG_EntradaOverlayTilesLen);

    // Copiando os tilemaps
    memcpy(SCREEN_BASE_BLOCK(16), BG_EntradaMap, BG_EntradaMapLen);
    memcpy(SCREEN_BASE_BLOCK(17), BG_EntradaOverlayMap, BG_EntradaOverlayMapLen);

    // Copiando a paleta de cores
    memcpy(BG_PALETTE, BG_EntradaPal, BG_EntradaPalLen);



    /**
     * Aqui os tilemaps e as paletas das imagens diferentes precisam ser
     * juntadas em um só, manualmente. O próximo passo será criar uma
     * função que possa fazer isso automaticamente.
     */
    BG_PALETTE[19] = BG_EntradaOverlayPal[0];
    BG_PALETTE[20] = BG_EntradaOverlayPal[1];

    u32 *ptr = CHAR_BASE_BLOCK(1);

    for (u32 i = 0; i < BG_EntradaOverlayTilesLen; i++)
        if (ptr[i] != 0) ptr[i] += 0x13131313;



    //Ativando os backgrounds 0 e 1
    SetMode(MODE_0 | BG0_ENABLE | BG1_ENABLE);

    while (1);
}
