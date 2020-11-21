#include <string.h>
#include <stdlib.h>

#include <tonc.h>
#include <tonc_libgba.h>

#include <data/SP_Memory_Raid.h>
#include <data/BG_Memory_Raid.h>










#define WARNING_TIMER	110
#define BULLET_TIMER	130
#define BREATHING_TIMER	150
#define FLASH_TIMER		10

#define NUM_REG_OBJS	4
#define NUM_AFF_OBJS	2
#define NUM_PALETTES	3
#define NUM_DIRECTIONS	8

#define MAX_PHASES       64
#define BULLET_SPEED     5
#define MEMORY_RAID_SEED 15



#define SHIELD_REG_MEM	(&obj_mem[0])
#define SHIELD_REG_BUF	(&obj_buffer[0])
#define SHIELD_AFF_MEM	(&obj_aff_mem[0])
#define SHIELD_AFF_BUF	(&obj_aff_buffer[0])

#define CORE_REG_MEM	(&obj_mem[1])
#define CORE_REG_BUF	(&obj_buffer[1])
#define CORE_AFF_MEM	(&obj_aff_mem[1])
#define CORE_AFF_BUF	(&obj_aff_buffer[1])

#define WARNING_MEM		(&obj_mem[2])
#define WARNING_BUF		(&obj_buffer[2])

#define BULLET_MEM		(&obj_mem[3])
#define BULLET_BUF		(&obj_buffer[3])



#define BG_GRADIENT_PTR	(&pal_bg_mem[160])
#define BG_GRADIENT_NUM	16









typedef enum MemRaidAngles // Alguns ângulos
{
	// Sentido Anti-Horário
	CC0		= 0,
	CC45	= 8192,
	CC90	= CC45 * 2,
	CC180	= CC90 * 2,
	CC270	= CC90 * 3,
	CC360	= CC180 * 2,

	// Sentido-Horário
	C0		= -CC0,
	C45		= -CC45,
	C90		= -CC90,
	C180	= -CC180,
	C270	= -CC270,
	C360	= -CC360,
} memRaidAngles_t;

typedef enum MemRaidSE // Tiles do jogo
{
	SE_SHIELD	= 0,
	SE_WARNING	= 8,
	SE_BULLET	= 136,
	SE_CORE		= 12,
} MemRaidSE_t;

typedef enum MemRaidPalBank // Paletas possíveis
{
	PAL_RED		= 8,
	PAL_GREEN	= 9,
	PAL_BLUE	= 10,
	PAL_WHITE	= 11,
	PAL_CORE	= 12,
} memRaidPakBank_t;





struct AffObject
{
	POINT pos;
	OBJ_ATTR *obj;
	OBJ_AFFINE *obj_aff;
	int cur_angle, real_angle;
	int pal_index;
	int dir_index;
} ALIGN4 shield, core;
typedef struct AffObject shield_t, core_t;

struct Object
{
	POINT pos;
	OBJ_ATTR *obj;
	int pal_index;
	int dir_index;
} ALIGN4 warning, bullet;
typedef struct Object warning_t, bullet_t;

// typedef struct Bullet
// {
// 	POINT pos;
// 	int pal_index;
// 	OBJ_ATTR *obj;
// 	int dir_index;
// } ALIGN4 bullet_t;










static int get_angle_index		(int dx, int dy);
static int interpolate_angles	(int a1, int a2, float t);

static void show_warnings		(int *phases_dirs, int *phases_pals, int *num_phases);
static void shoot_bullets		(int *phases_dirs, int *phases_pals, int *num_phases);

static void update_shield		(void);
static void update_background	(void);
static void update_core			(void);










// Direções que a bala se move
static const POINT directions[] = {
	{ BULLET_SPEED,  BULLET_SPEED }, { 0,  BULLET_SPEED }, { -BULLET_SPEED,  BULLET_SPEED },
	{ BULLET_SPEED,             0 },                       { -BULLET_SPEED,             0 },
	{ BULLET_SPEED, -BULLET_SPEED }, { 0, -BULLET_SPEED }, { -BULLET_SPEED, -BULLET_SPEED }
};

// Posições que a bala começa
static const POINT bullet_starts[] = {
	{ -30-16, -70-16 }, { 120-16, -70-16 }, { 270-16, -70-16 },
	{ -30-16,  80-16 },                     { 270-16,  80-16 },
	{ -30-16, 230-16 }, { 120-16, 230-16 }, { 270-16, 230-16 }
};

