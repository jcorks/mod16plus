#version 100

attribute vec2 position;
attribute vec2 uv;
attribute vec3 colorBack;
attribute vec3 colorMidBack;
attribute vec3 colorMidFront;
attribute vec3 colorFront;

varying vec2 uv_frag;
varying vec3 colorBack_frag;
varying vec3 colorMidBack_frag;
varying vec3 colorMidFront_frag;
varying vec3 colorFront_frag;

uniform mat4 proj;
uniform mediump float effect;

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
