#include <stdlib.h>
#include <string.h>

#include <tonc.h>

#include <general.h>
#include <game_wordle_words.h>

#include <data/SP_Wordle.h>










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



#define ANIMATION_HIGHLIGHT_DURATION	10
#define ANIMATION_HIGHLIGHT_EXPANSION	0.20f

#define ANIMATION_REVEAL_ZOOM			16
#define ANIMATION_REVEAL_WAIT_SMALL		0
#define ANIMATION_REVEAL_WAIT_BIG		5
#define ANIMATION_REVEAL_FRAMES			1

#define ANIMATION_BOUNCE_SIZE			10
#define ANIMATION_BOUNCE_STEP			2
#define ANIMATION_BOUNCE_WAIT_SMALL		0
#define ANIMATION_BOUNCE_WAIT_BIG		1
#define ANIMATION_BOUNCE_FRAMES			1



#define SELECTOR_SPEED_X	(BUTTON_SIZE_X / 4)
#define SELECTOR_SPEED_Y	(BUTTON_SIZE_Y / 4)



#define WORD_LENGTH		5
#define NUM_ATTEMPTS	6
#define NUM_ALPHABET	26
#define NUM_PALETTES	6
#define NUM_ANIMATIONS	WORD_LENGTH
#define NUM_BUTTONS		(BUTTON_LENGTH_X * BUTTON_LENGTH_Y)
#define NUM_LETTERS		(WORD_LENGTH * NUM_ATTEMPTS)
#define NUM_RESULTS		WORD_LENGTH










typedef enum WordlePalBank
{
	PAL_EMPTY = 0,
	PAL_CORRECT,
	PAL_CLOSE,
	PAL_INCORRECT,
	PAL_BUTTON,
	PAL_RESULT,
	PAL_SELECTOR,
} wordlePalBank_t;

typedef enum WordleSpecialSymbol
{
	SYMBOL_EMPTY = 0,
	SYMBOL_BACKSPACE = 1 + NUM_ALPHABET,
	SYMBOL_ENTER,
	SYMBOL_SELECTOR,
} wordleSpecialSymbol_t;










typedef struct SymbolInfo
{
	char letter;
	u8 pal;
} symbolInfo_t;

typedef struct HighlightAnimationInfo
{
	int timer;
	bool is_active;
} highlightAnimationInfo_t;










static int get_object_id				(int index);
static void update_symbols				(OBJ_ATTR *objs, symbolInfo_t *symbol_info, int num_symbols);
static int wordle_compare				(symbolInfo_t *letter_info, symbolInfo_t *button_info, char *solution);
static bool is_word_valid				(symbolInfo_t *letter_info);
static void start_highlight_animation	(highlightAnimationInfo_t *animation_info, OBJ_ATTR *obj_animation_reg, int duration, int id, wordlePalBank_t pal, int x, int y);
static void stop_highlight_animation	(highlightAnimationInfo_t *animation_info, OBJ_ATTR *obj_animation_reg);
static bool handle_highlight_animation	(highlightAnimationInfo_t *animation_info, OBJ_ATTR *obj_animations_reg, OBJ_AFFINE *obj_animations_aff);
static void handle_reveal_animation		(int y, OBJ_ATTR *obj_animations_reg, OBJ_AFFINE *obj_animations_aff, OBJ_ATTR *obj_letters, char *solution, symbolInfo_t *letter_info);
static void handle_bounce_animation		(int y, OBJ_ATTR *obj_animations_reg, OBJ_AFFINE *obj_animations_aff, OBJ_ATTR *obj_letters, char *solution);










// Serve para guardar as informações dos objetos antes de enviar para tela
// Pode parecer ineficiente mas as vezes é possível não existir tempo suficiente
// para atualizar todos os objetos, então é melhor assim
static OBJ_ATTR obj_buffer[128];									// 128 objetos regulares
static OBJ_AFFINE *const obj_aff_buffer = (OBJ_AFFINE *)obj_buffer;	//  32 objetos afins










