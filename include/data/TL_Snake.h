
//{{BLOCK(TL_Snake)

//======================================================================
//
//	TL_Snake, 64x64@8, 
//	Transparent color : FF,00,FF
//	+ palette 256 entries, not compressed
//	+ 7 tiles (t|f reduced) not compressed
//	+ regular map (flat), not compressed, 8x8 
//	Total size: 512 + 448 + 128 = 1088
//
//	Time-stamp: 2020-10-18, 17:29:31
//	Exported by Cearn's GBA Image Transmogrifier, v0.8.16
//	( http://www.coranac.com/projects/#grit )
//
//======================================================================

#ifndef GRIT_TL_SNAKE_H
#define GRIT_TL_SNAKE_H

#define TL_SnakeTilesLen 448
extern const unsigned int TL_SnakeTiles[112];

#define TL_SnakeMapLen 128
extern const unsigned short TL_SnakeMap[64];

#define TL_SnakePalLen 512
extern const unsigned short TL_SnakePal[256];

#endif // GRIT_TL_SNAKE_H

//}}BLOCK(TL_Snake)
