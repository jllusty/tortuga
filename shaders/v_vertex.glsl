attribute vec2 vPos;
varying vec2 vTex;
void main()
{
    vTex = vPos;
    gl_Position = vec4(vPos,0.0,1.0);
}
