#version 140

in vec2 uv_frag;
in vec3 colorBack_frag;
in vec3 colorMidBack_frag;
in vec3 colorMidFront_frag;
in vec3 colorFront_frag;
in vec3 colorTop_frag;
in vec3 colorBase_frag;


out vec4 fragColor;

uniform sampler2D sampler;
uniform float effect;
uniform float texture;
uniform float useStaticColors;

uniform vec3 backStatic;
uniform vec3 midBackStatic;
uniform vec3 midFrontStatic;
uniform vec3 frontStatic;
uniform vec3 topStatic;


void main() {   

    vec4 color;

    if (texture > 0) {
        vec3 front;
        vec3 back;
        vec3 midBack;
        vec3 midFront;
        vec3 top;

        if (useStaticColors > 0) {
            front = frontStatic;
            midFront = midFrontStatic;
            midBack = midBackStatic;
            back = midBack;
            top = topStatic;
        } else {
            front = colorFront_frag;
            midFront = colorMidFront_frag;
            midBack = colorMidBack_frag;
            back = colorBack_frag;
            top = colorTop_frag;
        }


        float a = texture2D(sampler, uv_frag).r;
        int index = int(floor(a*5.0 + 0.5));
        vec4 palette[6];

        palette[0] = vec4(0, 0, 0, 0);
        palette[1] = vec4(back, 1.0);
        palette[2] = vec4(midBack, 1.0);
        palette[3] = vec4(midFront, 1.0);
        palette[4] = vec4(front, 1.0);
        palette[5] = vec4(top, 1.0);

        color = vec4(colorBase_frag, 1) * palette[index];

    } else {
        color = vec4(colorBase_frag, 1);
    }

    

    // additive blending should be specified for the blend 
    // effects prior to rendering.    
    if (effect == 1.0)
        color = vec4(0, 0, 0, 0);

    //color.a = 1.0;
    //color.g = 1.0;
        
    fragColor = color;
}
