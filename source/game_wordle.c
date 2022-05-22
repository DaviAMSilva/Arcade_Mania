#include <stdlib.h>
#include <string.h>

#include <tonc.h>

#include <general.h>
#include <game_wordle_words.h>

#include <data/SP_Wordle.h>









#define WORD_LENGTH			    5
#define NUM_ATTEMPTS		    6
#define NUM_POSSIBLE_LETTERS	26
#define NUM_SPECIAL_BUTTONS     2
#define NUM_BUTTONS			    (NUM_POSSIBLE_LETTERS + NUM_SPECIAL_BUTTONS)
#define NUM_LETTERS			    (NUM_ATTEMPTS * WORD_LENGTH)



#define WORDLE_WORD_BANK_LENGTH		WORDLE_WORD_BANK_5_LENGTH
#define WORDLE_WORD_VALID_LENGTH	WORDLE_WORD_VALID_5_LENGTH
#define wordle_word_bank			wordle_word_bank_5
#define wordle_word_valid			wordle_word_valid_5



#define LETTER_SIZE_X	16
#define LETTER_SIZE_Y	16
#define LETTER_OFFSET_X	24
#define LETTER_OFFSET_Y	16
#define LETTER_LENGTH_X	WORD_LENGTH
#define LETTER_LENGTH_Y	NUM_ATTEMPTS

#define BUTTON_SIZE_X	16
#define BUTTON_SIZE_Y	16
#define BUTTON_OFFSET_X	152
#define BUTTON_OFFSET_Y	16
#define BUTTON_LENGTH_X	4
#define BUTTON_LENGTH_Y	7



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










typedef enum WordlePalBank
{
	PAL_EMPTY = 0,
	PAL_CORRECT,
	PAL_CLOSE,
	PAL_INCORRECT,
	PAL_BUTTON,
} wordlePalBank_t;

typedef enum WordleSpecialSymbol
{
	SYMBOL_EMPTY = 0,
	SYMBOL_BACKSPACE = 1 + NUM_POSSIBLE_LETTERS,
	SYMBOL_ENTER,
	SYMBOL_SELECTOR,
} wordleSpecialSymbol_t;









static int get_sprite_id(int index);
static void update_letters(OBJ_ATTR *obj_letters, char *letter_buffer, wordlePalBank_t *letter_pal_buffer);
static void wordle_compare(wordlePalBank_t *letter_pal_buffer, char *solution, char* word);











// Serve para guardar as informações dos sprites antes de enviar para tela
// Pode parecer ineficiente mas as vezes é possível não existir tempo suficiente
// para atualizar todos os sprites, então é melhor assim
static OBJ_ATTR obj_buffer[128];
static OBJ_ATTR *obj_letters    = obj_buffer;
static OBJ_ATTR *obj_buttons    = obj_buffer + NUM_LETTERS;
static OBJ_ATTR *obj_selector   = obj_buffer + NUM_LETTERS + NUM_BUTTONS;
static OBJ_ATTR *obj_cursor		= obj_buffer + NUM_LETTERS + NUM_BUTTONS + 1;



// Salvam o id das letras e sua paleta antes que os sprites sejam atualizados
static char letter_buffer[LETTER_LENGTH_Y][LETTER_LENGTH_X] = { SYMBOL_EMPTY };
static wordlePalBank_t letter_pal_buffer[LETTER_LENGTH_Y][LETTER_LENGTH_X] = { PAL_EMPTY };



// A palavra que se está tentando achar
static char solution[WORD_LENGTH] = "laugh"; // TODO: aleatorizar solução










// Trazendo as constantes que guardam as palavras
extern const char wordle_word_bank_5[WORDLE_WORD_BANK_5_LENGTH][5];
extern const char wordle_word_valid_5[WORDLE_WORD_VALID_5_LENGTH][5];










