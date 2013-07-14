#include <string.h>

#include <GL/gl.h>
#include <GL/glext.h> 
#include <GL/glu.h>

#include <GL/freeglut.h>

#include "android_interface.h"

//temp
#include "cpu.h"
#include "memory.h"

#include "graphics.h"


//#define TEXTURE_WIDTH 256
#define TEXTURE_WIDTH 160
//#define TEXTURE_HEIGHT 256
#define TEXTURE_HEIGHT 144 

static int image_width = 160;
static int image_height = 144;

#define RGB565(r, g, b)  (((r) << (5+6)) | ((g) << 6) | (b))
#define PIXELS_SIZE (sizeof(pixels[0]) * TEXTURE_WIDTH * TEXTURE_HEIGHT)

uint16_t frame[FRAME_HEIGHT][FRAME_WIDTH];

int window_width = 80;
int window_height = 77;

static GLuint texture = 0;

static uint16_t *pixels = NULL;

static int last_frame = 0;

static int target_elapsed = 16; // 60fps

bool save_state_called = false;
bool load_state_called = false;

void save_state_callback() {

    uint8_t* state = NULL;
    int size = 0;

    cgbemu_save_state(&state, &size);

    assert(size > 0);
    assert(state != NULL);

    FILE* file = fopen("save_state.bin", "wb");
    int bytes_written = 0;
    while(bytes_written < size)
        bytes_written += fwrite((state + bytes_written), 1, (size - bytes_written), file); 
       
    if(bytes_written == 0) {
        perror("fwrite() failed\n");
        return;
    }

    fclose(file);

    free(state);
}

void load_state_callback() {
 
	FILE *file = fopen("save_state.bin", "rb");

	if(!file) {
        perror("fopen() failed");
		return;
	}
	
	fseek(file, 0, SEEK_END);
	int	size = ftell(file);
	rewind(file);
	
	uint8_t* state = malloc(size);
	
    int bytes_read;
	if((bytes_read = fread(state, sizeof(*state), size, file)) == 0) {
        perror("fread() failed");
		fclose(file);
		return;
	}
	
	fclose(file);

    cgbemu_load_state(state, size);

    free(state);
}

     


bool step_through = false;
void step()
{
	if(step_through)
		glutPostRedisplay();
}

void idle(void)
{
	int current_time = glutGet(GLUT_ELAPSED_TIME);
	int elapsed = current_time - last_frame;
	
	if(elapsed < target_elapsed) {
#ifdef _WIN32
		//Sleep(elapsed);
#else
        //usleep(elapsed * 1000);
        //fprintf(stderr, "sleeping for %i micro-seconds\n", elapsed);
        //usleep(elapsed);

#endif
	}
	last_frame = current_time;


    if(save_state_called == true) {
        save_state_callback();
        save_state_called = false;
    } else if(load_state_called == true) {
        load_state_callback();
        load_state_called = false;
    }
	
	if(!step_through) 
		glutPostRedisplay();
}


static void render_pixels(uint16_t *pixels)
{
	get_next_frame(frame);
	
	int x, y;

	
	for(y = 0; y < TEXTURE_HEIGHT; ++y) {
		for(x = 0; x < TEXTURE_WIDTH; ++x) {
			if(x < image_width && y < image_height) {
				//*(pixels++) = RGB565(31, 31, 0);
				*(pixels++) = frame[y][x];
			} else {
				*(pixels++) = RGB565(0, 31, 31);
			}
		}
	}
}

void display(void)
{
	glClear (GL_COLOR_BUFFER_BIT);
	glColor3f (1.0, 1.0, 1.0);
	
	memset(pixels, 0, PIXELS_SIZE);
	render_pixels(pixels);
	
	glLoadIdentity();
	glTranslatef( 0, 0, 0.f );
	
	glTexImage2D(GL_TEXTURE_2D,		/* target */
		0,			/* level */
		GL_RGB,			/* internal format */
		TEXTURE_WIDTH,		/* width */
		TEXTURE_HEIGHT,		/* height */
		0,			/* border */
		GL_RGB,			/* format */
		GL_UNSIGNED_SHORT_5_6_5,/* type */
		pixels);		/* pixels */

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	
	glBegin (GL_QUADS);
	glTexCoord2f (0.0, 1.0); glVertex2f (-1.0, -1.0);
	glTexCoord2f (1.0, 1.0); glVertex2f (1.0, -1.0);
	glTexCoord2f (1.0, 0.0); glVertex2f (1.0, 1.0);
	glTexCoord2f (0.0, 0.0); glVertex2f (-1.0, 1.0);
	glEnd ();
	
	glFlush ();
}



