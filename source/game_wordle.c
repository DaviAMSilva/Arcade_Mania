#include <stdlib.h>
#include <string.h>

#include <tonc.h>

#include <general.h>

#include <data/SP_Wordle.h>









#define WORD_LENGTH			    5
#define NUM_ATTEMPTS		    6
#define BUTTON_LENGTH		    4
#define NUM_POSSIBLE_LETTERS	26
#define NUM_SPECIAL_BUTTONS     2
#define NUM_BUTTONS			    (NUM_POSSIBLE_LETTERS + NUM_SPECIAL_BUTTONS)
#define NUM_LETTERS			    (NUM_ATTEMPTS * WORD_LENGTH)



#define LETTER_SIZE_X	16
#define LETTER_SIZE_Y	16
#define LETTER_OFFSET_X	24
#define LETTER_OFFSET_Y	16

#define BUTTON_SIZE_X	16
#define BUTTON_SIZE_Y	16
#define BUTTON_OFFSET_X	152
#define BUTTON_OFFSET_Y	16



#define COLOR_GREEN		RGB15( 0,	31,	 0)
#define COLOR_RED		RGB15(31,	 0,	 0)
#define COLOR_BLUE		RGB15( 0,	 0,	31)
#define COLOR_YELLOW	RGB15(31,	31,	 0)
#define COLOR_PURPLE	RGB15(31,	 0,	31)
#define COLOR_CYAN		RGB15( 0,	31,	31)

#define COLOR_BLACK		RGB15( 0,	 0,	 0)
#define COLOR_GREY		RGB15(10,	10,	10)
#define COLOR_LIGHTGREY	RGB15(20,	20,	20)
#define COLOR_WHITE		RGB15(31,	31,	31)
#define COLOR_TRANSP	COLOR_PURPLE










typedef enum WordlePalBank // Paletas possíveis
{
	PAL_EMPTY=0,
    PAL_CORRECT,
    PAL_CLOSE,
    PAL_INCORRECT,
    PAL_BUTTON,
} wordlePalBank_t;

typedef enum WordleSpecialButton
{
    BUTTON_BACKSPACE=0,
    BUTTON_ENTER,
} wordleSpecialButton_t;










struct Object
{
	POINT pos;
	OBJ_ATTR *obj;
	int pal_index;
	int dir_index;
} ALIGN4;
typedef struct Object object_t;



struct LetterPallete
{
    COLOR transp;
    COLOR border1;
    COLOR background;
    COLOR text;
    COLOR border2;
    COLOR filler[11];
} ALIGN4;
typedef struct LetterPallete letterPallete_t;









static int get_sprite_id(int index, bool isLetter);











// Serve para guardar as informações dos sprites antes de enviar para tela
// Pode parecer ineficiente mas as vezes é possível não existir tempo suficiente
// para atualizar todos os sprites, então é melhor assim
static OBJ_ATTR obj_buffer[128];
static OBJ_ATTR *obj_buffer_letters = obj_buffer;
static OBJ_ATTR *obj_buffer_buttons = obj_buffer + NUM_LETTERS;








