//
uniform sampler2D tex01;

void main()
{
    vec3 color = vec3(texture2D(tex01, gl_TexCoord[0].st));
    gl_FragColor = vec4(color * 3.0 / 255.0, 1.0);
}


