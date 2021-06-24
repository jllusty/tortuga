precision mediump float;
varying vec2 vTex;
uniform sampler2D lineData;
uniform float level;

const int numLines = 1;

vec4 sampleLines(vec3 uv) {
    vec2 texCoords = vec2(uv);
    vec4 t = texture2D(lineData, texCoords);
    return vec4(t.xyz,1.0);
}

void main() 
{
    // distance from center
    vec3 aPos = vec3(vTex,level);
    float d = length(aPos);
    gl_FragColor = (d < 0.5) ? sampleLines(aPos) : vec4(0.0);
    //gl_FragColor = vec4(d, 0.5, 0.5, 1.0);
}