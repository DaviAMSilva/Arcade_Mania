#include <stdlib.h>
#include <string.h>

#include <tonc.h>

#include <general.h>
#include <game_wordle_words.h>

#include <data/SP_Wordle.h>









#define WORD_LENGTH		5
#define NUM_ATTEMPTS	6
#define NUM_ALPHABET	26
#define NUM_PALETTES	6



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

#define RESULT_SIZE_X	16
#define RESULT_SIZE_Y	16
#define RESULT_OFFSET_X	24
#define RESULT_OFFSET_Y	128
#define RESULT_LENGTH_X	WORD_LENGTH
#define RESULT_LENGTH_Y	1



#define NUM_BUTTONS	BUTTON_LENGTH_X * BUTTON_LENGTH_Y
#define NUM_LETTERS	WORD_LENGTH * NUM_ATTEMPTS
#define NUM_RESULTS	WORD_LENGTH



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
	PAL_RESULT,
} wordlePalBank_t;

typedef enum WordleSpecialSymbol
{
	SYMBOL_EMPTY = 0,
	SYMBOL_BACKSPACE = 1 + NUM_ALPHABET,
	SYMBOL_ENTER,
	SYMBOL_SELECTOR,
} wordleSpecialSymbol_t;










typedef struct LetterInfo
{
	char letter;
	u8 pal;
} letterInfo_t;










static int get_sprite_id(int index);
static void update_letters(OBJ_ATTR *obj_letters, letterInfo_t *letter_info);
static void update_buttons(OBJ_ATTR *obj_buttons, letterInfo_t *letter_info, char *solution);
static int wordle_compare(letterInfo_t *word, char *solution);
static bool is_word_valid(letterInfo_t *word);











// Serve para guardar as informações dos sprites antes de enviar para tela
// Pode parecer ineficiente mas as vezes é possível não existir tempo suficiente
// para atualizar todos os sprites, então é melhor assim
static OBJ_ATTR obj_buffer[128];
static OBJ_ATTR *obj_letters    = obj_buffer;									// NUM_LETTERS
static OBJ_ATTR *obj_buttons    = obj_buffer + NUM_LETTERS;						// NUM_BUTTONS
static OBJ_ATTR *obj_selector   = obj_buffer + NUM_LETTERS + NUM_BUTTONS;		// 1
static OBJ_ATTR *obj_cursor		= obj_buffer + NUM_LETTERS + NUM_BUTTONS + 1;	// 1
static OBJ_ATTR *obj_results	= obj_buffer + NUM_LETTERS + NUM_BUTTONS + 2;	// NUM_RESULTS










