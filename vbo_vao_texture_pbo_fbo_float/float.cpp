#include "shader.h"

GLhandleARB vert, frag, prog;    //Handlers for our vertex, geometry, and fragment shaders
GLint locate;
float fTime = 0.0f;

const float PIC_WIDTH_HALF = 136.0f;
const float PIC_HEIGHT_HALF = 136.0f;
GLint mWidth = 0;
GLint mHeight = 0;
GLubyte *ori_pixel = NULL;
GLenum const_format = GL_BGR_EXT;
GLuint textureID = 101;

GLubyte* loadTextureBMP_info(const char *file, GLint &width, GLint &height)
{
    GLint total_bytes;
    GLubyte* pixels = 0;

    FILE* pFile = fopen(file, "rb");
    if(pFile == 0)
    {
        printf("Can not open file %s...", file);
        return 0;
    }

    fseek(pFile, 0x0012, SEEK_SET);
    fread(&width, 4, 1, pFile);
    fread(&height, 4, 1, pFile);
    fseek(pFile, BMP_Header_Length, SEEK_SET);

    {
        GLint line_bytes = width * 3;
        while(line_bytes % 4 != 0)
        {
            ++line_bytes;
        }
        total_bytes = line_bytes * height;
    }

    pixels = (GLubyte*)malloc(total_bytes);
    if(pixels == 0)
    {
        printf("Pixeles in file %s is NULL...", file);
        fclose(pFile);
        return 0;
    }

    if(fread(pixels, total_bytes, 1, pFile) <= 0)
    {
        free(pixels);
        fclose(pFile);
        return 0;
    }
    return pixels;
}

void setupRS()
{
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_TEXTURE_2D);

    ori_pixel = loadTextureBMP_info("texture00.bmp", mWidth, mHeight);
    MYLOG("ori_pixel[%p], mWidth[%d], mHeight[%d]", ori_pixel, mWidth, mHeight);

/*
    glGenTextures(1, &textureID);
    glBindTexture(GL_TEXTURE_2D, textureID);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, mWidth, mHeight, 0, GL_BGR_EXT, GL_UNSIGNED_BYTE, ori_pixel);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
*/

    float *float_pixel = (float *) malloc(mWidth * mHeight * 3 * sizeof(float));
    if (!float_pixel) {
        MYLOG("out of memory\n");
        exit(0);
    }

    for (int i = 0; i < mWidth * mHeight * 3; i++) {
        float_pixel[i] = 1.0f * ori_pixel[i];
    }

    glGenTextures(1, &textureID);
    glBindTexture(GL_TEXTURE_2D, textureID);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F_ARB, mWidth, mHeight, 0, GL_BGR_EXT, GL_FLOAT, float_pixel);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
}

void reshape(int w, int h)
{
    if (0 == h)
        h = 1;
    glViewport(0,0,w,h);

    float as = (float)w/h;
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();

    gluPerspective(100.0f, as, 1, 150);

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
}

void drawRect()
{
    glBegin(GL_POLYGON);
    glTexCoord2f(0, 0);   glVertex2f(-PIC_WIDTH_HALF, -PIC_WIDTH_HALF);
    glTexCoord2f(1, 0);   glVertex2f( PIC_WIDTH_HALF, -PIC_WIDTH_HALF);
    glTexCoord2f(1, 1);   glVertex2f( PIC_WIDTH_HALF,  PIC_WIDTH_HALF);
    glTexCoord2f(0, 1);   glVertex2f(-PIC_WIDTH_HALF,  PIC_WIDTH_HALF);
    glEnd();
}

void display()
{
    glColor4f(1.0f,1.0f,0.0f,1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    //glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    glUseProgram(prog);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, textureID);

    glLoadIdentity();

    glTranslatef(0.0f, 0.0f, -120.0f);
    drawRect();

    glutSwapBuffers();
    glutPostRedisplay();

    glUniform1f(locate, fTime);
    fTime += 0.025f;
}
void keyboardCB(unsigned char key, int x, int y)
{
    switch(key)
    {
    case 27: // ESCAPE
        exit(0);
        break;
    default:
        ;
    }
}

int main(int argc, char *argv[])
{
    if (argc != 3) {
        MYLOG("input error\n");
        return -1;
    }

    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE|GLUT_DEPTH|GLUT_RGBA);
    glutInitWindowSize(500, 500);
    glutInitWindowPosition(0, 500);
    glutCreateWindow(argv[0]);
    
    glutDisplayFunc(display);
    glutReshapeFunc(reshape);
    glutKeyboardFunc(keyboardCB);

    setupRS();

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

    setShaders(argv[1], argv[2], vert, frag, prog);
    glUniform1iARB(glGetUniformLocationARB(prog, "tex01"), 0);
    locate = glGetUniformLocation(prog, "time");

    glutMainLoop();
    return 0;
}
