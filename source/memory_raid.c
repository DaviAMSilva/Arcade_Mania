#include <string.h>

#include <tonc.h>
#include <tonc_libgba.h>

#include <data/SP_Memory_Raid.h>










//










int angle_from_dir		(int dx, int dy);
int interpolate_angles	(int a1, int a2, float t);









static const POINT16 dirs[] = {
	// As direções são guardadas em centésimos de pixels
	// e apontam para o centro
	{ 141,  141},	{ 000,  100},	{-141,  141},
	{ 100,  000},	{ 000,  000},	{-100,  000},
	{ 141, -141},	{ 000, -100},	{-141, -141}
};

static const POINT16 bullet_starts[] = {
	// As direções são guardadas em centésimos de pixels
	// e apontam para fora do centro
	{-28200, -28200},	{ 00000, -20000},	{ 28200, -28200},
	{-20000,  00000},	{ 00000,  00000},	{ 20000,  00000},
	{-28200,  28200},	{ 00000,  20000},	{ 28200,  28200}
};

static const int angles[] = {
	// 8192 == 45° anti-horário
	8192*1,		0,			8192*7,
	8192*2,		0,			8192*6,
	8192*3,		8192*4,		8192*5,
};










typedef enum MemRaidAngles
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
	SE_CORE		= 1,
	SE_WARNING	= 9,
	SE_BULLET	= 137,
} MemRaidSE_t;

typedef enum MemRaidPalBank // Paletas possíveis
{
	PAL_RED		= 8,
	PAL_GREEN	= 9,
	PAL_BLUE	= 10,
	PAL_WHITE	= 11,
} memRaidPakBank_t;





typedef struct Core
{
	POINT16 pos;
	int pal_bank;
	OBJ_ATTR *obj;
	OBJ_AFFINE *obj_aff;
	int cur_angle, real_angle;
} core_t;

typedef struct Warning
{
	POINT16 pos;
	int pal_bank;
	OBJ_ATTR *obj;
	int show_time;
} warning_t;

typedef struct Bullet
{
	POINT16 pos;
	int pal_bank;
	OBJ_ATTR *obj;
	int dir_index;
} bullet_t;










/*
void obj_test()
{
	int x = 96, y = 32;
	u32 tid = 1, pb = 10; // (3) tile id, pal-bank
	OBJ_ATTR *metr = &obj_buffer[0];

	obj_set_attr(metr,
				 ATTR0_SQUARE | ATTR0_AFF,		  // Square affine sprite
				 ATTR1_SIZE_64 | ATTR1_AFF_ID(0), // 64x64p,
				 ATTR2_PALBANK(pb) | tid);		  // palbank 0, tile 0

	obj_aff_identity(&obj_aff_buffer[0]);

	// (4) position sprite (redundant here; the _real_ position
	// is set further down
	obj_set_pos(metr, x, y);

	int value = 0;

	while (1)
	{
		vid_vsync();
		key_poll();

		// (5) Do various interesting things
		// move left/right
		x += 2 * key_tri_horz();
		// move up/down
		y += 2 * key_tri_vert();

		// increment/decrement starting tile with R/L
		tid += bit_tribool(key_hit(-1), KI_R, KI_L);

		// flip
		// if(key_hit(KEY_A))  // horizontally
		//     metr->attr1 ^= ATTR1_HFLIP;
		// if(key_hit(KEY_B))  // vertically
		//     metr->attr1 ^= ATTR1_VFLIP;

		value += 256*64*key_tri_fire();

		obj_aff_rotate(&obj_aff_buffer[0], value);

		// make it glow (via palette swapping)
		pb = key_hit(KEY_SELECT) ? (pb + 1) % 16 : pb;

		// toggle mapping mode
		if (key_hit(KEY_START))
			REG_DISPCNT ^= DCNT_OBJ_1D;

		// Hey look, it's one of them build macros!
		metr->attr2 = ATTR2_BUILD(tid, pb, 0);
		obj_set_pos(metr, x, y);

		oam_copy(oam_mem, obj_buffer, 1); // (6) Update OAM (only one now)
		obj_aff_copy(obj_aff_mem, obj_aff_buffer, 1);
	}
} */

void init_memory_raid_game()
{
	// Servem para guardar as informações dos sprites antes de enviar para tela
	// Pode parecer ineficiente mas as vezes é possível não existir tempo suficiente
	// para atualizar todos os sprites, então é melhor assim
	OBJ_ATTR obj_buffer[128];
	OBJ_AFFINE *obj_aff_buffer = (OBJ_AFFINE *)obj_buffer;









	// Copia as informações necessárias
	memcpy(&tile_mem_obj[0][0], SP_Memory_RaidTiles, SP_Memory_RaidTilesLen);
	memcpy(pal_obj_mem, SP_Memory_RaidPal, SP_Memory_RaidPalLen);





	core_t core = {{SCR_W / 2 - 32, SCR_H / 2 - 32}, PAL_WHITE, &obj_buffer[0], &obj_aff_buffer[0], 0, 0};
	// warning_t warning = {NULL_POS, PAL_WHITE, &obj_buffer[1], 0};

	// Esconde todos os sprites
	oam_init(obj_buffer, 128);

	// Configuração inicial
	obj_set_attr(
		core.obj,
		ATTR0_SQUARE | ATTR0_AFF,
		ATTR1_SIZE_64 | ATTR1_AFF_ID(0),
		ATTR2_PALBANK(core.pal_bank) | ATTR2_ID(SE_CORE)
	);
	obj_aff_identity(core.obj_aff);









	REG_DISPCNT = DCNT_OBJ | DCNT_OBJ_2D;





	while (1)
	{
		vid_vsync();
		key_poll();

		// Atualiza o ângulo apenas quando necessário
		if (key_tri_horz() != 0 || key_tri_vert() != 0)
			core.real_angle = angle_from_dir(key_tri_horz(), key_tri_vert());

		// Cria uma animação mais suave
		core.cur_angle = interpolate_angles(core.cur_angle, core.real_angle, 0.7);

		obj_set_pos(core.obj, core.pos.x, core.pos.y);
		obj_aff_rotate(core.obj_aff, core.cur_angle);

		vid_vsync();

		oam_copy(oam_mem, obj_buffer, 1);
		obj_aff_copy(obj_aff_mem, obj_aff_buffer, 1);
	}
}










// Acha o ângulo correspondente dependendo da direção
int angle_from_dir(int dx, int dy)
{
	return angles[dx + 1 + 3 * (dy + 1)];
}

// Acha o ângulo médio entre dois ângulos
int interpolate_angles(int a1, int a2, float t)
{
	int da = (a2 - a1) % CC360;
	return a1 + t * (2 * da % CC360 - da);
}