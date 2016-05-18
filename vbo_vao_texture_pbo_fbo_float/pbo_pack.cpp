#include "shader.h"

GLuint texture01 = 0;

void displayCB();
void displayFirstCB();
void reshapeCB(int w, int h);
void keyboardCB(unsigned char key, int x, int y);
void exitCB();

bool initSharedMem();
void clearSharedMem();
void add(unsigned char* src, int width, int height, int shift, unsigned char* dst);
void toOrtho();
void toPerspective();

// constants
const int SCREEN_WIDTH = 256;
const int SCREEN_HEIGHT = 256;
const float CAMERA_DISTANCE = 1.0f;
const int CHANNEL_COUNT = 4;
const int DATA_SIZE = SCREEN_WIDTH * SCREEN_HEIGHT * CHANNEL_COUNT;
const GLenum PIXEL_FORMAT = GL_BGRA;
const int PBO_COUNT = 2;

// global variables
GLuint pboIds[PBO_COUNT];           // IDs of PBOs
int drawMode = 0;
GLubyte* colorBuffer = 0;

GLuint vao = 101;
GLuint vbo_vertex = 102;
GLuint vbo_color = 106;
GLuint vbo_coord = 103;
GLuint ebo = 105;

GLfloat vertices[] = {
    //  ---- 位置 ----     ---- 颜色 ----  ---- 纹理坐标 ----
    -0.5f, 0.5f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 1.0f,  // 左上
    -0.5f, -0.5f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f,// 左下
    0.5f, 0.5f, 0.0f, 0.0f, 1.0f, 0.0f, 1.0f, 1.0f,  // 右上
    0.5f, -0.5f, 0.0f, 1.0f, 1.0f, 0.0f, 1.0f, 0.0f, // 右下
};

GLuint indices[] = {0,1,2,3};

void renderTri()
{
    glClearColor(0.0, 1.0, 0.85, 1.0);
    glClear(GL_COLOR_BUFFER_BIT);

    // Bind Texture
    glBindTexture(GL_TEXTURE_2D, texture01);
    glBindVertexArray(vao);
    glEnableClientState(GL_VERTEX_ARRAY);
    glEnableClientState(GL_TEXTURE_COORD_ARRAY);
    glEnableClientState(GL_COLOR_ARRAY);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
    glDrawElements(GL_TRIANGLE_STRIP, 4, GL_UNSIGNED_INT, 0);

    glDisableClientState(GL_COLOR_ARRAY);
    glDisableClientState(GL_TEXTURE_COORD_ARRAY);
    glDisableClientState(GL_VERTEX_ARRAY);
}

void initRS()
{
    glShadeModel(GL_SMOOTH);
    glEnable(GL_TEXTURE_2D);

    // ###################################################
    int width, height;
    GLubyte *data;
    readingBmp("texture_face.bmp", &data, width, height);

    glGenTextures(1, &texture01);
    glBindTexture(GL_TEXTURE_2D, texture01);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_BGR_EXT, GL_UNSIGNED_BYTE, data);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

    // ###################################################
    glGenBuffers(PBO_COUNT, pboIds);
    glBindBuffer(GL_PIXEL_PACK_BUFFER, pboIds[0]);
    glBufferData(GL_PIXEL_PACK_BUFFER, DATA_SIZE, 0, GL_STREAM_READ);
    glBindBuffer(GL_PIXEL_PACK_BUFFER, pboIds[1]);
    glBufferData(GL_PIXEL_PACK_BUFFER, DATA_SIZE, 0, GL_STREAM_READ);
    glBindBuffer(GL_PIXEL_PACK_BUFFER, 0);

    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);

    glGenBuffers(1, &vbo_vertex);
    glBindBuffer(GL_ARRAY_BUFFER, vbo_vertex);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
    glVertexPointer(3, GL_FLOAT, 8 * sizeof(GL_FLOAT), 0);
    glEnableClientState(GL_VERTEX_ARRAY);

    glGenBuffers(1, &vbo_coord);
    glBindBuffer(GL_ARRAY_BUFFER, vbo_coord);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
    glTexCoordPointer(3, GL_FLOAT, 8 * sizeof(GL_FLOAT), (GLvoid *)(6 * sizeof(GL_FLOAT)));
    glEnableClientState(GL_TEXTURE_COORD_ARRAY);

    glGenBuffers(1, &vbo_color);
    glBindBuffer(GL_ARRAY_BUFFER, vbo_color);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
    glColorPointer(3, GL_FLOAT, 8 * sizeof(GL_FLOAT), (GLvoid *)(6 * sizeof(GL_FLOAT)));
    glEnableClientState(GL_COLOR_ARRAY);

    glGenBuffers(1, &ebo);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);
}

bool initSharedMem()
{
    drawMode = 0;

    // allocate buffers to store frames
    colorBuffer = new GLubyte[DATA_SIZE];
    memset(colorBuffer, 255, DATA_SIZE);

    return true;
}

void clearSharedMem()
{
    // deallocate frame buffer
    delete [] colorBuffer;
    colorBuffer = 0;

    // clean up PBOs
    glDeleteBuffers(PBO_COUNT, pboIds);
}