int init_wordle_game(void)
{
	REG_DISPCNT = DCNT_MODE0;

	// Copia as informações necessárias sobre os objetos
	memcpy32(tile_mem_obj, SP_WordleTiles, SP_WordleTilesLen / 4);

	// Ativando os sprites
	REG_DISPCNT = DCNT_OBJ | DCNT_OBJ_2D;





	// Esconde todos os sprites
	oam_init(obj_buffer, 128);





	// Copiando a paleta 5 vezes
	for (int i = 0; i < 5; i++)
		memcpy32(pal_obj_bank[i], SP_WordlePal, 8); // 8 words = 32 bytes = 16 cores

	pal_obj_bank[PAL_EMPTY][3] = COLOR_BLACK;
	pal_obj_bank[PAL_CORRECT][3] = COLOR_GREEN;
	pal_obj_bank[PAL_CLOSE][3] = COLOR_YELLOW;
	pal_obj_bank[PAL_INCORRECT][3] = COLOR_RED;
	pal_obj_bank[PAL_BUTTON][3] = COLOR_BLUE;












	// Inicializando as letras das palavras
	for (int i = 0; i < LETTER_LENGTH_X; i++)
	{
		for (int j = 0; j < LETTER_LENGTH_Y; j++)
		{
			int index = (i + j * LETTER_LENGTH_X);

			obj_set_attr(
				obj_letters + index,
				ATTR0_SQUARE,
				ATTR1_SIZE_16,
				ATTR2_PALBANK(PAL_EMPTY) | ATTR2_ID(0) | ATTR2_PRIO(1)
			);

			obj_set_pos(
				obj_letters + index,
				(i * LETTER_SIZE_X) + LETTER_OFFSET_X,
				(j * LETTER_SIZE_Y) + LETTER_OFFSET_Y
			);

			obj_unhide(obj_letters + index, ATTR0_REG);
		}
	}





	// Inicializando os botões
	for (int i = 0; i < NUM_BUTTONS; i++)
	{
		obj_set_attr(
			obj_buttons + i,
			ATTR0_SQUARE,
			ATTR1_SIZE_16,
			ATTR2_PALBANK(PAL_BUTTON) | ATTR2_ID(get_sprite_id(i + 1)) | ATTR2_PRIO(1)
		);

		obj_set_pos(
			// Transforma o índice linear do botão em coordenadas bidimensionais
			obj_buttons + i,
			(i * BUTTON_SIZE_X) % (BUTTON_LENGTH_X * BUTTON_SIZE_X) + BUTTON_OFFSET_X,
			BUTTON_OFFSET_Y + (i / BUTTON_LENGTH_X) * BUTTON_SIZE_Y
		);

		obj_unhide(obj_buttons + i, ATTR0_REG);
	}





	// Inicializando o seletor
	obj_set_attr(
		obj_selector,
		ATTR0_SQUARE,
		ATTR1_SIZE_16,
		ATTR2_PALBANK(PAL_EMPTY) | ATTR2_ID(get_sprite_id(SYMBOL_SELECTOR))
	);

	obj_set_pos(
		obj_selector,
		BUTTON_OFFSET_X,
		BUTTON_OFFSET_Y
	);

	obj_unhide(obj_selector, ATTR0_REG);





	// Inicializando o cursor
	obj_set_attr(
		obj_cursor,
		ATTR0_SQUARE,
		ATTR1_SIZE_16,
		ATTR2_PALBANK(PAL_EMPTY) | ATTR2_ID(get_sprite_id(SYMBOL_SELECTOR))
	);

	obj_set_pos(
		obj_cursor,
		LETTER_OFFSET_X,
		LETTER_OFFSET_Y
	);

	obj_unhide(obj_cursor, ATTR0_REG);





	// Essa variável será usada para saber qual letra está selecionada
	POINT selector_pos = {0, 0};

	// Essa variável será usada para saber qual letra será escrita
	POINT cursor_pos = {0, 0};





	while(true)
	{
		key_poll();





		// Quando é apertado o botão A adiciona-se a letra selecionada
		if (key_hit(KEY_A) || key_hit(KEY_B) || key_hit(KEY_START))
		{
			int selector_index = selector_pos.x + selector_pos.y * BUTTON_LENGTH_X + 1;

			if ((key_hit(KEY_A) && selector_index == SYMBOL_BACKSPACE) || key_hit(KEY_B))
			{
				// Remover a letra se o cursor não estiver na posição 0,0
				// Mover o cursor para a linha anterior caso necessário
				if (cursor_pos.x > 0)
				{
					cursor_pos.x--;
					letter_buffer[cursor_pos.y][cursor_pos.x] = SYMBOL_EMPTY;
				}
			}
			else if ((key_hit(KEY_A) && selector_index == SYMBOL_ENTER) || key_hit(KEY_START))
			{
				bool is_valid = false;

				// Compara a palavra guardada com todas as palavras válidas
				for (int i = 0; i < WORDLE_WORD_VALID_LENGTH; i++)
				{
					if (strncmp(letter_buffer[cursor_pos.y], wordle_word_valid[i], WORD_LENGTH) == 0)
					{
						is_valid = true;
						break;
					}
				}

				// Se o cursor estiver no final da palavra e não é a última linha e a palavra é válida então a linha é finalizada
				if (is_valid && cursor_pos.x == LETTER_LENGTH_X && cursor_pos.y < LETTER_LENGTH_Y - 1)
				{
					wordle_compare(letter_pal_buffer[cursor_pos.y], solution, letter_buffer[cursor_pos.y]);

					cursor_pos.x = 0;
					cursor_pos.y++;
				}
			}
			else if (key_hit(KEY_A)) {
				// Adicionar a letra selecionada
				if (cursor_pos.x <= LETTER_LENGTH_X - 1)
				{
					letter_buffer[cursor_pos.y][cursor_pos.x] = selector_index - 1 + 'a';
					cursor_pos.x++;
				}
			}
		}




		// Move o seletor para a esquerda ou direita
		if (key_hit(KEY_LEFT) || key_hit(KEY_RIGHT))
		{
			selector_pos.x += key_tri_horz();
			selector_pos.x = (selector_pos.x + BUTTON_LENGTH_X) % BUTTON_LENGTH_X;
		}

		// Move o seletor para cima ou baixo
		if (key_hit(KEY_UP) || key_hit(KEY_DOWN))
		{
			selector_pos.y += key_tri_vert();
			selector_pos.y = (selector_pos.y + BUTTON_LENGTH_Y) % BUTTON_LENGTH_Y;
		}

		obj_set_pos(
			// Transforma o índice linear do seletor em coordenadas bidimensionais
			// Limita o seletor dentro das bordas dos botões
			obj_selector,
			BUTTON_OFFSET_X + BUTTON_SIZE_X * selector_pos.x,
			BUTTON_OFFSET_Y + BUTTON_SIZE_Y * selector_pos.y
		);

		obj_set_pos(
			// Transforma o índice linear do seletor em coordenadas bidimensionais
			// Limita o seletor dentro das bordas dos botões
			obj_cursor,
			LETTER_OFFSET_X + LETTER_SIZE_X * cursor_pos.x,
			LETTER_OFFSET_Y + LETTER_SIZE_Y * cursor_pos.y
		);



		update_letters(obj_letters, (char *)letter_buffer, (wordlePalBank_t *)letter_pal_buffer);



		VBlankIntrWait();
		obj_copy(obj_mem, obj_buffer, 128);
	}

	return 0;
}










