#version 100

attribute vec2 position;
attribute vec2 uv;

varying vec2 uv_frag;

uniform mat4 localMat;
uniform mat4 proj;
uniform int effect;

void main() {
    uv_frag = uv;    
    gl_Position = proj*(localMat * vec4(position, 0.0, 1.0));    
    
    // the depth buffer is used as the mask
    // a depth of 1 means "masked"
    //
    // depth mode success should depend on the effect value
    // before rendering.
    if (effect == 1||
        effect == 2)
        gl_Position.z = 1;
}
