precision mediump float;
varying vec2 vTex;
uniform sampler2D lineData;
uniform float level;
uniform float scaling;

uniform int numLines;

vec4 sampleLines(vec3 uv) {
    //vec3 rt = 255.*vec3(texture2D(lineData, vec2(1.5/64.0,2.5/64.0)));
    //rt -= vec3(32.,32.,32.);
    //vec3 rt = vec3(0.,0.,0.);
    //if(length(uv-rt) <= 2.) return vec4(1.0);
    //else return vec4(0.);
    
    for(int k = 0; k < numLines; ++k) {
        int ii = int(mod(float(k),64.0));
        int jj = k / 64;
        float j = (float(ii)+0.5)/64.0, i = (2.0*float(jj) + 0.5)/64.0;
        vec3 r0 = 255.*vec3(texture2D(lineData, vec2(i,j)));
        r0 = r0 - vec3(32.,32.,32.);
        vec3 rf = 255.*vec3(texture2D(lineData, vec2(i+0.5/64.0,j)));
        rf = rf - vec3(32.,32.,32.);
        //vec3 rt = vec3(0.,-19.,30.);
        //if(distance(uv,r0) < 5.) return vec4(1.);
        float tH = dot(uv-r0,rf-r0)/dot(rf-r0,rf-r0);
        if((tH<0.)||(tH>1.)) continue; // return vec4(0.);
        vec3 pL = r0 + tH*(rf-r0);
        float d = sqrt(dot(pL-uv,pL-uv));
        if(d <= 1.0) return vec4(1.);
    }
    return vec4(0.);
}

void main() 
{
    // distance from center
    vec3 aPos = 32.0*vec3(vTex,level);
    gl_FragColor = sampleLines(aPos);
    //gl_FragColor = vec4(level/2.+1.,0.,0.,1.);
    //gl_FragColor = vec4(d, 0.5, 0.5, 1.0);
}