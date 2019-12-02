
uniform sampler2D tex;
uniform sampler2D lut;

void main(void) {
    vec3 disp=texture2D( lut, gl_TexCoord[0].xy).rgb;

/*
    vec2 sz = textureSize2D( tex, 0 );
    sz = 1.0 / sz;
    float dx = 0.5 * sz.x;
    float dy = 0.5 * sz.y;

    vec4 col1 = texture2D( tex,disp + vec2( dx, dy )  ).rgba;
    vec4 col2 = texture2D( tex,disp + vec2( dx,-dy )  ).rgba;
    vec4 col3 = texture2D( tex,disp + vec2(-dx, dy )  ).rgba;
    vec4 col4 = texture2D( tex,disp + vec2(-dx,-dy )  ).rgba;
    vec4 col = (col1 + col2 + col3 + col4)/4.0;
*/

    vec4 col = texture2D( tex,disp.rg ).rgba;

/*
    gl_FragColor.r = col.r;
    gl_FragColor.g = col.g;
    gl_FragColor.b = col.b;
    gl_FragColor.a = col.a;
*/
    gl_FragColor.rgb = col.rgb * disp.b;
    gl_FragColor.a = 1.0;
}

