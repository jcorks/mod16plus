#version 100

varying mediump vec2 uv_frag;

uniform sampler2D sampler;
uniform mediump vec4 palette[5];
uniform int effect;


void main() {   
    lowp float a = texture2D(sampler, uv_frag).a;
    int index = int(clamp(float(a*255.0), 0.0, 4.0));

    lowp vec4 color;

    color = palette[index];

    // additive blending should be specified for the blend 
    // effects prior to rendering.    
    if (effect == 1)
        color = vec4(0, 0, 0, 0);

    //color.a = 1.0;
    //color.g = 1.0;
        
    gl_FragColor = color;
}