void reshape (int w, int h)
{
	window_height = h;
	window_width = w;
	
	glViewport (0, 0, (GLsizei) w, (GLsizei) h); 
	
	glDeleteTextures(1, &texture);
	
	glEnable(GL_TEXTURE_2D);
	
	glGenTextures(1, &texture);
	glBindTexture(GL_TEXTURE_2D, texture);
	
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
}

/*
void dump_vram()
{
    FILE* dump = fopen("vram_dump.bin", "w");
    fwrite(selected_gameboy_vram_bank, 1, 0x2000, dump);
    fclose(dump);
}
*/

void processKeyboard(int key, bool down)
{
	switch (key) {
		case 'z':
			cgbemu_set_button_pressed(BUTTON_B, down);
			break;
		case 'x':
			cgbemu_set_button_pressed(BUTTON_A, down);
			break;
		case 'f':
			if(!down) step_through = !step_through;
			break;
		case ' ':
			if(!down) step();
			break;
        case 'q':
            exit(EXIT_SUCCESS);
        case 'b':
            if(!down) enable_breakpoints = !enable_breakpoints;
            break;
        case 's':
            if(!down) cpu_step = !cpu_step;
        case 'd':
            //dump_vram();
            break;
        case 'm':
            print_map();
            break;

		default:
			return;
	}
	
}

void processSpecial(int key, bool down)
{
	switch(key) {
		case GLUT_KEY_UP:
			cgbemu_set_button_pressed(BUTTON_UP, down);
			break;
		case GLUT_KEY_RIGHT:
			cgbemu_set_button_pressed(BUTTON_RIGHT, down);
			break;
		case GLUT_KEY_DOWN:
			cgbemu_set_button_pressed(BUTTON_DOWN, down);
			break;
		case GLUT_KEY_LEFT:
			cgbemu_set_button_pressed(BUTTON_LEFT, down);
			break;
        case GLUT_KEY_F1:
            if(!down)
                save_state_called = true;
            break;
        case GLUT_KEY_F2:
            if(!down)
                load_state_called = true;
            break;

		default:
			return;
	}
}


void keyboard(unsigned char key, int x, int y)
{
	processKeyboard(key, true);
}

void keyboardUp(unsigned char key, int x, int y)
{
	processKeyboard(key, false);
}

void special(int key, int x, int y)
{
	processSpecial(key, true);
}

void specialUp(int key, int x, int y)
{
	processSpecial(key, false);
}

int open_cartridge(char* filepath)
{
    if(filepath == NULL)
    	filepath = "./roms/cpu_instrs.gb";


	FILE *file = fopen(filepath, "rb");

	

	if(!file) {
        perror("fopen() failed");
		return 0;
	}
	
	fseek(file, 0, SEEK_END);
	int	size = ftell(file);
	rewind(file);
	
	uint8_t* buffer = malloc(size);
	
	if(fread(buffer, sizeof(*buffer), size, file) == 0) {
        perror("fread() failed");
		fclose(file);
		return 0;
	}
	
	fclose(file);
	
	cgbemu_load_cartridge(buffer, size);
	
	free(buffer);
	
	return size;
}

int load_bios()
{
    
    char filepath[] = "./bios/gbc_bios.bin";

	FILE *file = fopen(filepath, "rb");

	if(!file) {
        perror("fopen() failed");
		return 0;
	}
	
	fseek(file, 0, SEEK_END);
	int	size = ftell(file);
	rewind(file);
	
	uint8_t* buffer = malloc(size);
	
	if(fread(buffer, sizeof(*buffer), size, file) == 0) {
        perror("fread() failed");
		fclose(file);
		return 0;
	}
	
	fclose(file);
	
	cgbemu_load_bios(buffer, size);
	
	free(buffer);
	
	return size;
 
}

int main(int argc, char** argv)
{
	pixels = malloc(PIXELS_SIZE);
	
	
	setup();
    
    if(argc > 1) {
        open_cartridge(argv[1]);
    } else {

	    open_cartridge(NULL);
    }

    
    load_bios();
	
	glutInit(&argc, argv);
	glutInitDisplayMode (GLUT_SINGLE | GLUT_RGB);
	glutInitWindowSize (window_width, window_height); 
	glutInitWindowPosition (100, 100);
	glutCreateWindow (argv[0]);

	glutDisplayFunc(display);
	glutIdleFunc(idle);
	glutReshapeFunc(reshape);
	glutKeyboardFunc(keyboard);
	glutKeyboardUpFunc(keyboardUp);
	glutSpecialFunc(special);
	glutSpecialUpFunc(specialUp);
	
    //fprintf(stderr, "before glutMainLoop()\n");

	glutMainLoop();
	return 0;
}
