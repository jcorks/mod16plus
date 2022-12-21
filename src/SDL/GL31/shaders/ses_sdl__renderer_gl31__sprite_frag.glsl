#version 140

in vec2 uv_frag;
in vec3 colorBack_frag;
in vec3 colorMidBack_frag;
in vec3 colorMidFront_frag;
in vec3 colorFront_frag;


out vec4 fragColor;

uniform sampler2D sampler;
uniform float effect;



void main() {   
    float a = texture2D(sampler, uv_frag).r;
    int index = int(floor(a*4.0 + 0.5));
    vec4 palette[5];
    vec4 color;

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
        
    fragColor = color;
}
