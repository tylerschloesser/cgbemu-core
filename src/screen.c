
/*
#ifdef WIN32
#include <Windows.h>
#endif
*/

#include <assert.h>

#include "screen.h"
#include "joypad.h"

//u8 screen_buffer[SCREEN_HEIGHT][SCREEN_WIDTH][3];
uint16_t screen_buffer[SCREEN_HEIGHT][SCREEN_WIDTH];

static bool screen_initialized = false;

u64 freq = 0;
u64 last_update = 0;
const u64 target_elapsed_usec = 1000000 / 60 - 1000;

void initialize_screen() {
	
	assert( screen_initialized == false );

	/*
	//REDUNDANT
	QueryPerformanceFrequency( (LARGE_INTEGER*)&freq );
	QueryPerformanceCounter((LARGE_INTEGER*)&last_update);
	*/
	
	fprintf( stdout, "initializing screen\n" );
	
	int i, j;
	for(i = 0; i < SCREEN_HEIGHT; ++i) {
		for(j = 0; j < SCREEN_WIDTH; ++j) {
			screen_buffer[i][j] = 0;
			/*
			screen_buffer[i][j][0] = 0;
			screen_buffer[i][j][1] = 0;
			screen_buffer[i][j][2] = 0;
			*/
		}
	}
	
	screen_initialized = true;
}

void reinitialize_screen()
{
	assert( screen_initialized == true );
	screen_initialized = false;
	initialize_screen();
}

/*
void update_screen() {

	//assert(surface != NULL);
	
	//surface = SDL_SetVideoMode(160,144,32,SDL_SWSURFACE);
	//if(!surface) return;

	u64 now;
	QueryPerformanceCounter((LARGE_INTEGER*)&now);
	u64 elapsed_usec = ((now - last_update) / (double)freq) * 1000000.0;
	if(elapsed_usec < target_elapsed_usec) {
		return;
	}
	last_update = now;
	
	if(surface == NULL) {
	
		static bool getting_video_surface = false;
	
		if( getting_video_surface == false ) {
			fprintf( stdout, "getting video surface\n" );
		}		
		
		getting_video_surface = true;
		surface = SDL_GetVideoSurface();
				
		if(surface == NULL) {
			return;
		} else {
			fprintf( stdout, "got video surface\n" );
		}
	}
	
		
	u32* sdl_pixel = (u32*)surface->pixels;

	int i, j;
	for(i = 0; i < SCREEN_HEIGHT; ++i) {
		for(j = 0; j < SCREEN_WIDTH; ++j) {
		
			u8 r, g, b;
			r = screen_buffer[i][j][0];
			g = screen_buffer[i][j][1];
			b = screen_buffer[i][j][2];
			
			u32 sdl_color = SDL_MapRGB(surface->format, r*8, g*8, b*8);
			*(sdl_pixel++) = sdl_color;
		}	
	}
	
	SDL_UpdateRect(surface, 0, 0, 0, 0); 
	
//gtk_sdl_display_update ( GTK_SDL ( SDL ) ) ;
}
*/

void update_screen_buffer( int y_pixel, int x_pixel, u16 pallete_entry ) 
{
	u8 r, g, b;
	
	r = (pallete_entry >> 0 ) & 0x1F;
	g = (pallete_entry >> 5 ) & 0x1F;
	b = (pallete_entry >> 10) & 0x1F;

	uint16_t rgb_565 = b | (g << 6) | (r << 11);
	screen_buffer[y_pixel][x_pixel] = rgb_565;
}
