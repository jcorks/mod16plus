#version 140

in vec2 position;
in vec2 uv;
in vec3 colorBack;
in vec3 colorMidBack;
in vec3 colorMidFront;
in vec3 colorFront;

out vec2 uv_frag;
out vec3 colorBack_frag;
out vec3 colorMidBack_frag;
out vec3 colorMidFront_frag;
out vec3 colorFront_frag;

uniform mat4 proj;
uniform float effect;

void main() {
    uv_frag = uv;    
    gl_Position = proj*(vec4(position, 0.0, 1.0));    

    colorBack_frag = colorBack;
    colorMidBack_frag = colorMidBack;
    colorMidFront_frag = colorMidFront;
    colorFront_frag = colorFront;
    
    
    // the depth buffer is used as the mask
    // a depth of 1 means "masked"
    //
    // depth mode success should depend on the effect value
    // before rendering.
    if (effect == 1.0 ||
        effect == 2.0 )
        gl_Position.z = 1.0;
}
