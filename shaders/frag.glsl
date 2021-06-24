precision mediump float;
varying vec3 vTex;
uniform vec4 color;
void main() 
{
    // distance from center
    float d = distance(vTex,vec3(0.0))/sqrt(3.0);
    gl_FragColor = (d>0.5) ? color : vec4(0.);
}