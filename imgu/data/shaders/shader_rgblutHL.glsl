

/////////////////////////////////////////
// hi-lo lut texture for matrox 8 bit textures

uniform sampler2D tex;
uniform sampler2D lutH;
uniform sampler2D lutL;

void main(void) {
    vec2 t = vec2( gl_TexCoord[0].x, gl_TexCoord[0].y );

    vec2 dispH=texture2D( lutH, t.xy).rg;
    vec2 dispL=texture2D( lutL, t.xy).rg;
    vec2 disp=dispH+dispL/256.0;

    vec4 col = texture2D( tex,disp ).rgba;
    gl_FragColor = col;
}


