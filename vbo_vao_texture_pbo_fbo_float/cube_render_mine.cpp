#include "shader.h"

GLhandleARB vert, frag, prog;    //Handlers for our vertex, geometry, and fragment shaders
GLint locate;
const GLuint SCR_WIDTH = 500, SCR_HEIGHT = 500;
GLuint texture01 = 0;
float fTime = 0.0f;

#define eps1 1.0 /*0.99*/
#define br   2.0  /* box radius */

static const GLfloat tex_coords[] = {
   /* +X side */
   -1.0,  eps1, -eps1,
   -1.0,  eps1,  eps1,
   -1.0, -eps1,  eps1,
   -1.0, -eps1, -eps1,

   /* -X side */
   1.0, -eps1, -eps1,
   1.0, -eps1,  eps1,
   1.0,  eps1,  eps1,
   1.0,  eps1, -eps1,

   /* +Y side */
   -eps1, 1.0, -eps1,
   -eps1, 1.0,  eps1,
   eps1, 1.0,  eps1,
   eps1, 1.0, -eps1,

   /* -Y side */
   -eps1, -1.0, -eps1,
   -eps1, -1.0,  eps1,
   eps1, -1.0,  eps1,
   eps1, -1.0, -eps1,

   /* +Z side */
   -eps1,  eps1, 1.0,
   eps1,  eps1, 1.0,
   eps1, -eps1, 1.0,
   -eps1, -eps1, 1.0,

   /* -Z side */
   -eps1, -eps1, -1.0,
   eps1, -eps1, -1.0,
   eps1,  eps1, -1.0,
   -eps1,  eps1, -1.0,
};

static const GLfloat vtx_coords[] = {
   /* +X side */
   br, -br, -br,
   br, -br,  br,
   br,  br,  br,
   br,  br, -br,

   /* -X side */
   -br,  br, -br,
   -br,  br,  br,
   -br, -br,  br,
   -br, -br, -br,

   /* +Y side */
   -br,  br, -br,
   -br,  br,  br,
    br,  br,  br,
    br,  br, -br,

   /* -Y side */
   -br, -br, -br,
   -br, -br,  br,
    br, -br,  br,
    br, -br, -br,

   /* +Z side */
    br, -br, br,
   -br, -br, br,
   -br,  br, br,
    br,  br, br,

   /* -Z side */
    br,  br, -br,
   -br,  br, -br,
   -br, -br, -br,
    br, -br, -br,
};

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
void initRS()
{
    glEnable(GL_DEPTH_TEST);
    //glEnable(GL_TEXTURE_2D);
    glEnable(GL_TEXTURE_CUBE_MAP_ARB);

    // ###################################################
    int width, height;
    GLubyte *data;
    glGenTextures(1, &texture01);
    glBindTexture(GL_TEXTURE_CUBE_MAP_ARB, texture01);
    readingBmp("cube_left.bmp", &data, width, height);
    glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X_ARB, 0, GL_RGB, width, height, 0, GL_BGR_EXT, GL_UNSIGNED_BYTE, data);
    readingBmp("cube_right.bmp", &data, width, height);
    glTexImage2D(GL_TEXTURE_CUBE_MAP_NEGATIVE_X_ARB, 0, GL_RGB, width, height, 0, GL_BGR_EXT, GL_UNSIGNED_BYTE, data);
    readingBmp("cube_top.bmp", &data, width, height);
    glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_Y_ARB, 0, GL_RGB, width, height, 0, GL_BGR_EXT, GL_UNSIGNED_BYTE, data);
    readingBmp("cube_bottom.bmp", &data, width, height);
    glTexImage2D(GL_TEXTURE_CUBE_MAP_NEGATIVE_Y_ARB, 0, GL_RGB, width, height, 0, GL_BGR_EXT, GL_UNSIGNED_BYTE, data);
    readingBmp("cube_back.bmp", &data, width, height);
    glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_Z_ARB, 0, GL_RGB, width, height, 0, GL_BGR_EXT, GL_UNSIGNED_BYTE, data);
    readingBmp("cube_front.bmp", &data, width, height);
    glTexImage2D(GL_TEXTURE_CUBE_MAP_NEGATIVE_Z_ARB, 0, GL_RGB, width, height, 0, GL_BGR_EXT, GL_UNSIGNED_BYTE, data);

    glTexParameteri(GL_TEXTURE_CUBE_MAP_ARB, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP_ARB, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP_ARB, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_CUBE_MAP_ARB, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_CUBE_MAP_ARB, GL_TEXTURE_WRAP_R, GL_REPEAT);

    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);

    glGenBuffers(1, &vbo_vertex);
    glBindBuffer(GL_ARRAY_BUFFER, vbo_vertex);
    glBufferData(GL_ARRAY_BUFFER, sizeof(tex_coords), tex_coords, GL_STATIC_DRAW);
    glVertexPointer(3, GL_FLOAT, 3 * sizeof(GLfloat), 0);
    glEnableClientState(GL_VERTEX_ARRAY);

    glGenBuffers(1, &vbo_coord);
    glBindBuffer(GL_ARRAY_BUFFER, vbo_coord);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vtx_coords), vtx_coords, GL_STATIC_DRAW);
    glTexCoordPointer(3, GL_FLOAT, 3 * sizeof(GLfloat), 0);
    glEnableClientState(GL_TEXTURE_COORD_ARRAY);
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
    
    gluPerspective(120.0f, as, 0.01, 150.0);

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
}

float mAng = 0.0f;
void renderTri()
{
    // Bind Texture
    //glBindTexture(GL_TEXTURE_CUBE_MAP_ARB, texture01);

    glBindVertexArray(vao);
    glEnableClientState(GL_VERTEX_ARRAY);
    glEnableClientState(GL_TEXTURE_COORD_ARRAY);

    glPushMatrix();
    glDrawArrays(GL_QUADS, 0, 24);
    //glutSolidSphere(0.0125, 20, 20);
    glPopMatrix();

    glPushMatrix();
    glTranslatef(0.0, -0.55, -0.725);
    glRotatef(mAng, 1.0, 0.25, 0.65);
    glScalef(0.15, 0.15, 0.15);
    glDrawArrays(GL_QUADS, 0, 24);
    glPopMatrix();

    glDisableClientState(GL_TEXTURE_COORD_ARRAY);
    glDisableClientState(GL_VERTEX_ARRAY);

    mAng += 0.125;
}

void display()
{
    glClearColor(0.0f, 0.0f, 0.0125f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glLoadIdentity();

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

    initRS();

    glutMainLoop();
    return 0;
}

