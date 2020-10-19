#ifndef SNAKE_H
#define SNAKE_H

#include <data/TL_Snake.h>
#include <tonc.h>
#include <tonc_video.h>

#include <stdlib.h>
#include <string.h>

#define SNAKE_BACKGROUND 0
#define SNAKE_HEAD 1
#define SNAKE_BODY 2
#define SNAKE_TAIL 3
#define SNAKE_FRUIT 4
#define SNAKE_POO 5

void init_snake(void);

#endif /* SNAKE_H */