int init_wordle_game(void)
{
	REG_DISPCNT = DCNT_MODE0;

	// Copia as informações necessárias sobre os objetos
	memcpy32(tile_mem_obj, SP_WordleTiles, SP_WordleTilesLen / 4);

	// Ativando os sprites
	REG_DISPCNT = DCNT_OBJ | DCNT_OBJ_2D;





	// Esconde todos os sprites
	oam_init(obj_buffer, 128);





    // Inicializando as paletas de cores
    const letterPallete_t EmptyPallete = {COLOR_TRANSP, COLOR_BLACK, COLOR_BLACK, COLOR_BLACK, COLOR_GREY};
    const letterPallete_t CorrectPallete = {COLOR_TRANSP, COLOR_BLACK, COLOR_WHITE, COLOR_GREEN, COLOR_GREY};
    const letterPallete_t ClosePallete = {COLOR_TRANSP, COLOR_BLACK, COLOR_WHITE, COLOR_YELLOW, COLOR_GREY};
    const letterPallete_t IncorrectPallete = {COLOR_TRANSP, COLOR_BLACK, COLOR_WHITE, COLOR_GREY, COLOR_GREY};
    const letterPallete_t ButtonPallete = {COLOR_TRANSP, COLOR_BLACK, COLOR_WHITE, COLOR_BLUE, COLOR_GREY};

    const letterPallete_t letterPalletes[] = {EmptyPallete, CorrectPallete, ClosePallete, IncorrectPallete, ButtonPallete};

    for (int i = 0; i < sizeof(letterPalletes) / sizeof(letterPallete_t); i++)
    {
        pal_obj_bank[i][0] = letterPalletes[i].transp;
        pal_obj_bank[i][1] = letterPalletes[i].border1;
        pal_obj_bank[i][2] = letterPalletes[i].background;
        pal_obj_bank[i][3] = letterPalletes[i].text;
        pal_obj_bank[i][4] = letterPalletes[i].border2;
    }









    // Inicializando as letras das palavras
	for (int i = 0; i < WORD_LENGTH; i++)
	{
		for (int j = 0; j < NUM_ATTEMPTS; j++)
		{
			int index = (i + j * WORD_LENGTH);

			obj_set_attr(
				obj_buffer_letters + index,
				ATTR0_SQUARE,
				ATTR1_SIZE_16,
				ATTR2_PALBANK(PAL_EMPTY) | ATTR2_ID(0)
			);

			obj_set_pos(
				obj_buffer_letters + index,
				(i * LETTER_SIZE_X) + LETTER_OFFSET_X,
				(j * LETTER_SIZE_Y) + LETTER_OFFSET_Y
			);

			obj_unhide(obj_buffer_letters + index, ATTR0_REG);
		}
	}





    // Inicializando os botões
    for (int i = 0; i < NUM_BUTTONS; i++)
    {
        obj_set_attr(
            obj_buffer_buttons + i,
            ATTR0_SQUARE,
            ATTR1_SIZE_16,
            ATTR2_PALBANK(PAL_BUTTON) | ATTR2_ID(get_sprite_id(i, true))
        );

        obj_set_pos(
            obj_buffer_buttons + i,
            (i * BUTTON_SIZE_X) % (BUTTON_LENGTH * BUTTON_SIZE_X) + BUTTON_OFFSET_X,
            BUTTON_OFFSET_Y + (i / BUTTON_LENGTH) * BUTTON_SIZE_Y
        );

        obj_unhide(obj_buffer_buttons + i, ATTR0_REG);
    }

    // Os botões de BACKSPACE e ENTER tem id diferentes
    (obj_buffer_buttons + NUM_BUTTONS - 1)->attr2 = ATTR2_PALBANK(PAL_BUTTON) | ATTR2_ID(get_sprite_id(BUTTON_BACKSPACE, false));
    (obj_buffer_buttons + NUM_BUTTONS - 2)->attr2 = ATTR2_PALBANK(PAL_BUTTON) | ATTR2_ID(get_sprite_id(BUTTON_ENTER, false));





	while(true)
	{
		VBlankIntrWait();
		obj_copy(obj_mem, obj_buffer, 128);
	}

	return 0;
}










static int get_sprite_id(int index, bool isLetter)
{
    // os sprites tem um tamanho de 16x16px (2x2 tiles), mas a imagem de tiles é composta de 16x16 tiles
    // então é necessário utilizar os índices de tile das seguintes posições:
    /*
    +------------------------------------------------+
    |[00][__][02][__][04][__][..][__][28][__][30][__]|
    |[__][__][__][__][__][__][..][__][__][__][__][__]|
    |[64][__][66][__][68][__][..][__][92][__][94][__]|
    |[__][__][__][__][__][__][..][__][__][__][__][__]|
    |[128]...........................................|
    +------------------------------------------------+
    */

    if (isLetter)
    {
        return index * 2 + 32 * (index / 16);
    } else {
        return index * 2 + 128;
    }

}