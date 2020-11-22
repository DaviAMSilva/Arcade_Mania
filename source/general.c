#include <tonc.h>










void clear_video_mem(void)
{
	vid_vsync();
	memset32(tile_mem, 0, CBB_SIZE);
	memset32(pal_bg_mem, 0, PAL_SIZE/2);
}