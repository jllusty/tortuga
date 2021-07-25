attribute vec3 vPos;
varying vec2 vTex;
void main()
{
    vTex = vec2(vPos);
    gl_Position = vec4(vPos,1.0);
}
