#include "shader.h"

GLhandleARB vert, frag, prog;    //Handlers for our vertex, geometry, and fragment shaders
GLint locate;
const GLuint SCR_WIDTH = 500, SCR_HEIGHT = 500;
GLuint texture01 = 156;
float fTime = 0.0f;

GLfloat vertices[] = {
    //  ---- 位置 ----     ---- 颜色 ----  ---- 纹理坐标 ----
    -0.5f, 0.5f, 0.0f, 1.0f, 1.0f, 0.0f, 0.0f, 1.0f,  // 左上
    -0.5f, -0.5f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f,// 左下
    0.5f, 0.5f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f, 1.0f,  // 右上
    0.5f, -0.5f, 0.0f, 0.0f, 1.0f, 0.0f, 1.0f, 0.0f, // 右下
};

GLuint indices[] = {0,1,2,3};
int DRAW_MODE = 0;

GLubyte* imageData = NULL;
int IMAGE_WIDTH = 200;
int IMAGE_HEIGHT = 100;
int DATA_SIZE = IMAGE_WIDTH * IMAGE_HEIGHT;
GLuint pboId;

void initPixels()
{
    for (int i = 0; i < DATA_SIZE; i++)
    {
        int offset = i * 3;
        imageData[offset] = 0X00;
        imageData[offset + 1] = 0X68;
        imageData[offset + 2] = 0XFF;
    }
}

void updatePixels()
{
    for (int i = 0; i < DATA_SIZE; i++)
    {
        int offset = i * 3;
        imageData[offset] += 1;
        imageData[offset + 1] += 2;
        imageData[offset + 2] += 1;
    }
}

void updatePixels(GLubyte* dst)
{
    for (int i = 0; i < DATA_SIZE; i++)
    {
        int offset = i * 3;
        dst[offset] += 5;
        dst[offset + 1] += 1;
        dst[offset + 2] += 1;
    }
}

void keyboardCB(unsigned char key, int x, int y)
{
    switch(key)
    {
    case 27: // ESCAPE
        exit(0);
        break;
    case ' ':
        DRAW_MODE++;
        DRAW_MODE %= 2;
        glutPostRedisplay();
        break;
    default:
        break;
    }
}

GLuint vao = 101;
GLuint vbo_vertex = 102;
GLuint vbo_coord = 103;
GLuint ebo = 105;
void initRS()
{
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_TEXTURE_2D);

    // ###################################################
    imageData = new GLubyte[DATA_SIZE * 3];
    initPixels();

    int width, height;
    GLubyte *data;
    readingBmp("texture_face.bmp", &data, width, height);
    glGenTextures(1, &texture01);
    glBindTexture(GL_TEXTURE_2D, texture01);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_BGR, GL_UNSIGNED_BYTE, data);
    glBindTexture(GL_TEXTURE_2D, 0);

    // ###################################################
    //texture01 = loadTextureBMP("texture_face.bmp");
    // ###################################################
    glGenBuffers(1, &pboId);
    glBindBuffer(GL_PIXEL_UNPACK_BUFFER, pboId);
    glBufferData(GL_PIXEL_UNPACK_BUFFER, DATA_SIZE * 3, 0, GL_STREAM_DRAW);
    glBindBuffer(GL_PIXEL_UNPACK_BUFFER, 0);

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

    glGenBuffers(1, &ebo);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vbo_coord);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);
}

void reshape(int w, int h)
{
    float range = 50.0f;
    if (0 == h) {
        h = 1;
    }
    glViewport(0, 0, w, h);

    float as = (float)w/h;
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    
    gluPerspective(100.0f, as, 0.01, 150.0);

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
}

void renderTri()
{
    // Bind Texture
    glBindTexture(GL_TEXTURE_2D, texture01);

    if (0 == DRAW_MODE)
    {
        glTexSubImage2D(GL_TEXTURE_2D, 0, 100, 20, IMAGE_WIDTH, IMAGE_HEIGHT, GL_BGR, GL_UNSIGNED_BYTE, (GLvoid*)imageData);
        updatePixels();
    }
    else
    {
        glBindBuffer(GL_PIXEL_UNPACK_BUFFER, pboId);
        glTexSubImage2D(GL_TEXTURE_2D, 0, 20, 50, IMAGE_WIDTH, IMAGE_HEIGHT, GL_BGR, GL_UNSIGNED_BYTE, 0);

        glBindBuffer(GL_PIXEL_UNPACK_BUFFER, pboId);
        glBufferData(GL_PIXEL_UNPACK_BUFFER, DATA_SIZE * 3, 0, GL_STREAM_DRAW);
        GLubyte* ptr = (GLubyte*)glMapBuffer(GL_PIXEL_UNPACK_BUFFER, GL_WRITE_ONLY);
        if (ptr)
        {
            updatePixels(ptr);
            glUnmapBuffer(GL_PIXEL_UNPACK_BUFFER);
        }
        glBindBuffer(GL_PIXEL_UNPACK_BUFFER, 0);
    }

    glBindVertexArray(vao);
    glEnableClientState(GL_VERTEX_ARRAY);
    glEnableClientState(GL_TEXTURE_COORD_ARRAY);
    //glClearColor(1.0, 0.0, 1.0, 1.0);
    //glClear(GL_COLOR_BUFFER_BIT);
    //glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
    glClearColor(0.0, 1.0, 1.0, 1.0);
    glClear(GL_COLOR_BUFFER_BIT);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vbo_coord);
    glDrawElements(GL_TRIANGLE_STRIP, 4, GL_UNSIGNED_INT, 0);

    glDisableClientState(GL_TEXTURE_COORD_ARRAY);
    glDisableClientState(GL_VERTEX_ARRAY);
}

void display()
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glColor4f(1.0f, 1.0f, 0.0f, 1.0f);

    glLoadIdentity();
    glTranslatef(0.0f, 0.0f, -0.525f);

    renderTri();

    glutSwapBuffers();
    glutPostRedisplay();

    glUniform1f(locate, fTime);
    fTime += 0.025f;
}

void exitCB()
{
    delete [] imageData;
    imageData = NULL;

    // clean up texture
    glDeleteTextures(1, &texture01);
    glDeleteBuffers(1, &pboId);
}

int main(int argc, char *argv[])
{
    if (argc != 3) {
        MYLOG("input error\n");
        return -1;
    }

    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE|GLUT_DEPTH|GLUT_RGBA);
    glutInitWindowSize(SCR_WIDTH, SCR_HEIGHT);
    glutInitWindowPosition(0, 500);
    glutCreateWindow(argv[0]);
    
    glutDisplayFunc(display);
    glutReshapeFunc(reshape);
    glutKeyboardFunc(keyboardCB);
    atexit(exitCB);
    
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

    initRS();

    glutMainLoop();
    return 0;
}