void add(unsigned char* src, int width, int height, int shift, unsigned char* dst)
{
    if(!src || !dst) {
        return;
    }

    int offset_i = 0;
    int offset_j = 0;

    if (1 == drawMode)
    {
        offset_i = 20;
        offset_j = 20;
    } else if (2 == drawMode)
    {
        offset_i = 50;
        offset_j = 50;
    }

    int value;
    for(int i = offset_i; i < offset_i + height/2; ++i)
    {
        for(int j = offset_j; j < offset_j + width/2; ++j)
        {
            value = *src + shift;
            if (value > 255) {*dst = (unsigned char)255;}

            int offset = ((i * width) + j) * 4;
            dst[offset] = src[offset] + shift;
            dst[offset + 1] = src[offset + 1] + shift;
            dst[offset + 2] = src[offset + 2] + shift;
        }
    }
}

void toOrtho()
{
    // set viewport to be the entire window
    glViewport((GLsizei)SCREEN_WIDTH, 0, (GLsizei)SCREEN_WIDTH, (GLsizei)SCREEN_HEIGHT);

    // set orthographic viewing frustum
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(0, SCREEN_WIDTH, 0, SCREEN_HEIGHT, -1, 1);

    // switch to modelview matrix in order to set scene
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
}

void toPerspective()
{
    // set viewport to be the entire window
    glViewport(0, 0, (GLsizei)SCREEN_WIDTH, (GLsizei)SCREEN_HEIGHT);

    // set perspective viewing frustum
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective(60.0f, (float)(SCREEN_WIDTH)/SCREEN_HEIGHT, 0.1f, 1000.0f); // FOV, AspectRatio, NearClip, FarClip

    // switch to modelview matrix in order to set scene
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
}

void display()
{
    static int shift = 0;
    static int index = 0;
    int nextIndex = 0;

    // brightness shift amount
    shift = ++shift % 200;

    if(1 == drawMode)
    {
        nextIndex = index;
    } else if (2 == drawMode) {
        index = (index + 1) % 2;
        nextIndex = (index + 1) % 2;
    }

    // set the framebuffer to read
    glReadBuffer(GL_FRONT);

    if (0 == drawMode)
    {
        glReadPixels(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, PIXEL_FORMAT, GL_UNSIGNED_BYTE, colorBuffer);
        add(colorBuffer, SCREEN_WIDTH, SCREEN_HEIGHT, shift, colorBuffer);
    }
    else
    {
        // copy pixels from framebuffer to PBO
        // Use offset instead of ponter.
        // OpenGL should perform asynch DMA transfer, so glReadPixels() will return immediately.
        glBindBuffer(GL_PIXEL_PACK_BUFFER, pboIds[index]);
        glReadPixels(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, PIXEL_FORMAT, GL_UNSIGNED_BYTE, 0);

        // map the PBO that contain framebuffer pixels before processing it
        glBindBuffer(GL_PIXEL_PACK_BUFFER, pboIds[nextIndex]);
        GLubyte* src = (GLubyte*)glMapBuffer(GL_PIXEL_PACK_BUFFER, GL_READ_ONLY);
        if(src)
        {
            // change brightness
            add(src, SCREEN_WIDTH, SCREEN_HEIGHT, shift, colorBuffer);
            glUnmapBuffer(GL_PIXEL_PACK_BUFFER);     // release pointer to the mapped buffer
        }

        glBindBuffer(GL_PIXEL_PACK_BUFFER, 0);
    }

    // render to the framebuffer //////////////////////////
    glDrawBuffer(GL_BACK);
    toPerspective();

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
    glTranslatef(0, 0, -CAMERA_DISTANCE);
    glPushMatrix();
    renderTri();
    glPopMatrix();

    // draw the read color buffer to the right side of the window
    toOrtho();
    glRasterPos2i(0, 0);
    glDrawPixels(SCREEN_WIDTH, SCREEN_HEIGHT, PIXEL_FORMAT, GL_UNSIGNED_BYTE, colorBuffer);

    glutSwapBuffers();
    glutPostRedisplay();
}

void reshape(int w, int h)
{
    toPerspective();
}

void keyboardCB(unsigned char key, int x, int y)
{
    switch(key)
    {
    case 27: // ESCAPE
        exit(0);
        break;

    case ' ':
        drawMode = ++drawMode % 3;
        MYLOG("current mode[%d]", drawMode);
        break;

    default:
        ;
    }
}

void exitCB()
{
    clearSharedMem();
}

int main(int argc, char **argv)
{
    initSharedMem();
    atexit(exitCB);

    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_RGBA | GLUT_DOUBLE | GLUT_ALPHA);
    glutInitWindowSize(SCREEN_WIDTH * 2, SCREEN_HEIGHT);
    glutInitWindowPosition(10, 700);
    int handle = glutCreateWindow(argv[0]);

    glutDisplayFunc(display);
    glutReshapeFunc(reshape);
    glutKeyboardFunc(keyboardCB);

    glewInit();
    if (glewIsSupported("GL_VERSION_2_1"))
    {
        MYLOG("Ready for OpenGL 2.1\n");
    }
    else
    {
        MYLOG("OpenGL 2.1 not supported\n");
        exit(1);
    }
    if (GLEW_ARB_vertex_shader && GLEW_ARB_fragment_shader && GL_EXT_geometry_shader4)
    {
        MYLOG("Ready for GLSL - vertex, fragment, and geometry units\n");
    }
    else
    {
        MYLOG("Not totally ready :( \n");
        exit(1);
    }

    initRS();

    glutMainLoop();

    return 0;
}