int init_wordle_game(void)
{
	REG_DISPCNT = DCNT_MODE0;

	// Copia as informações necessárias sobre os objetos
	memcpy32(tile_mem_obj, SP_WordleTiles, SP_WordleTilesLen / 4);
	memcpy32(pal_obj_mem, SP_WordlePal, SP_WordlePalLen / 4);

	// Ativando os objetos
	REG_DISPCNT = DCNT_OBJ | DCNT_OBJ_2D;

	// Colorindo o fundo
	pal_bg_mem[0] = RGB15(2,  2,  2); // #121213

	// Inicializa todos os objetos
	oam_init(obj_buffer, 128);





	// Objetos regulares
	OBJ_ATTR *const obj_letters			= obj_buffer;												// NUM_LETTERS
	OBJ_ATTR *const obj_buttons			= obj_buffer + NUM_LETTERS;									// NUM_BUTTONS
	OBJ_ATTR *const obj_selector		= obj_buffer + NUM_LETTERS + NUM_BUTTONS;					// 1
	OBJ_ATTR *const obj_results			= obj_buffer + NUM_LETTERS + NUM_BUTTONS + 1;				// NUM_RESULTS
	OBJ_ATTR *const obj_animations_reg	= obj_buffer + NUM_LETTERS + NUM_BUTTONS + 1 + NUM_RESULTS;	// NUM_ANIMATIONS

	// Objetos afins
	// Ambos os objetos regulares quantos os afins estão presentes na mesma
	// posição de memória em obj_mem, portanto usa-se o mesmo buffer
	OBJ_AFFINE *const obj_animations_aff = obj_aff_buffer; // NUM_ANIMATIONS










	// Salva as letras junto com suas paletas antes que os objetos sejam atualizados
	// Isso é basicamente fazer um buffer do buffer mas é mais simples do que acessar
	// o valores dos objetos em cada operação
	symbolInfo_t letter_info[LETTER_LENGTH_Y][LETTER_LENGTH_X] = { 0 };
	symbolInfo_t button_info[BUTTON_LENGTH_Y][BUTTON_LENGTH_X] = { 0 };

	// Salva as informações sobre as animações
	highlightAnimationInfo_t animation_info[NUM_ANIMATIONS] = { 0 };





	// A palavra que se está tentando achar
	char solution[WORD_LENGTH] = {0};

	// Selecionando a palavra a ser resolvida aleatoriamente da lista de palavras mais comuns
	int solution_index = rand() % WORDLE_WORD_BANK_LENGTH;
	for (int i = 0; i < WORD_LENGTH; i++)
		solution[i] = wordle_word_bank[solution_index][i];





	// Essa variável será usada para saber qual letra está selecionada
	POINT selector_pos = {0, 0};

	// Essa variável será usada para saber qual letra será escrita
	POINT cursor_pos = {0, 0};

	// Essas variáveis servem para animar o seletor de um lugar para outro
	POINT selector_current_pos		= {	BUTTON_OFFSET_X, BUTTON_OFFSET_Y };
	POINT selector_destination_pos	= {	BUTTON_OFFSET_X, BUTTON_OFFSET_Y };










	// Inicializando as letras das palavras
	for (int j = 0; j < LETTER_LENGTH_Y; j++)
	{
		for (int i = 0; i < LETTER_LENGTH_X; i++)
		{
			int index = (i + j * LETTER_LENGTH_X);

			obj_set_attr(
				obj_letters + index,
				ATTR0_SQUARE | ATTR0_REG,
				ATTR1_SIZE_16,
				ATTR2_PALBANK(PAL_EMPTY) | ATTR2_ID(0) | ATTR2_PRIO(1)
			);

			obj_set_pos(
				obj_letters + index,
				(i * LETTER_SIZE_X) + LETTER_OFFSET_X,
				(j * LETTER_SIZE_Y) + LETTER_OFFSET_Y
			);

			letter_info[j][i].letter = SYMBOL_EMPTY;
			letter_info[j][i].pal = PAL_EMPTY;
		}
	}





	// Inicializando os botões
	for (int j = 0; j < BUTTON_LENGTH_Y; j++)
	{
		for (int i = 0; i < BUTTON_LENGTH_X; i++)
		{
			int index = (i + j * BUTTON_LENGTH_X);

			obj_set_attr(
				obj_buttons + index,
				ATTR0_SQUARE | ATTR0_REG,
				ATTR1_SIZE_16,
				ATTR2_PALBANK(PAL_BUTTON) | ATTR2_ID(get_object_id(i + 1)) | ATTR2_PRIO(1)
			);

			obj_set_pos(
				obj_buttons + index,
				(i * BUTTON_SIZE_X) + BUTTON_OFFSET_X,
				(j * BUTTON_SIZE_Y) + BUTTON_OFFSET_Y
			);

			button_info[j][i].letter = index + 'a';
			button_info[j][i].pal = PAL_BUTTON;
		}
	}
	update_symbols(obj_buttons, (symbolInfo_t *)button_info, NUM_BUTTONS);




	// Inicializando o seletor
	obj_set_attr(
		obj_selector,
		ATTR0_Y(BUTTON_OFFSET_X) | ATTR0_SQUARE | ATTR0_REG,
		ATTR1_X(BUTTON_OFFSET_Y) | ATTR1_SIZE_16,
		ATTR2_PALBANK(PAL_SELECTOR) | ATTR2_ID(get_object_id(SYMBOL_SELECTOR))
	);





	// Inicializando os resultados
	for (int i = 0; i < NUM_RESULTS; i++)
	{
		obj_set_attr(
			obj_results + i,
			ATTR0_SQUARE | ATTR0_HIDE,
			ATTR1_SIZE_16,
			ATTR2_PALBANK(PAL_RESULT) | ATTR2_PRIO(1) | ATTR2_ID(get_object_id(("troll")[i] - 'a' + 1)) // trolled
		);

		obj_set_pos(
			// Transforma o índice linear dos resultado em coordenadas bidimensionais
			obj_results + i,
			(i * RESULT_SIZE_X) % (RESULT_LENGTH_X * RESULT_SIZE_X) + RESULT_OFFSET_X,
			RESULT_OFFSET_Y + (i / RESULT_LENGTH_X) * RESULT_SIZE_Y
		);
	}





	// Inicializando as animações
	for (int i = 0; i < NUM_ANIMATIONS; i++)
	{
		obj_set_attr(
			obj_animations_reg + i,
			ATTR0_SQUARE | ATTR0_HIDE,
			ATTR1_SIZE_16 | ATTR1_AFF_ID(i),
			ATTR2_PALBANK(PAL_EMPTY) | ATTR2_ID(0)
		);
	}










	// Loop principal do jogo
	int correct_count = 0;
	while (true)
	{
		bool is_animating = false;





		// Atualiza todas as animações de destaque
		is_animating = handle_highlight_animation(animation_info, obj_animations_reg, obj_animations_aff);



		// Salva os estados das teclas
		key_poll();
		


		// Define a posição de destino do cursor
		if (key_hit(KEY_DIR))
		{
			// Move o seletor para a esquerda ou direita
			if (key_hit(KEY_LEFT | KEY_RIGHT))
			{
				selector_pos.x += key_tri_horz();
				selector_pos.x = CLAMP(selector_pos.x, 0, BUTTON_LENGTH_X);
				selector_destination_pos.x = BUTTON_OFFSET_X + selector_pos.x * BUTTON_SIZE_X;
			}

			// Move o seletor para cima ou baixo
			if (key_hit(KEY_UP | KEY_DOWN))
			{
				selector_pos.y += key_tri_vert();
				selector_pos.y = CLAMP(selector_pos.y, 0, BUTTON_LENGTH_Y);
				selector_destination_pos.y = BUTTON_OFFSET_Y + selector_pos.y * BUTTON_SIZE_Y;
			}
		}



		// Move o seletor da posição atual para a posição destino
		selector_current_pos.x += SELECTOR_SPEED_X * SGN3(selector_destination_pos.x - selector_current_pos.x);
		selector_current_pos.y += SELECTOR_SPEED_Y * SGN3(selector_destination_pos.y - selector_current_pos.y);

		obj_set_pos(
			obj_selector,
			selector_current_pos.x,
			selector_current_pos.y
		);





		// Comportamento quando qualquer botão é pressionado
		if (key_hit(KEY_FIRE | KEY_SPECIAL))
		{
			int selector_index = selector_pos.x + selector_pos.y * BUTTON_LENGTH_X;

			if ((key_hit(KEY_A) && selector_index + 1 == SYMBOL_BACKSPACE) || key_hit(KEY_B))
			{
				// Remover a letra se o cursor não estiver na posição (0, 0)
				if (cursor_pos.x > 0 && cursor_pos.y < LETTER_LENGTH_Y)
				{
					cursor_pos.x--;
					letter_info[cursor_pos.y][cursor_pos.x].letter = SYMBOL_EMPTY;

					// Finaliza a animação de destaque
					stop_highlight_animation(animation_info + cursor_pos.x, obj_animations_reg + cursor_pos.x);
				}
			}
			else if ((key_hit(KEY_A) && selector_index + 1 == SYMBOL_ENTER) || key_hit(KEY_START))
			{
				// Move uma linha para baixo se estiver na última letra
				// Se o cursor estiver no final da palavra e não é a última linha e a palavra é válida então a linha é finalizada
				if (cursor_pos.x == LETTER_LENGTH_X && cursor_pos.y < LETTER_LENGTH_Y)
				{
					// Cancela todas as animações
					for (int i = 0; i < NUM_ANIMATIONS; i++)
						stop_highlight_animation(animation_info + i, obj_animations_reg + i);

					if (is_word_valid(letter_info[cursor_pos.y]))
					{
						correct_count = wordle_compare(letter_info[cursor_pos.y], (symbolInfo_t *)button_info, solution);

						// Inicia a animação de revelar a palavra
						handle_reveal_animation(cursor_pos.y, obj_animations_reg, obj_animations_aff, obj_letters, solution, (symbolInfo_t *)letter_info[cursor_pos.y]);

						update_symbols(obj_letters, (symbolInfo_t *)letter_info, NUM_LETTERS);
						update_symbols(obj_buttons, (symbolInfo_t *)button_info, NUM_BUTTONS);



						// Se a palavra estiver correta iniciar a animação de pulo da palavra correta
						if (correct_count == WORD_LENGTH)
						{
							VBlankIntrDelay(15);
							handle_bounce_animation(cursor_pos.y, obj_animations_reg, obj_animations_aff, obj_letters, solution);
						}



						cursor_pos.x = 0;
						cursor_pos.y++;



						// Se as tentativas acabarem iniciar a animação de pulo da solução
						if (cursor_pos.y == NUM_ATTEMPTS && correct_count != WORD_LENGTH)
						{
							VBlankIntrDelay(15);
							handle_bounce_animation(-1, obj_animations_reg, obj_animations_aff, obj_letters, solution);
						}
					}
					else
					{
						// Animação de destaque, para que o usuário saiba que a palavra não é válida
						for (int i = 0; i < NUM_ANIMATIONS; i++)
						{
							start_highlight_animation(
								animation_info + i,
								obj_animations_reg + i,
								ANIMATION_HIGHLIGHT_DURATION,
								get_object_id(letter_info[cursor_pos.y][i].letter - 'a' + 1),
								PAL_EMPTY,
								LETTER_OFFSET_X + i * LETTER_SIZE_X - LETTER_SIZE_X / 2,
								LETTER_OFFSET_Y + cursor_pos.y * LETTER_SIZE_Y - LETTER_SIZE_Y / 2
							);
						}
					}
				}
				else
				{
					// Animação de destaque, para que o usuário saiba que a palavra não é válida
					for (int i = 0; i < cursor_pos.x; i++)
					{
						start_highlight_animation(
							animation_info + i,
							obj_animations_reg + i,
							ANIMATION_HIGHLIGHT_DURATION,
							get_object_id(letter_info[cursor_pos.y][i].letter - 'a' + 1),
							PAL_EMPTY,
							LETTER_OFFSET_X + i * LETTER_SIZE_X - LETTER_SIZE_X / 2,
							LETTER_OFFSET_Y + cursor_pos.y * LETTER_SIZE_Y - LETTER_SIZE_Y / 2
						);
					}
				}
			}
			else if (key_hit(KEY_SELECT))
			{
				// Apaga todas as letras da linha atual
				if (cursor_pos.x > 0 && cursor_pos.y < LETTER_LENGTH_Y)
				{
					for (int i = 0; i < LETTER_LENGTH_X; i++)
					{
						letter_info[cursor_pos.y][i].letter = SYMBOL_EMPTY;

						// Finaliza a animação de destaque
						stop_highlight_animation(animation_info + i, obj_animations_reg + i);
					}

					cursor_pos.x = 0;
				}
			}
			else if (key_hit(KEY_A)) {
				// Adicionar a letra selecionada
				if (cursor_pos.x <= LETTER_LENGTH_X - 1 && cursor_pos.y < LETTER_LENGTH_Y)
				{
					// Animação da letra aumentar e diminuir
					start_highlight_animation(
						animation_info + cursor_pos.x,
						obj_animations_reg + cursor_pos.x,
						ANIMATION_HIGHLIGHT_DURATION,
						get_object_id(selector_index + 1),
						PAL_EMPTY,
						LETTER_OFFSET_X + cursor_pos.x * LETTER_SIZE_X - LETTER_SIZE_X / 2,
						LETTER_OFFSET_Y + cursor_pos.y * LETTER_SIZE_Y - LETTER_SIZE_Y / 2
					);

					letter_info[cursor_pos.y][cursor_pos.x].letter = selector_index + 'a';
					cursor_pos.x++;
				}
			}



			// Atualiza o buffer de objetos de acordo com as informações das letras
			update_symbols(obj_letters, (symbolInfo_t *)letter_info, NUM_LETTERS);
		}










		// Se o jogador acerta tudo ou chegar na última tentativa o jogo acaba
		if ((correct_count == WORD_LENGTH || cursor_pos.y == LETTER_LENGTH_Y) && !is_animating)
		{
			// Mostrar o resultado apenas se o jogador não tiver acertado todas as letras
			if (cursor_pos.y == LETTER_LENGTH_Y && correct_count != WORD_LENGTH)
			{
				for (int i = 0; i < NUM_RESULTS; i++)
				{
					BFN_SET(obj_results[i].attr2, get_object_id(solution[i] - 'a' + 1), ATTR2_ID);
					obj_unhide(obj_results + i, ATTR0_REG);
				}
			}

			VBlankIntrWait();
			obj_copy(obj_mem, obj_buffer, 128);

			VBlankIntrDelay(60);
			key_wait_till_hit(KEY_ANY);

			fade_to_black();

			// Retorna 0 se perdeu, senão retorna a quantidade de tentativas
			return correct_count == WORD_LENGTH ? cursor_pos.y : 0;
		}



		VBlankIntrWait();
		oam_copy(obj_mem, obj_buffer, 128);
	}










	// Não deve chegar aqui
	return 0;
}










