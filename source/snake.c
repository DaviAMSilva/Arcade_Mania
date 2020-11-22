#include <stdlib.h>
#include <string.h>

#include <tonc.h>

#include <data/TL_Snake.h>










#define MAX_SNAKE_LENGTH	(SCR_WT * SCR_HT)
#define MAX_POOS_LENGTH		20

#define BACKGROUND_WIDTH	4
#define BACKGROUND_HEIGHT	2

#define MOD_WIDTH(x) 	(((x) + SCR_WT) % SCR_WT)
#define MOD_HEIGHT(x) 	(((x) + SCR_HT) % SCR_HT)

#define SNAKE_PALETTE_INDEX		129
#define SNAKE_PALETTE_PTR		(pal_bg_mem + SNAKE_PALETTE_INDEX)
#define SNAKE_PALETTE_LENGTH	3

#define SNAKE_SEED 15

#define FRAME_NORMAL	10
#define FRAME_FAST		5

#define SPECIAL_TIMER	144	// Dura 12 segundos
#define FOOD_TIMER		36	// Dura 6 segundos



#define asset_layer			se_mat[8]
#define snake_layer			se_mat[9]
#define background_layer	se_mat[10]



static const COLOR rainbow_colors[]	= { CLR_RED, CLR_ORANGE, CLR_YELLOW, CLR_LIME, CLR_CYAN, CLR_BLUE, CLR_PURPLE, CLR_FUCHSIA };
static const COLOR snake_colors[]	= { 576U, 825U, 5509U };








typedef enum SnakeSE // Tiles do jogo
{
	SE_BLANK		= 0,
	SE_BACKGROUND	= 35,
	SE_FRUIT1		= 29,
	SE_FRUIT2		= 30,
	SE_FRUIT3		= 31,
	SE_BUNNY1		= 26,
	SE_BUNNY2		= 27,
	SE_BUNNY3		= 28,
	SE_POO1			= 32,
	SE_POO2			= 33,
	SE_SPECIAL		= 34
} snakeSE_t;

typedef enum SnakeSE_D // Direções da cobra
{
	FROM_UP		= 0,
	FROM_RIGHT	= 5,
	FROM_DOWN	= 10,
	FROM_LEFT	= 15,
	FROM_TAIL	= 20,

	TO_UP		= 1,
	TO_RIGHT	= 2,
	TO_DOWN		= 3,
	TO_LEFT		= 4,
	TO_HEAD		= 5,
} snakeSE_D_t;










typedef struct Part
{
	POINT pos; // x e y
	u16 from, to;
} ALIGN4 part_t;

typedef struct Asset
{
	POINT pos; // x e y
	u16 se;
} ALIGN4 asset_t;

typedef struct Snake
{
	/**
	 * Equivale a 30 * 20 = 600 partes (uma para cada tile da tela)
	 * Embora isso seja bastante ineficiente é a solução mais simples,
	 * já que o ideal seria usar uma lista encadeada, mas alocação dinâmica
	 * no GBA é um pouco complexa
	 * Apesar disso a memória é mais do que suficiente para este jogo e um
	 * dos benefícios é não precisar liberar a memória
	 */
	part_t body[MAX_SNAKE_LENGTH];
	int length;
} ALIGN4 snake_t;

typedef struct Poos
{
	asset_t list[MAX_POOS_LENGTH];
	int index;
} ALIGN4 poos_t;




















static void create_snake	(snake_t *snake);
static void grow_snake		(snake_t *snake);
static void draw_snake		(snake_t *snake);
static void move_snake		(snake_t *snake, POINT dir);
static bool hit_asset		(snake_t *snake, asset_t *asset);

static asset_t new_fruit	(void);

static void move_bunny		(asset_t *bunny);
static asset_t new_bunny	(void);

static asset_t new_special	(void);

static void draw_asset		(asset_t *asset);
static bool hit_snake		(snake_t *snake);
static void copy_background	(void);

