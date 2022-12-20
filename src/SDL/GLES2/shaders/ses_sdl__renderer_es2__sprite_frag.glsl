#version 100

varying mediump vec2 uv_frag;
varying mediump vec3 colorBack_frag;
varying mediump vec3 colorMidBack_frag;
varying mediump vec3 colorMidFront_frag;
varying mediump vec3 colorFront_frag;

uniform sampler2D sampler;
uniform mediump float effect;



void main() {   
    lowp float a = texture2D(sampler, uv_frag).a;
    int index = int(floor(a*4.0 + 0.5));
    mediump vec4 palette[5];
    lowp vec4 color;

    palette[0] = vec4(0, 0, 0, 0);
    palette[1] = vec4(colorBack_frag, 1.0);
    palette[2] = vec4(colorMidBack_frag, 1.0);
    palette[3] = vec4(colorMidFront_frag, 1.0);
    palette[4] = vec4(colorFront_frag, 1.0);


    color = palette[index];

    // additive blending should be specified for the blend 
    // effects prior to rendering.    
    if (effect == 1.0)
        color = vec4(0, 0, 0, 0);

    //color.a = 1.0;
    //color.g = 1.0;
        
    gl_FragColor = color;
}
