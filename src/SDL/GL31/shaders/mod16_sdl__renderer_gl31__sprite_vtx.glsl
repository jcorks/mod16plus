#version 140

in vec3 position;
in vec2 uv;
in vec3 colorBack;
in vec3 colorMidBack;
in vec3 colorMidFront;
in vec3 colorFront;
in vec3 colorBase;

out vec2 uv_frag;
out vec3 colorBack_frag;
out vec3 colorMidBack_frag;
out vec3 colorMidFront_frag;
out vec3 colorFront_frag;
out vec3 colorBase_frag;

uniform mat4 proj;
uniform float effect;
uniform float texture;
uniform float useStaticColors;

uniform vec3 backStatic;
uniform vec3 midBackStatic;
uniform vec3 midFrontStatic;
uniform vec3 frontStatic;


void main() {
    uv_frag = uv;    
    gl_Position = proj*(vec4(position, 1.0));    

    colorBack_frag = colorBack;
    colorMidBack_frag = colorMidBack;
    colorMidFront_frag = colorMidFront;
    colorFront_frag = colorFront;
    colorBase_frag = colorBase;
    
    
    // the depth buffer is used as the mask
    // a depth of 1 means "masked"
    //
    // depth mode success should depend on the effect value
    // before rendering.
    if (effect == 1.0 ||
        effect == 2.0 )
        gl_Position.z = 1.0;
}