// Locais em que o aviso aparece
static const POINT warning_starts[] = {
	{ 56-16,  16-16 }, { 120-16,  16-16 }, { 184-16,  16-16 },
	{ 56-16,  80-16 },                     { 184-16,  80-16 },
	{ 56-16, 144-16 }, { 120-16, 144-16 }, { 184-16, 144-16 }
};

// Ângulos que o escudo pode ter
static const int angles[] = {
	CC45*1, CC0,    CC45*7,
	CC45*2,         CC45*6,
	CC45*3, CC45*4, CC45*5,
};

// Paletas de cores que existem
static const int palettes[] = {
	PAL_RED, PAL_GREEN, PAL_BLUE
};










// Servem para guardar as informações dos sprites antes de enviar para tela
// Pode parecer ineficiente mas as vezes é possível não existir tempo suficiente
// para atualizar todos os sprites, então é melhor assim
static OBJ_ATTR obj_buffer[128];
static OBJ_AFFINE *obj_aff_buffer = (OBJ_AFFINE *)obj_buffer;





// Guarda as pontuações
extern int memory_raid_score;
extern int memory_raid_score_high;










int init_memory_raid_game()
{
	// Lista que guarda as direções e as cores para cada bala do jogo
	int phases_dirs[MAX_PHASES] = { rand() % NUM_DIRECTIONS, 0 };
	int phases_pals[MAX_PHASES] = { rand() % NUM_PALETTES, 0 };
	int num_phases = 1;





	srand(MEMORY_RAID_SEED);











	// Copia as informações necessárias sobre o background
	memcpy32(tile8_mem[0], BG_Memory_RaidTiles, BG_Memory_RaidTilesLen / 4);
	memcpy32(se_mem[8], BG_Memory_RaidMap, BG_Memory_RaidMapLen / 4);
	memcpy32(pal_bg_mem, BG_Memory_RaidPal, BG_Memory_RaidPalLen / 4);



	// Copia as informações necessárias sobre os objetos
	memcpy(tile_mem_obj, SP_Memory_RaidTiles, SP_Memory_RaidTilesLen);
	memcpy(pal_obj_mem, SP_Memory_RaidPal, SP_Memory_RaidPalLen);





	// Valores iniciais
	shield	= (shield_t)	{{120 - 32, 80 - 32}, SHIELD_REG_BUF, SHIELD_AFF_BUF, CC0, CC0, phases_pals[0], phases_dirs[0]};
	core	= (core_t)		{{120 - 16, 80 - 16}, CORE_REG_BUF, CORE_AFF_BUF, CC0, CC0, 0, 0};
	warning	= (warning_t)	{{0,0}, WARNING_BUF, phases_pals[0], phases_dirs[0]};
	bullet	= (bullet_t)	{bullet_starts[phases_dirs[0]], BULLET_BUF, phases_pals[0], phases_dirs[0]};





	// Esconde todos os sprites
	oam_init(obj_buffer, 128);



	// Configuração inicial dos objetos afims
	obj_set_attr(
		shield.obj,
		ATTR0_SQUARE | ATTR0_MOSAIC | ATTR0_HIDE,
		ATTR1_SIZE_64 | ATTR1_AFF_ID(0),
		ATTR2_PALBANK(phases_pals[shield.pal_index]) | ATTR2_PRIO(2) | ATTR2_ID(SE_SHIELD)
	);
	obj_aff_identity(shield.obj_aff);
	obj_set_pos(SHIELD_REG_BUF, shield.pos.x, shield.pos.y);
	obj_copy(SHIELD_REG_MEM, SHIELD_REG_BUF, 1);

	obj_set_attr(
		core.obj,
		ATTR0_SQUARE | ATTR0_AFF,
		ATTR1_SIZE_32 | ATTR1_AFF_ID(1),
		ATTR2_PALBANK(PAL_CORE) | ATTR2_PRIO(3) | ATTR2_ID(SE_CORE)
	);
	obj_aff_identity(core.obj_aff);
	obj_set_pos(CORE_REG_BUF, core.pos.x, core.pos.y);
	obj_copy(CORE_REG_MEM, CORE_REG_BUF, 1);



	// Configuração inicial dos objetos regulares
	obj_set_attr(
		warning.obj,
		ATTR0_SQUARE,
		ATTR1_SIZE_32,
		ATTR2_PALBANK(phases_pals[warning.pal_index]) | ATTR2_PRIO(1) | ATTR2_ID(SE_WARNING)
	);
	obj_set_attr(
		bullet.obj,
		ATTR0_SQUARE,
		ATTR1_SIZE_32,
		ATTR2_PALBANK(phases_pals[bullet.pal_index]) | ATTR2_PRIO(0) | ATTR2_ID(SE_BULLET)
	);





	// Ativa os objetos e o background
	REG_DISPCNT = MODE_0 | DCNT_OBJ | DCNT_OBJ_2D | DCNT_BG0;
	REG_BG0CNT = BG_CBB(0) | BG_SBB(8)	| BG_REG_32x32 | BG_8BPP | BG_PRIO(3);










	while (1)
	{		
		// Dá tempo de respirar
		for (int i = 0; i < BREATHING_TIMER; i++)
		{
			update_shield();
			update_core();
			obj_aff_copy(CORE_AFF_MEM, CORE_AFF_BUF, 1);
		}

		update_background();





		// Mostra quais movimentos virão
		show_warnings(phases_dirs, phases_pals, &num_phases);

		// Realiza os movimentos em si
		shoot_bullets(phases_dirs, phases_pals, &num_phases);





		// Finaliza o jogo quando necessário
		if (memory_raid_score)
			return memory_raid_score;





		// Aleatoriza as fases
		phases_dirs[num_phases] = rand() % NUM_DIRECTIONS;
		phases_pals[num_phases] = rand() % NUM_PALETTES;

		num_phases++;
	}
}










