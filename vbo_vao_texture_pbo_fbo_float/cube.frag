//
uniform samplerCube tex01;

void main()
{
    vec3 color = vec3(textureCube(tex01, gl_TexCoord[0].stp));
    gl_FragColor = vec4(color, 1.0);
    //gl_FragColor = vec4(1.0, 0.5, 0.2, 1.0);
}


