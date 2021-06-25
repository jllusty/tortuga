precision mediump float;
varying vec3 vTex;
varying vec3 wPos;
uniform vec4 color;
void main() 
{
    // distance from edges
    float dx = min(abs(1.0-vTex.x),abs(-1.0+vTex.x));
    float dy = min(abs(1.0-vTex.y),abs(-1.0+vTex.y));
    float dz = min(abs(1.0-vTex.z),abs(-1.0+vTex.z));
    float d = min(dx,min(dy,dz));
    gl_FragColor = (d>0.25) ? vec4(vec3(0.4,0.99,0.55),1.0) : vec4(0.,0.,0.,1.0);
}