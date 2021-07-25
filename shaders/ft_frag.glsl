precision highp float;
varying vec2 vTex;
void main() {
    const vec3 red = vec3(1.,0.,0.);
    const vec3 blue = vec3(0.,0.,1.);
    //vec3 color = (vTex.x < 0.0) ? red : blue;
    vec3 color = mix(red,blue,vTex.x/2.0+0.5);
    gl_FragColor = vec4(color,1.0);
}