static void show_warnings(int *phases_dirs, int *phases_pals, int *num_phases)
{
	obj_unhide(WARNING_BUF, ATTR0_REG);



	for (int cur_phase = 0; cur_phase < *num_phases; cur_phase++)
	{
		warning.dir_index = phases_dirs[cur_phase];
		warning.pal_index = phases_pals[cur_phase];

		warning.pos = warning_starts[warning.dir_index];
		warning.obj->attr2 &= ~ATTR2_PALBANK_MASK;
		warning.obj->attr2 |= ATTR2_PALBANK(palettes[warning.pal_index]);



		obj_set_pos(WARNING_BUF, warning.pos.x, warning.pos.y);

		obj_copy(WARNING_MEM, WARNING_BUF, 1);


		// Aviso fica parado por um tempo
		for (int t = 0; t < WARNING_TIMER; t++)
		{
			update_shield();
			update_core();
			obj_aff_copy(CORE_AFF_MEM, CORE_AFF_BUF, 1);
		}
	}



	obj_hide(WARNING_BUF);
	vid_vsync();
	obj_copy(WARNING_MEM, WARNING_BUF, 1);
}










static void shoot_bullets(int *phases_dirs, int *phases_pals, int *num_phases)
{
	for (int cur_phase = 0; cur_phase < *num_phases; cur_phase++)
	{
		obj_unhide(BULLET_BUF, ATTR0_REG);



		bullet.dir_index = phases_dirs[cur_phase];
		bullet.pal_index = phases_pals[cur_phase];

		bullet.pos = bullet_starts[bullet.dir_index];
		bullet.obj->attr2 &= ~ATTR2_PALBANK_MASK;
		bullet.obj->attr2 |= ATTR2_PALBANK(palettes[bullet.pal_index]);



		// Testa para ver se o escudo e a bala não colidiram ainda
		while (30 < ABS(shield.pos.x + 32 - bullet.pos.x - 16) + ABS(shield.pos.y + 32 - bullet.pos.y - 16))
		{
			bullet.pos.x += directions[bullet.dir_index].x;
			bullet.pos.y += directions[bullet.dir_index].y;

			obj_set_pos(BULLET_BUF, bullet.pos.x, bullet.pos.y);



			update_shield();
			update_core();
			obj_copy(BULLET_MEM, BULLET_BUF, 1);
			obj_aff_copy(CORE_AFF_MEM, CORE_AFF_BUF, 1);
		}



		// O jogador não estava na direção certa e o jogo acabou
		if (bullet.dir_index != shield.dir_index)
		{
			// Espera 2s
			for (int i = 0; i < 120; i++)
				vid_vsync();

			// Escurece a tela depois de 5s
			for (int i = 0; i < 5; i++)
			{
				for (int j = 0; j < 60; j++)
					vid_vsync();

				clr_fade_fast(pal_bg_mem, CLR_BLACK, pal_bg_mem, 256, 12);
				clr_fade_fast(pal_obj_mem, CLR_BLACK, pal_obj_mem, 256, 12);
			}

			memset16(pal_bg_mem, CLR_BLACK, 256);
			memset16(pal_obj_mem, CLR_BLACK, 256);



			// Salva a pontuação do jogador
			memory_raid_score = *num_phases;
		}
		else if (bullet.pal_index != shield.pal_index)
		{
			// Caso o jogador esteja na posição correta mas na cor errada
			// ele é forçado a repetir os movimentos por mais uma fase
			(*num_phases)--;

			vid_vsync();
			obj_hide(BULLET_BUF);
			obj_copy(BULLET_MEM, BULLET_BUF, 1);

			return;
		}



		vid_vsync();
		SHIELD_REG_MEM->attr2 &= ~ATTR2_PALBANK_MASK;
		SHIELD_REG_MEM->attr2 |= ATTR2_PALBANK(PAL_WHITE);
		BULLET_MEM->attr2 &= ~ATTR2_PALBANK_MASK;
		BULLET_MEM->attr2 |= ATTR2_PALBANK(PAL_WHITE);

		for (int i = 0; i < FLASH_TIMER; i++)
			vid_vsync();


		obj_hide(BULLET_BUF);



		vid_vsync();
		obj_copy(BULLET_MEM, BULLET_BUF, 1);
		obj_copy(SHIELD_REG_MEM, SHIELD_REG_BUF, 1);



		// Tempo entre as balas
		for (int t = 0; t < BULLET_TIMER; t++)
		{
			update_shield();
			update_core();
			obj_aff_copy(CORE_AFF_MEM, CORE_AFF_BUF, 1);
		}
	}
}










