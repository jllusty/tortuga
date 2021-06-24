precision mediump float;
varying vec2 vTex;
uniform float level;
void main() 
{
    // distance from center
    float d = length(vec3(vTex,level));
    d = (d < 0.5) ? 1.0 : 0.0;
    gl_FragColor = vec4(d, 0.5, 0.5, 1.0);
}