#include "shader.h"

// GLUT CALLBACK functions //
void displayCB();
void reshapeCB(int w, int h);
void exitCB();

bool initSharedMem();
void clearSharedMem();
void draw();
// FBO utils
bool checkFramebufferStatus();

// constants
const int   SCREEN_WIDTH    = 300;
const int   SCREEN_HEIGHT   = 300;
const float CAMERA_DISTANCE = 6.0f;

const int   TEXTURE_WIDTH   = 512;
const int   TEXTURE_HEIGHT  = 512;

GLuint fboId;
GLuint textureId;
GLuint rboId;

int screenWidth;
int screenHeight;
float cameraDistance;
bool fboSupported;
bool fboUsed;
float angle = 0.0f;

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

////////////////////////////////////////////
// draw a textured cube with GL_TRIANGLES
////////////////////////////////////////////
void draw()
{
    glBindTexture(GL_TEXTURE_2D, textureId);
    glBegin(GL_TRIANGLES);
        // front faces
        glNormal3f(0,0,1);
        // face v0-v1-v2
        glTexCoord2f(1,1);  glVertex3f(2,2,2);
        glTexCoord2f(0,1);  glVertex3f(-2,2,2);
        glTexCoord2f(0,0);  glVertex3f(-2,-2,2);
        // face v2-v3-v0
        glTexCoord2f(0,0);  glVertex3f(-2,-2,2);
        glTexCoord2f(1,0);  glVertex3f(2,-2,2);
        glTexCoord2f(1,1);  glVertex3f(2,2,2);
    glEnd();
    glBindTexture(GL_TEXTURE_2D, 0);
}

void initRS()
{
    glEnable(GL_TEXTURE_2D);

    // create a texture object
    glGenTextures(1, &textureId);
    glBindTexture(GL_TEXTURE_2D, textureId);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, TEXTURE_WIDTH, TEXTURE_HEIGHT, 0, GL_RGBA, GL_UNSIGNED_BYTE, 0);
    glBindTexture(GL_TEXTURE_2D, 0);

    // create a framebuffer object, you need to delete them when program exits.
    glGenFramebuffers(1, &fboId);
    glBindFramebuffer(GL_FRAMEBUFFER, fboId);

    // create a renderbuffer object to store depth info
    // NOTE: A depth renderable image should be attached the FBO for depth test.
    // If we don't attach a depth renderable image to the FBO, then

    // the rendering output will be corrupted because of missing depth test.
    // If you also need stencil test for your rendering, then you must
    // attach additional image to the stencil attachement point, too.
    glGenRenderbuffers(1, &rboId);
    glBindRenderbuffer(GL_RENDERBUFFER, rboId);

    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, TEXTURE_WIDTH, TEXTURE_HEIGHT);
    glBindRenderbuffer(GL_RENDERBUFFER, 0);

    // attach a texture to FBO color attachement point
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, textureId, 0);
    // attach a renderbuffer to depth attachment point
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, rboId);

    // check FBO status

    bool status = checkFramebufferStatus();
    if(!status)
        fboUsed = false;

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

///////////////////////////////////////////////////////////////////////////////
int main(int argc, char **argv)
{
    // init global vars
    initSharedMem();

    // register exit callback
    atexit(exitCB);

    // init GLUT and GL
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_RGBA | GLUT_DOUBLE | GLUT_DEPTH | GLUT_STENCIL);
    glutInitWindowSize(screenWidth, screenHeight);
    glutInitWindowPosition(200, 200);
    int handle = glutCreateWindow(argv[0]);
    glutDisplayFunc(displayCB);
    glutReshapeFunc(reshapeCB);
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

///////////////////////////////////////////////////////////////////////////////
// initialize global variables
///////////////////////////////////////////////////////////////////////////////
bool initSharedMem()
{
    screenWidth = SCREEN_WIDTH;
    screenHeight = SCREEN_HEIGHT;

    cameraDistance = CAMERA_DISTANCE;
    fboId = rboId = textureId = 0;
    fboSupported = fboUsed = false;
    return true;
}

///////////////////////////////////////////////////////////////////////////////
// clean up global variables
///////////////////////////////////////////////////////////////////////////////
void clearSharedMem()
{
    glDeleteTextures(1, &textureId);
    textureId = 0;

    // clean up FBO, RBO
    if(fboSupported)
    {
        glDeleteFramebuffers(1, &fboId);
        fboId = 0;
        glDeleteRenderbuffers(1, &rboId);
        rboId = 0;
    }
}

///////////////////////////////////////////////////////////////////////////////
// check FBO completeness
///////////////////////////////////////////////////////////////////////////////
bool checkFramebufferStatus()
{
    // check FBO status
    GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
    return (GL_FRAMEBUFFER_COMPLETE == status);
}

//=============================================================================
// CALLBACKS
//=============================================================================

void displayCB()
{
    angle += 1.225;
    // adjust viewport and projection matrix to texture dimension
    glViewport(0, 0, TEXTURE_WIDTH, TEXTURE_HEIGHT);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective(60.0f, (float)(TEXTURE_WIDTH)/TEXTURE_HEIGHT, 1.0f, 100.0f);
    glMatrixMode(GL_MODELVIEW);
    // camera transform
    glLoadIdentity();
    glTranslatef(0, 0, -CAMERA_DISTANCE);

    // render directly to a texture
    // set the rendering destination to FBO
    glBindFramebuffer(GL_FRAMEBUFFER, fboId);

    // clear buffer
    glClearColor(0, 0, 1, 1);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // draw a rotating teapot at the origin
    glPushMatrix();
    glRotatef(angle, 0, 1, 0);
    glutWireTeapot(2.0f);
    glPopMatrix();
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    // back to normal viewport and projection matrix
    glViewport(0, 0, screenWidth, screenHeight);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective(60.0f, (float)(screenWidth)/screenHeight, 1.0f, 1000.0f);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    // tramsform camera
    glTranslatef(0, 0, -cameraDistance);

    // clear framebuffer
    glClearColor(0, 0, 0, 0);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

    glPushMatrix();
    // draw a cube with the dynamic texture
    draw();
    glPopMatrix();
    glutSwapBuffers();
    glutPostRedisplay();
}

void reshapeCB(int width, int height)
{
    screenWidth = width;
    screenHeight = height;

    // set viewport to be the entire window
    glViewport(0, 0, (GLsizei)screenWidth, (GLsizei)screenHeight);
    // set perspective viewing frustum
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective(60.0f, (float)(screenWidth)/screenHeight, 1.0f, 1000.0f);

    // switch to modelview matrix in order to set scene
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
}

void exitCB()
{
    clearSharedMem();
}
