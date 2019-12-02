
uniform sampler2D tex;

void main(void) {
    vec2 c = gl_TexCoord[0].st;
    vec3 rgb = texture2D( tex, c ).rgb;
    gl_FragColor = vec4(rgb, 1.0);
}

