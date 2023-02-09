#version 140

in vec2 uv_frag;

uniform sampler2D sampler;



// 16-bit color with half-shade dithering



bool is32shadel(float a, float b) {
    return int(a * 32.0) < int(b * 32.0);
}

bool is64shadel(float a, float b) {
    return int(a * 64.0) < int(b * 64.0);
}


void main() {  

    vec4 shadeReal = texture2D(sampler, uv_frag);

    /// R4 G5 R4
    vec4 shade16 = vec4(
        floor(shadeReal.r * 16.0) / 16.0,
        floor(shadeReal.g * 32.0) / 32.0,
        floor(shadeReal.b * 16.0) / 16.0,
        1
    );

    int x = int(gl_FragCoord.x);
    int y = int(gl_FragCoord.y);

    // dither by ever other (half-shade) for 16bit effect
    float offset = mod(float(x/4 + y/4), 2.0);

    shade16.r += (is32shadel(shade16.r, shadeReal.r)) ? offset / 16.0 : 0.0;
    shade16.g += (is64shadel(shade16.g, shadeReal.g)) ? offset / 32.0 : 0.0;
    shade16.b += (is32shadel(shade16.b, shadeReal.b)) ? offset / 16.0 : 0.0;
        //shade16.b = 1.0;

    gl_FragColor = shade16;

}
