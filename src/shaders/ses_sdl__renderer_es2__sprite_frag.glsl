#version 100

varying mediump vec2 uv_frag;

uniform sampler2D sampler;
uniform vec4 palette[5];
uniform int effect;


void main() {   
    float a = texture2D(sampler, uv_frag).a;
    gl_FragColor = palette[int(clamp(a*255, 0, 4)];

    // additive blending should be specified for the blend 
    // effects prior to rendering.    
    if (effect == 1)
        gl_FragColor = vec4(0, 0, 0, 0);
        
}