int init_wordle_game(void)
{
	REG_DISPCNT = DCNT_MODE0;

	// Copia as informações necessárias sobre os objetos
	memcpy32(tile_mem_obj, SP_WordleTiles, SP_WordleTilesLen / 4);

	// Ativando os sprites
	REG_DISPCNT = DCNT_OBJ | DCNT_OBJ_2D;

	// Esconde todos os sprites
	oam_init(obj_buffer, 128);










	// Salva as letras junto com suas paletas antes que os sprites sejam atualizados
	// Isso é basicamente fazer um buffer do buffer mas é mais simples do que acessar
	// o valores dos sprites em cada operação
	letterInfo_t letter_info[LETTER_LENGTH_Y][LETTER_LENGTH_X] = { 0 };





	// A palavra que se está tentando achar
	char solution[WORD_LENGTH] = {0};

	// Selecionando a palavra a ser resolvida aleatoriamente da lista de palavras mais comuns
	strncpy(solution, wordle_word_bank[rand() % WORDLE_WORD_BANK_LENGTH], WORD_LENGTH);





	// Essa variável será usada para saber qual letra está selecionada
	POINT selector_pos = {0, 0};

	// Essa variável será usada para saber qual letra será escrita
	POINT cursor_pos = {0, 0};










	// Copiando a paleta várias vezes
	for (int i = 0; i < NUM_PALETTES; i++)
		memcpy32(pal_obj_bank[i], SP_WordlePal, 8); // 8 words = 32 bytes = 16 cores

	pal_obj_bank[PAL_EMPTY][3] = COLOR_BLACK;
	pal_obj_bank[PAL_CORRECT][3] = COLOR_GREEN;
	pal_obj_bank[PAL_CLOSE][3] = COLOR_YELLOW;
	pal_obj_bank[PAL_INCORRECT][3] = COLOR_GREY;
	pal_obj_bank[PAL_BUTTON][3] = COLOR_LIGHTGREY;
	pal_obj_bank[PAL_RESULT][3] = COLOR_BLUE;










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





	// Inicializando os resultados
	for (int i = 0; i < NUM_RESULTS; i++)
	{
		obj_set_attr(
			obj_results + i,
			ATTR0_SQUARE,
			ATTR1_SIZE_16,
			ATTR2_PALBANK(PAL_RESULT) | ATTR2_ID(get_sprite_id(solution[i] - 'a' + 1))
		);

		obj_set_pos(
			// Transforma o índice linear dos resultado em coordenadas bidimensionais
			obj_results + i,
			(i * RESULT_SIZE_X) % (RESULT_LENGTH_X * RESULT_SIZE_X) + RESULT_OFFSET_X,
			RESULT_OFFSET_Y + (i / RESULT_LENGTH_X) * RESULT_SIZE_Y
		);

		obj_hide(obj_results + i);
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





	while(true)
	{
		int correct_count = 0;

		key_poll();





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
					letter_info[cursor_pos.y][cursor_pos.x].letter = SYMBOL_EMPTY;
				}
			}
			else if ((key_hit(KEY_A) && selector_index == SYMBOL_ENTER) || key_hit(KEY_START))
			{
				// Move uma linha para baixo se estiver na última letra
				// Se o cursor estiver no final da palavra e não é a última linha e a palavra é válida então a linha é finalizada
				if (cursor_pos.x == LETTER_LENGTH_X && cursor_pos.y < LETTER_LENGTH_Y)
				{
					if (is_word_valid(letter_info[cursor_pos.y]))
					{
						correct_count = wordle_compare(letter_info[cursor_pos.y], solution);

						update_buttons(obj_buttons, letter_info[cursor_pos.y], solution);

						cursor_pos.x = 0;
						cursor_pos.y++;
					}
				}
			}
			else if (key_hit(KEY_A)) {
				// Adicionar a letra selecionada
				if (cursor_pos.x <= LETTER_LENGTH_X - 1 && cursor_pos.y < LETTER_LENGTH_Y)
				{
					letter_info[cursor_pos.y][cursor_pos.x].letter = selector_index - 1 + 'a';
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
			obj_selector,
			BUTTON_OFFSET_X + BUTTON_SIZE_X * selector_pos.x,
			BUTTON_OFFSET_Y + BUTTON_SIZE_Y * selector_pos.y
		);

		obj_set_pos(
			// Transforma o índice linear do cursor em coordenadas bidimensionais
			obj_cursor,
			LETTER_OFFSET_X + LETTER_SIZE_X * cursor_pos.x,
			LETTER_OFFSET_Y + LETTER_SIZE_Y * cursor_pos.y
		);



		update_letters(obj_letters, (letterInfo_t *)letter_info);





		// Se o jogador acerta tudo ou chegar na última tentativa o jogo acaba
		if (correct_count == NUM_RESULTS || cursor_pos.y == LETTER_LENGTH_Y)
		{
			for (int i = 0; i < NUM_RESULTS; i++)
			{
				obj_unhide(obj_results + i, ATTR0_REG);
			}

			VBlankIntrWait();
			obj_copy(obj_mem, obj_buffer, 128);

			VBlankIntrDelay(120);
			key_wait_till_hit(KEY_ANY);

			return 0; // FIXME: Retornar o número de tentativas
		}



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

	return 2 * (index + 16 * (index / 16));
}





static void update_letters(OBJ_ATTR *obj_letters, letterInfo_t *letter_info)
{
	// Atualiza os ids dos sprites a partir do buffer
	for (int i = 0; i < NUM_LETTERS; i++)
	{
		int letter_index = 0;

		if (letter_info[i].letter != SYMBOL_EMPTY)
			letter_index = letter_info[i].letter + 1 - 'a';

		int id = get_sprite_id(letter_index);

		BFN_SET(obj_letters[i].attr2, id, ATTR2_ID);
		BFN_SET(obj_letters[i].attr2, letter_info[i].pal, ATTR2_PALBANK);
	}
}





static int wordle_compare(letterInfo_t *word, char *solution)
{
	int letter_count[NUM_ALPHABET] = {0};
	int correct_count = 0;

	// Salva a quantidade de letras
    for (int i = 0; i < WORD_LENGTH; i++)
	{
        letter_count[solution[i] - 'a']++;

		if (word[i].letter == solution[i]) {
			// Se a letra for igual está correto
			word[i].pal = PAL_CORRECT;
			letter_count[word[i].letter - 'a']--;
			correct_count++;
		}
    }

	// Esse loop precisa ser separado em dois porque as letras nas posições
	// corretas têm prioridade sobre as que existem mas estão na posição errada
	for (int i = 0; i < WORD_LENGTH; i++) {
		if (word[i].letter != solution[i])
		{
			if (letter_count[word[i].letter - 'a'] > 0) {
				// Se a letra for diferente mas ela existe na solução está perto
				word[i].pal = PAL_CLOSE;
				letter_count[word[i].letter - 'a']--;
			} else {
				// Senão a palavra não está na solução
				word[i].pal = PAL_INCORRECT;
			}
		}
	}

	// Retorna a quantidade de letras corretas existem na palavra
	return correct_count;
}





// Utiliza pesquisa binária para encontrar a palavra mais rapidamente
// Isso só é possível se as palavras estiverem ordenadas
static bool is_word_valid(letterInfo_t *word)
{
	int left = 0;
	int right = WORDLE_WORD_VALID_LENGTH - 1;

	while (left <= right)
	{
		int middle = (left + right) / 2;

		int result = 0;

		for (int i = 0; i < WORD_LENGTH; i++)
		{
			if (word[i].letter > wordle_word_valid[middle][i])
			{
				result = 1;
				break;
			}
			else if (word[i].letter < wordle_word_valid[middle][i])
			{
				result = -1;
				break;
			}
		}

		switch (result)
		{
			case  0: return true;
			case  1: left  = middle + 1; break;
			case -1: right = middle - 1; break;
		}
	}

	return false;
}





static void update_buttons(OBJ_ATTR *obj_buttons, letterInfo_t *letter_info, char *solution)
{
	int letter_count[NUM_ALPHABET] = {0};

	// Salva a quantidade de letras
    for (int i = 0; i < WORD_LENGTH; i++)
	{
        letter_count[solution[i] - 'a']++;

		if (letter_info[i].letter == solution[i]) {
			// Se a letra for igual está correto
			BFN_SET(obj_buttons[letter_info[i].letter - 'a'].attr2, PAL_CORRECT, ATTR2_PALBANK);
		}
    }

	// Esse loop precisa ser separado em dois porque as letras nas posições
	// corretas têm prioridade sobre as que existem mas estão na posição errada
	for (int i = 0; i < WORD_LENGTH; i++) {
		if (letter_info[i].letter != solution[i])
		{
			if (letter_count[letter_info[i].letter - 'a'] > 0) {
				// Se a letra for diferente mas ela existe na solução está perto
				if (BFN_GET(obj_buttons[letter_info[i].letter - 'a'].attr2, ATTR2_PALBANK) != PAL_CORRECT)
					BFN_SET(obj_buttons[letter_info[i].letter - 'a'].attr2, PAL_CLOSE, ATTR2_PALBANK);
			} else {
				// Senão a palavra não está na solução
				BFN_SET(obj_buttons[letter_info[i].letter - 'a'].attr2, PAL_INCORRECT, ATTR2_PALBANK);
			}
		}
	}
}