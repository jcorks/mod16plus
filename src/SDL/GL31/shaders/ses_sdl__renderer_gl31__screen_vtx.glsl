#version 140

in vec2 position;
in vec2 uv;

out vec2 uv_frag;

void main() {
    uv_frag = uv;    
    gl_Position = vec4(position, 0.0, 1.0);    
}