static void create_poos		(poos_t *poos);
static void new_poo			(poos_t *poos, s16 x, s16 y);
static bool hit_poos		(snake_t *snake, poos_t *poos);
static void draw_poos		(poos_t *poos);










// Guarda a pontuação
int internal_score = 0;










int init_snake_game(void)
{
	REG_DISPCNT = DCNT_MODE0;

	// Iniciando os backgrounds
	REG_BG0CNT = BG_CBB(0) | BG_SBB(8)	| BG_REG_32x32 | BG_8BPP;
	REG_BG1CNT = BG_CBB(0) | BG_SBB(9)	| BG_REG_32x32 | BG_8BPP;
	REG_BG2CNT = BG_CBB(0) | BG_SBB(10)	| BG_REG_32x32 | BG_8BPP;

	// Copiando os tiles
	tile8_mem[0][0] = (TILE8){0};
	memcpy32(tile8_mem[0] + 1, TL_SnakeTiles, TL_SnakeTilesLen / 4);
	copy_background();

	// Copiando a paleta de cores
	memcpy32(pal_bg_mem, TL_SnakePal, TL_SnakePalLen / 4);

	// Ativando os backgrounds
	REG_DISPCNT = DCNT_MODE0 | DCNT_BG0 | DCNT_BG1 | DCNT_BG2;










	snake_t snake = { 0 };
	create_snake(&snake);

	poos_t poos = { 0 };
	create_poos(&poos);





	POINT dir = { 1, 0 }, background_offset = { 0, 0 };
	asset_t fruit = new_fruit();
	asset_t bunny = new_bunny();
	asset_t special = new_special();
	special.pos.x = special.pos.y = 31;



	int frame_skip = FRAME_NORMAL, special_timer = -1, food_timer = FOOD_TIMER;



	while (true)
	{
		// Passa frames
		for (int i = 0; i < frame_skip; i++)
		{
			// Espera até o próximo frame
			vid_vsync();

			// É preciso testar as teclas em cada passagem
			key_poll();
			if (key_tri_vert())
				dir.x = 0, dir.y = key_tri_vert();
			else if (key_tri_horz())
				dir.x = key_tri_horz(), dir.y = 0;
		}



		// Timer do especial
		if (special_timer > 0)
		{
			special_timer--;
		}
		else if (special_timer == 0)
		{
			special_timer = -1;

			// Volta ao normal
			frame_skip = FRAME_NORMAL;

			memcpy16(SNAKE_PALETTE_PTR, snake_colors, sizeof(snake_colors) / 2);

			// Dá tempo de processar a mudança
			for (int i = 0; i < 45; i++)
				vid_vsync();
		}
		else
		{
			// Timer da comida
			if (snake.length > 3)
				food_timer--;

			if (food_timer == 0)
			{
				new_poo(&poos, snake.body[snake.length - 1].pos.x, snake.body[snake.length - 1].pos.y);
				food_timer = FOOD_TIMER;
				snake.length--;
			}
		}
		



		// Faz o jogo ficar um pouco mais aleatório
		srand(rand() + dir.x + dir.y);





		// Impede que a cobra se mova para o próprio pescoço
		if (MOD_WIDTH (snake.body[0].pos.x + dir.x) == snake.body[1].pos.x &&
		    MOD_HEIGHT(snake.body[0].pos.y + dir.y) == snake.body[1].pos.y)
		{
			dir.x *= -1, dir.y *= -1;
		}





		move_snake(&snake, dir);

		// Move o coelho aleatoriamente
		if (rand() % 4 == 0)
			move_bunny(&bunny);





		// A cobra acaba de acertar a fruta
		if (hit_asset(&snake, &fruit))
		{
			grow_snake(&snake);
			fruit = new_fruit();
			food_timer = FOOD_TIMER;

			// Chance de aparecer o especial: 2%
			if (special_timer <= 0 && rand() % 50 == 0)
				special = new_special();

			internal_score += 100;
		}



		// A cobra acaba de acertar o coelho
		if (hit_asset(&snake, &bunny))
		{
			// Chance de crescer duas vezes
			grow_snake(&snake);
			if (rand() % 10 == 0)
				grow_snake(&snake);

			bunny = new_bunny();
			food_timer = FOOD_TIMER * (rand() % 2 + 1);

			// Chance de aparecer o especial: 5%
			if (special_timer <= 0 && rand() % 20 == 0)
				special = new_special();
		}



		// A cobra acaba de acertar o especial
		if (hit_asset(&snake, &special))
		{
			// Esconde o especial fora do mapa
			special.pos.x = 31;
			special.pos.y = 31;

			// Inicia o temporizador do especial
			special_timer += SPECIAL_TIMER;
			frame_skip = FRAME_FAST;

			// Copia um arco iris para criar a animação
			memcpy16(SNAKE_PALETTE_PTR, rainbow_colors, sizeof(rainbow_colors) / 2);
		}



		// A cobra acaba de se acertar ou acertar um poo
		// a menos que o especial esteja ativado
		if (special_timer <= 0 && (hit_snake(&snake) || hit_poos(&snake, &poos)))
		{
			vid_vsync();
			se_fill(&snake_layer[0][0], SE_BLANK);
			draw_snake(&snake);

			// Espera 2s
			for (int i = 0; i < 120; i++)
				vid_vsync();

			// Escurece a tela depois de 5s
			for (int i = 0; i < 5; i++)
			{
				for (int j = 0; j < 60; j++)
					vid_vsync();

				clr_fade_fast(pal_bg_mem, CLR_BLACK, pal_bg_mem, 256, 12);
			}

			memset16(pal_bg_mem, CLR_BLACK, 256);

			return snake.length;
		}





		// Desenhando tudo
		vid_vsync();

		// Anima as cores da cobra
		if (special_timer > 0)
			clr_rotate(SNAKE_PALETTE_PTR, sizeof(rainbow_colors) / 2, 1);

		se_fill(&snake_layer[0][0], SE_BLANK);
		se_fill(&asset_layer[0][0], SE_BLANK);

		draw_snake(&snake);
		draw_poos(&poos);
		draw_asset(&bunny);
		draw_asset(&fruit);
		draw_asset(&special);

		// Efeito parallax no background
		// É preciso fazer isso porque REG_BG**OFS
		// permite escrita mas não leitura
		background_offset.x += dir.x;
		background_offset.y += dir.y;
		REG_BG2HOFS = background_offset.x / 2;
		REG_BG2VOFS = background_offset.y / 2;
	}
}




















