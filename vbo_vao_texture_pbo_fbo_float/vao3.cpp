#include "shader.h"

GLhandleARB vert, frag, prog;    //Handlers for our vertex, geometry, and fragment shaders
GLint locate;
const GLuint SCR_WIDTH = 500, SCR_HEIGHT = 500;
int texture01 = 0;
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
    texture01 = loadTextureBMP("texture_face.bmp");

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

    glBindVertexArray(vao);
    glEnableClientState(GL_VERTEX_ARRAY);
    glEnableClientState(GL_TEXTURE_COORD_ARRAY);

    if (0 == DRAW_MODE)
    {
        glClearColor(1.0, 0.0, 1.0, 1.0);
        glClear(GL_COLOR_BUFFER_BIT);
        glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
    }
    else
    {
        glClearColor(0.0, 1.0, 1.0, 1.0);
        glClear(GL_COLOR_BUFFER_BIT);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vbo_coord);
        glDrawElements(GL_TRIANGLE_STRIP, 4, GL_UNSIGNED_INT, 0);
    }

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