// Retorna o índice do tile que contém o símbolo index
static int get_object_id(int index)
{
	// os objetos tem um tamanho de 16x16px (2x2 tiles), mas a imagem de tiles é composta de 16x16 tiles
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










static void update_symbols(OBJ_ATTR *objs, symbolInfo_t *symbol_info, int num_symbols)
{
	// Atualiza os ids dos objetos a partir do buffer
	for (int i = 0; i < num_symbols; i++)
	{
		int letter_index = 0;

		if (symbol_info[i].letter != SYMBOL_EMPTY)
			letter_index = symbol_info[i].letter + 1 - 'a';

		BFN_SET(objs[i].attr2, get_object_id(letter_index), ATTR2_ID);
		BFN_SET(objs[i].attr2, symbol_info[i].pal, ATTR2_PALBANK);
	}
}










static int wordle_compare(symbolInfo_t *letter_info, symbolInfo_t *button_info, char *solution)
{
	int letter_count[NUM_ALPHABET] = {0};
	int correct_count = 0;

	// Salva a quantidade de letras
    for (int i = 0; i < WORD_LENGTH; i++)
	{
		const int index = letter_info[i].letter - 'a';

        letter_count[solution[i] - 'a']++;

		if (letter_info[i].letter == solution[i]) {
			// Se a letra for igual está correto
			letter_info[i].pal = PAL_CORRECT;
			button_info[index].pal = PAL_CORRECT;
			letter_count[index]--;
			correct_count++;
		}
    }

	// Esse loop precisa ser separado em dois porque as letras nas posições
	// corretas têm prioridade sobre as que existem mas estão na posição errada
	for (int i = 0; i < WORD_LENGTH; i++) {
		if (letter_info[i].letter != solution[i])
		{
			const int index = letter_info[i].letter - 'a';

			if (letter_count[index] > 0) {
				// Se a letra for diferente mas ela existe na solução está perto
				letter_info[i].pal = PAL_CLOSE;
				letter_count[index]--;

				// Apenas atualizar o botão se ele não estiver correto
				if (button_info[index].pal != PAL_CORRECT)
					button_info[index].pal = PAL_CLOSE;
			} else {
				// Senão a palavra não está na solução
				letter_info[i].pal = PAL_INCORRECT;

				// Apenas atualizar o botão se ele não estiver correto ou perto
				if (button_info[index].pal != PAL_CORRECT && button_info[index].pal != PAL_CLOSE)
					button_info[index].pal = PAL_INCORRECT;
			}
		}
	}

	// Retorna a quantidade de letras corretas existem na palavra
	return correct_count;
}










// Utiliza pesquisa binária para encontrar a palavra mais rapidamente
// Isso só é possível se as palavras estiverem ordenadas
static bool is_word_valid(symbolInfo_t *letter_info)
{
	int left = 0;
	int right = WORDLE_WORD_VALID_LENGTH - 1;

	while (left <= right)
	{
		int middle = (left + right) / 2;

		int result = 0;

		for (int i = 0; i < WORD_LENGTH; i++)
		{
			if (letter_info[i].letter > wordle_word_valid[middle][i])
			{
				result = 1;
				break;
			}
			else if (letter_info[i].letter < wordle_word_valid[middle][i])
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










// Define os valores iniciais para que um objeto comece a ser animado
static void start_highlight_animation(highlightAnimationInfo_t *animation_info, OBJ_ATTR *obj_animation_reg, int duration, int id, wordlePalBank_t pal, int x, int y)
{
	animation_info->is_active = true;
	animation_info->timer = duration;

	obj_unhide(obj_animation_reg, ATTR0_AFF_DBL);
	obj_set_pos(obj_animation_reg, x, y);

	BFN_SET(obj_animation_reg[0].attr2, id, ATTR2_ID);
	BFN_SET(obj_animation_reg[0].attr2, pal, ATTR2_PALBANK);
}



// Interrompe a animação de um objeto
static void stop_highlight_animation(highlightAnimationInfo_t *animation_info, OBJ_ATTR *obj_animation_reg)
{
	animation_info->is_active = false;
	animation_info->timer = -1;

	obj_hide(obj_animation_reg);
}



// Animação de destacar uma letra
static bool handle_highlight_animation(highlightAnimationInfo_t *animation_info, OBJ_ATTR *obj_animations_reg, OBJ_AFFINE *obj_animations_aff)
{
	// Variáveis constantes usadas para calcular a animação de destaque
	const FIXED max_offset = float2fx(ANIMATION_HIGHLIGHT_EXPANSION);
	const FIXED half_duration = int2fx(ANIMATION_HIGHLIGHT_DURATION / 2);



	bool is_animating = false;

	for (int i = 0; i < NUM_ANIMATIONS; i++)
	{
		// Pula as animações que não estão sendo executadas
		if (!animation_info[i].is_active)
			continue;

		// Se o timer é menor que zero parar a animação
		if (animation_info[i].timer < 0)
		{
			stop_highlight_animation(animation_info + i, obj_animations_reg + i);
			continue;
		}

		is_animating = true;

		// A matriz de um objeto afim utiliza ponto fixo .8 para representar os seus valores,
		// assim é preciso converter os valores para esse formato
		// Além disso é melhor fazer a conversão no início, pois o GBA não tem um chip
		// de aritmética de ponto flutuante, tornando o ponto fixo a opção mais rápida
		const FIXED timer = int2fx(animation_info[i].timer);
		FIXED value = FIX_ONE;

		if (animation_info[i].timer > ANIMATION_HIGHLIGHT_DURATION / 2)
		{
			// Animando de 1.0f para 1.20f
			// 1 + max_offset * (2 - timer / half_duration)
			value = fxadd(fxmul(fxsub(2 * FIX_ONE, fxdiv(timer, half_duration)), max_offset), FIX_ONE);
		}
		else
		{
			// Animando de 1.20f para 1.0f
			// 1 + max_offset * (timer / half_duration)
			value = fxadd(fxmul(fxdiv(timer, half_duration), max_offset), FIX_ONE);
		}

		// Normalmente 0.5f mostraria um objeto com o dobro de tamanho do original
		// mas é possível inverter esse valor para que as proporções sejam fáceis de entender
		obj_aff_scale_inv(obj_animations_aff + i, value, value);

		animation_info[i].timer--;
	}

	return is_animating;
}










// Animação de revelar a palavra
static void handle_reveal_animation(int y, OBJ_ATTR *obj_animations_reg, OBJ_AFFINE *obj_animations_aff, OBJ_ATTR *obj_letters, char *solution, symbolInfo_t *letter_info)
{
	// Escondendo as letras
	obj_hide_multi(obj_letters + LETTER_LENGTH_X * y, WORD_LENGTH);

	// Exibindo os objetos de animação
	obj_unhide_multi(obj_animations_reg, ATTR0_AFF, NUM_ANIMATIONS);

	// Posicionando e definindo as informações sobre os objetos de animação
	for (int i = 0; i < NUM_ANIMATIONS; i++)
	{
		obj_set_pos(obj_animations_reg + i, LETTER_OFFSET_X + i * LETTER_SIZE_X, LETTER_OFFSET_Y + y * LETTER_SIZE_Y);
		BFN_SET(obj_animations_reg[i].attr2, get_object_id(letter_info[i].letter - 'a' + 1), ATTR2_ID);
		BFN_SET(obj_animations_reg[i].attr2, PAL_EMPTY, ATTR2_PALBANK);
		obj_aff_identity(obj_animations_aff + i);
	}

	VBlankIntrWait();

	oam_copy(oam_mem, obj_buffer, 128);

	VBlankIntrDelay(ANIMATION_REVEAL_WAIT_BIG);



	for (int i = 0; i < NUM_ANIMATIONS; i++)
	{
		// Diminuindo a letra
		for (int j = 1; j <= ANIMATION_REVEAL_ZOOM; j++)
		{
			obj_aff_scale(obj_animations_aff + i, FIX_ONE, int2fx(j));
			VBlankIntrDelay(ANIMATION_REVEAL_FRAMES);
			obj_copy(obj_mem, obj_buffer, 128);
			obj_aff_copy(obj_aff_mem, obj_aff_buffer, 32);
		}

		// Modificando a cor da letra do objeto
		BFN_SET(obj_animations_reg[i].attr2, letter_info[i].pal, ATTR2_PALBANK);
		VBlankIntrDelay(ANIMATION_REVEAL_WAIT_SMALL);

		// Aumentando a letra
		for (int j = ANIMATION_REVEAL_ZOOM; j >= 1; j--)
		{
			obj_aff_scale(obj_animations_aff + i, FIX_ONE, int2fx(j));
			VBlankIntrDelay(ANIMATION_REVEAL_FRAMES);
			obj_copy(obj_mem, obj_buffer, 128);
			obj_aff_copy(obj_aff_mem, obj_aff_buffer, 32);
		}

		VBlankIntrDelay(ANIMATION_REVEAL_WAIT_BIG);
	}

	// Esconde os objetos de animação
	obj_hide_multi(obj_animations_reg, NUM_ANIMATIONS);

	// Exibindo as letras
	obj_unhide_multi(obj_letters + WORD_LENGTH * y, ATTR0_REG, WORD_LENGTH);
}



static void handle_bounce_animation(int y, OBJ_ATTR *obj_animations_reg, OBJ_AFFINE *obj_animations_aff, OBJ_ATTR *obj_letters, char *solution)
{
	// Escondendo as letras se for o caso
	if (y >= 0)
		obj_hide_multi(obj_letters + LETTER_LENGTH_X * y, WORD_LENGTH);

	// Exibindo os objetos de animação
	obj_unhide_multi(obj_animations_reg, ATTR0_AFF, NUM_ANIMATIONS);

	// As posições dos objetos de animação são definidas no início da função
	POINT positions[NUM_ANIMATIONS] = { 0 };

	// Posicionando e definindo as informações sobre os objetos de animação
	for (int i = 0; i < NUM_ANIMATIONS; i++)
	{
		if (y >= 0)
		{
			// Posição dentro das letras
			positions[i].x = LETTER_OFFSET_X + i * LETTER_SIZE_X;
			positions[i].y = LETTER_OFFSET_Y + y * LETTER_SIZE_Y;
		}
		else
		{
			// Posição do resultado
			positions[i].x = RESULT_OFFSET_X + i * RESULT_SIZE_X;
			positions[i].y = RESULT_OFFSET_Y;
		}

		// Posição, id e matriz identidade
		obj_set_pos(obj_animations_reg + i, positions[i].x, positions[i].y);
		BFN_SET(obj_animations_reg[i].attr2, get_object_id(solution[i] - 'a' + 1), ATTR2_ID);
		obj_aff_identity(obj_animations_aff + i);

		if (y >= 0)
			BFN_SET(obj_animations_reg[i].attr2, PAL_CORRECT, ATTR2_PALBANK);
		else
			BFN_SET(obj_animations_reg[i].attr2, PAL_RESULT, ATTR2_PALBANK);
	}

	VBlankIntrWait();

	oam_copy(oam_mem, obj_buffer, 128);



	VBlankIntrDelay(ANIMATION_BOUNCE_WAIT_BIG);

	for (int i = 0; i < NUM_ANIMATIONS; i++)
	{
		for (int j = 0; j < ANIMATION_BOUNCE_SIZE; j += ANIMATION_BOUNCE_STEP)
		{
			// Posição do objeto de animação
			obj_set_pos(obj_animations_reg + i, positions[i].x, --positions[i].y);
			VBlankIntrDelay(ANIMATION_BOUNCE_FRAMES);
			obj_copy(obj_mem, obj_buffer, 128);
		}

		for (int j = 0; j < ANIMATION_BOUNCE_SIZE; j += ANIMATION_BOUNCE_STEP)
		{
			// Posição do objeto de animação
			obj_set_pos(obj_animations_reg + i, positions[i].x, ++positions[i].y);
			VBlankIntrDelay(ANIMATION_BOUNCE_FRAMES);
			obj_copy(obj_mem, obj_buffer, 128);
		}

		VBlankIntrDelay(ANIMATION_BOUNCE_WAIT_BIG);
	}

	// Esconde os objetos de animação
	obj_hide_multi(obj_animations_reg, NUM_ANIMATIONS);

	// Exibindo as letras
	if (y >= 0)
		obj_unhide_multi(obj_letters + LETTER_LENGTH_X * y, ATTR0_REG, WORD_LENGTH);
}