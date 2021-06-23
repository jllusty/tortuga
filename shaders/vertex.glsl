attribute vec3 vPosition;
varying vec3 vTex;
uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;
void main()
{
    vTex = vec3(vPosition*2.0);
    gl_Position = projection * view * model * vec4(vPosition,1.0);
}