static void create_snake(snake_t *snake)
{
	// Configurações iniciais
	snake->body[0] = (part_t) { { 15, 10 }, FROM_LEFT, TO_HEAD  };
	snake->body[1] = (part_t) { { 14, 10 }, FROM_LEFT, TO_RIGHT };
	snake->body[2] = (part_t) { { 13, 10 }, FROM_LEFT, TO_RIGHT };
	snake->body[3] = (part_t) { { 12, 10 }, FROM_LEFT, TO_RIGHT };
	snake->body[4] = (part_t) { { 11, 10 }, FROM_TAIL, TO_RIGHT };

	snake->length = 5;
}

static void grow_snake(snake_t *snake)
{
	snake->length++;
	snake->body[snake->length - 1].pos = (POINT) { 31, 31 };
}

static void move_snake(snake_t *snake, POINT dir)
{
	// É ineficiente mas o GBA ainda aguenta
	for (int i = snake->length - 1; i >= 1; i--)
		snake->body[i] = snake->body[i - 1];

	if (dir.x ==  1) {
		snake->body[0].from = FROM_LEFT;
		snake->body[0].to = TO_HEAD;
		snake->body[1].to = TO_RIGHT;
	}
	else if (dir.x == -1) {
		snake->body[0].from = FROM_RIGHT;
		snake->body[0].to = TO_HEAD;
		snake->body[1].to = TO_LEFT;
	}
	else if (dir.y ==  1) {
		snake->body[0].from = FROM_UP;
		snake->body[0].to = TO_HEAD;
		snake->body[1].to = TO_DOWN;
	}
	else if (dir.y == -1) {
		snake->body[0].from = FROM_DOWN;
		snake->body[0].to = TO_HEAD;
		snake->body[1].to = TO_UP;
	}

	snake->body[snake->length - 1].from = FROM_TAIL;

	snake->body[0].pos.x = MOD_WIDTH (snake->body[0].pos.x + dir.x);
	snake->body[0].pos.y = MOD_HEIGHT(snake->body[0].pos.y + dir.y);
}

