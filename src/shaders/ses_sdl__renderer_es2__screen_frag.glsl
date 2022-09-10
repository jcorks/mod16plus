#version 100

varying mediump vec2 uv_frag;

uniform sampler2D sampler;


void main() {   
    gl_FragColor = texture2D(sampler, uv_frag);
}
