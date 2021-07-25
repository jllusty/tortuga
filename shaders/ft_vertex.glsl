attribute vec3 vPosition;
varying vec2 vTex;
void main() {
    gl_Position = vec4(vPosition,1.);
    vTex = vec2(vPosition);
}