static void draw_snake(snake_t *snake)
{
	for (int i = snake->length - 1; i >= 0; i--)
		snake_layer[snake->body[i].pos.y][snake->body[i].pos.x] = snake->body[i].to + snake->body[i].from;
}

static bool hit_snake(snake_t *snake)
{
	for (int i = 1; i < snake->length; i++)
		if (snake->body[0].pos.x == snake->body[i].pos.x &&
			snake->body[0].pos.y == snake->body[i].pos.y)
			return true;

	return false;
}










static void create_poos(poos_t *poos)
{
	for (int i = 0; i < MAX_POOS_LENGTH; i++)
	{
		poos->list[i].pos = (POINT) { 31, 31 };
		poos->list[i].se = SE_POO1;
	}

	poos->index = 0;
}

static void new_poo(poos_t *poos, s16 x, s16 y)
{
	poos->list[poos->index].pos = (POINT) { x, y };
	poos->list[poos->index].se = (rand() % 10) ? SE_POO1 : SE_POO2;

	poos->index = (poos->index + 1) % MAX_POOS_LENGTH; 
}

static bool hit_poos(snake_t *snake, poos_t *poos)
{
	for (int i = 0; i < MAX_POOS_LENGTH; i++)
		if (snake->body[0].pos.x == poos->list[i].pos.x &&
			snake->body[0].pos.y == poos->list[i].pos.y)
			return true;

	return false;
}

static void draw_poos(poos_t *poos)
{
	for (int i = 0; i < MAX_POOS_LENGTH; i++)
		asset_layer[poos->list[i].pos.y][poos->list[i].pos.x] = poos->list[i].se;
}









static asset_t new_fruit(void)
{
	return (asset_t) { {rand() % SCR_WT, rand() % SCR_HT}, (rand() % 10) ? SE_FRUIT1 : (rand() % 10) ? SE_FRUIT2 : SE_FRUIT3 };
}











static asset_t new_bunny(void)
{
	return (asset_t) { {rand() % SCR_WT, rand() % SCR_HT}, (rand() % 10) ? SE_BUNNY1 : (rand() % 10) ? SE_BUNNY2 : SE_BUNNY3 };
}

static void move_bunny(asset_t *bunny)
{
	POINT rand_dir = { rand() % 5 - 2, rand() % 5 - 2 };

	bunny->pos.x = MOD_WIDTH (bunny->pos.x + rand_dir.x);
	bunny->pos.y = MOD_HEIGHT(bunny->pos.y + rand_dir.y);

	// Vira o coelho pra esquerda ou direita dependendo da direção
	if (rand_dir.x > 0)
		bunny->se &= ~SE_HFLIP;
	else if (rand_dir.x < 0)
		bunny->se |= SE_HFLIP;
}










static asset_t new_special(void)
{
	return (asset_t) { {rand() % SCR_WT, rand() % SCR_HT}, SE_SPECIAL };
}










static void draw_asset(asset_t *asset)
{
	asset_layer[asset->pos.y][asset->pos.x] = asset->se;
}

static bool hit_asset(snake_t *snake, asset_t *asset)
{
	if (snake->body[0].pos.x == asset->pos.x &&
		snake->body[0].pos.y == asset->pos.y)
		return true;

	return false;
}









static void copy_background(void)
{
	for (int j = 0; j < 32; j++)
		for (int i = 0; i < 32; i++)  
		{
			background_layer[j][i] = SE_BACKGROUND + i % BACKGROUND_WIDTH + (j % BACKGROUND_HEIGHT) * BACKGROUND_WIDTH;
		}
}