// Atualiza a rotação do escudo
static void update_shield()
{
	// Permite o efeito mosaico
	static int mosaic_strength = 0;



	key_poll();



	shield.dir_index = get_angle_index(key_tri_horz(), key_tri_vert());



	// O jogador aperta um botão então o escudo é visível
	if (shield.dir_index >= 0)
	{
		if (mosaic_strength > 0)
		{
			mosaic_strength--;
			REG_MOSAIC = MOS_BUILD(0, 0, mosaic_strength, mosaic_strength);
		}



		obj_unhide(shield.obj, ATTR0_AFF);



		// Cria uma animação mais suave
		shield.real_angle = angles[shield.dir_index];
		shield.cur_angle = interpolate_angles(shield.cur_angle, shield.real_angle, 0.5);

		obj_aff_rotate(shield.obj_aff, shield.cur_angle);



		// Muda as cores do escudo com as teclas A/B L/R
		if (key_hit(KEY_A) | key_hit(KEY_R))
			shield.pal_index = (shield.pal_index + NUM_PALETTES + 1) % NUM_PALETTES;
		else if (key_hit(KEY_B) | key_hit(KEY_L))
			shield.pal_index = (shield.pal_index + NUM_PALETTES - 1) % NUM_PALETTES;



		shield.obj->attr2 &= ~ATTR2_PALBANK_MASK;
		shield.obj->attr2 |= ATTR2_PALBANK(palettes[shield.pal_index]);
	}
	else
	{
		// O escudo não está visível
		if (mosaic_strength < 15)
		{
			mosaic_strength++;
			REG_MOSAIC = MOS_BUILD(0, 0, mosaic_strength, mosaic_strength);
		}
		else if (mosaic_strength >= 15)
		{
			obj_hide(shield.obj);
		}
	}



	// Torna o jogo um pouco mais aleatório
	srand(rand() + shield.pal_index);




	// Atualiza o escudo para a memória
	vid_vsync();
	obj_copy(SHIELD_REG_MEM, SHIELD_REG_BUF, 1);
	obj_aff_copy(SHIELD_AFF_MEM, SHIELD_AFF_BUF, 1);
}



// Muda as cores do background
static void update_background()
{
	vid_vsync();
	clr_rotate(BG_GRADIENT_PTR, BG_GRADIENT_NUM, 1);
}



// Atualiza a rotação do core
static void update_core()
{
	core.real_angle -= 100;

	obj_aff_rotate(CORE_AFF_BUF, core.real_angle);

	// vid_vsync();
	// obj_copy(CORE_REG_MEM, CORE_REG_BUF, 1);
	// obj_aff_copy(CORE_AFF_MEM, CORE_AFF_BUF, 1);
}










// Acha o ângulo correspondente dependendo da direção
static int get_angle_index(int dx, int dy)
{
	static const int indexes[] = {
		0,  1, 2,
		3, -1, 4,
		5,  6, 7
	};

	return indexes[dx + 1 + 3 * (dy + 1)];
}



// Acha o ângulo médio entre dois ângulos
static int interpolate_angles(int a1, int a2, float t)
{
	int da = (a2 - a1) % CC360;
	return a1 + t * (2 * da % CC360 - da);
}