#include <string.h>
#include <assert.h>
#include <stdio.h>

#include <GL/gl.h>
#include <GL/glext.h> 
#include <GL/glu.h>
#include <GL/freeglut.h>

#include "cgbemu.h"

#define TEXTURE_WIDTH 160
#define TEXTURE_HEIGHT 144 

#define RGB565(r, g, b)  (((r) << (5+6)) | ((g) << 6) | (b))
#define SCREEN_BUFFER_SIZE (sizeof(screen_buffer[0]) * TEXTURE_WIDTH * TEXTURE_HEIGHT)

int window_width = 320;
int window_height = 288;

static GLuint texture = 0;
static uint16_t *screen_buffer = NULL;

static int last_frame = 0;
static int target_elapsed = 16; // 60fps

bool save_state_called = false;
bool load_state_called = false;

void save_state_callback() {
    cgbemu_save_state("save_state.bin");
}

void load_state_callback() {
    cgbemu_load_state("save_state.bin");
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
    
    glutPostRedisplay();
}

void display(void)
{
    glClear (GL_COLOR_BUFFER_BIT);
    glColor3f (1.0, 1.0, 1.0);
    
    cgbemu_run_to_vblank();
    
    glLoadIdentity();
    glTranslatef( 0, 0, 0.f );
    
    glTexImage2D(GL_TEXTURE_2D,
        0,
        GL_RGB,
        TEXTURE_WIDTH,
        TEXTURE_HEIGHT,
        0,
        GL_RGB,
        GL_UNSIGNED_SHORT_5_6_5,
        screen_buffer);

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

void processKeyboard(int key, bool down)
{
    switch (key) {
        case 'z':
            cgbemu_set_button_pressed(B, down);
            break;
        case 'x':
            cgbemu_set_button_pressed(A, down);
            break;
        case 13: // Enter key
            cgbemu_set_button_pressed(START, down);
            break;
        case ' ':
            cgbemu_set_button_pressed(SELECT, down);
            break;
        case 'q':
            exit(EXIT_SUCCESS);
        default:
            return;
    }
    
}

void processSpecial(int key, bool down)
{
    switch(key) {
        case GLUT_KEY_UP:
            cgbemu_set_button_pressed(UP, down);
            break;
        case GLUT_KEY_RIGHT:
            cgbemu_set_button_pressed(RIGHT, down);
            break;
        case GLUT_KEY_DOWN:
            cgbemu_set_button_pressed(DOWN, down);
            break;
        case GLUT_KEY_LEFT:
            cgbemu_set_button_pressed(LEFT, down);
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

int main(int argc, char** argv)
{
    if(argc != 2) {
        printf("Invalid arguments\n");
        return EXIT_FAILURE;
    }

    if(initialize_cgbemu(argv[1], NULL) != 0) {
        return EXIT_FAILURE;
    }

    screen_buffer = cgbemu_get_screen_buffer();
    if(screen_buffer == NULL) {
        fprintf(stderr, "Failed to get screen buffer\n");
        return EXIT_FAILURE;
    }
       
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
    
    glutMainLoop();
    return 0;
}
