#version 100

attribute vec2 position;
attribute vec2 uv;

varying vec2 uv_frag;

void main() {
    uv_frag = uv;    
    gl_Position = vec4(position, 0.0, 1.0);    
}