// Retorna o índice do tile que contém o símbolo index
static int get_sprite_id(int index)
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

	return index * 2 + 32 * (index / 16);
}





static void update_letters(OBJ_ATTR *obj_letters, char *letter_buffer, wordlePalBank_t *letter_pal_buffer)
{
	// Atualiza os ids dos sprites a partir do buffer
	for (int i = 0; i < NUM_LETTERS; i++)
	{
		int letter_index = 0;

		if (letter_buffer[i] != SYMBOL_EMPTY)
			letter_index = letter_buffer[i] + 1 - 'a';

		int id = get_sprite_id(letter_index);

		BFN_SET(obj_letters[i].attr2, id, ATTR2_ID);
		BFN_SET(obj_letters[i].attr2, letter_pal_buffer[i], ATTR2_PALBANK);
	}
}





static void wordle_compare(wordlePalBank_t *letter_pal_buffer, char *solution, char* word)
{
	unsigned char contagem[NUM_POSSIBLE_LETTERS] = {0};

	// Salva a quantidade de letras
    for (int i = 0; i < WORD_LENGTH; i++) {
        contagem[solution[i] - 'a']++;
    }

    if (strncmp(solution, word, WORD_LENGTH) == 0) {
		// A palavra está correta TODO
        return;
    } else {
        for (int i = 0; i < WORD_LENGTH; i++) {
            if (word[i] == solution[i]) {
				// Se a letra for igual está correto
                letter_pal_buffer[i] = PAL_CORRECT;
                contagem[word[i] - 'a']--;
            } else if (contagem[word[i] - 'a'] > 0) {
				// Se a letra for diferente mas ela existe na solução está perto
                letter_pal_buffer[i] = PAL_CLOSE;
                contagem[word[i] - 'a']--;
            } else {
				// Senão a palavra não está na solução
                letter_pal_buffer[i] = PAL_INCORRECT;
            }
        }
    }
}