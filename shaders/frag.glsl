precision mediump float;
varying vec3 vTex;
void main() 
{
    // distance from center
    float d = distance(vTex,vec3(0.0))/sqrt(3.0);
    gl_FragColor = vec4(d, 0.0, 0.0, 1.0);
}