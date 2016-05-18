//
//uniform float time;

attribute vec4 TexCoord0;
attribute vec4 VertCoord;

void main()
{
    gl_TexCoord[0] = gl_MultiTexCoord0;
    gl_Position = gl_ModelViewProjectionMatrix * gl_Vertex;

    //gl_TexCoord[0] = TexCoord0;
    //gl_Position = gl_ModelViewProjectionMatrix * VertCoord;
}

