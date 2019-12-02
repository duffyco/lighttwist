
uniform sampler2D tex;

// YUV offset
const vec3 offset = vec3(-0.0625, -0.5, -0.5);

// RGB coefficients
const vec3 Rcoeff = vec3(1.164,  0.000,  1.596);
const vec3 Gcoeff = vec3(1.164, -0.391, -0.813);
const vec3 Bcoeff = vec3(1.164,  2.018,  0.000);


void main(void) {
/*
    ivec2 size = textureSize(tex);
    vec2 ycoord = vec2( gl_TexCoord[0].x, gl_TexCoord[0].y * 2.0 / 3.0 );
    vec2 ucoord = vec2( gl_TexCoord[0].x/2.0, gl_TexCoord[0].y/3.0 + size.y*2.0/3.0 );
    vec2 vcoord = vec2( gl_TexCoord[0].x/2.0 + size.x/2.0, gl_TexCoord[0].y/3.0 + size.y*2.0/3.0 );
*/
    vec2 ycoord = vec2( gl_TexCoord[0].x, gl_TexCoord[0].y * 0.666666666);
    vec2 ucoord = vec2( gl_TexCoord[0].x/2.0, (gl_TexCoord[0].y+2.0)/3.0);
    vec2 vcoord = vec2( (gl_TexCoord[0].x+1.0)/2.0, (gl_TexCoord[0].y+2.0)/3.0 );



    vec3 yuv;
/*
    yuv.x = texture2DRect( tex, ycoord ).r;
    yuv.y = texture2DRect( tex, ucoord ).r;
    yuv.z = texture2DRect( tex, vcoord ).r;
*/
    yuv.x = texture2D( tex, ycoord ).r;
    yuv.y = texture2D( tex, ucoord ).r;
    yuv.z = texture2D( tex, vcoord ).r;

    vec3 rgb;
    yuv += offset;
    rgb.r = dot(yuv, Rcoeff);
    rgb.g = dot(yuv, Gcoeff);
    rgb.b = dot(yuv, Bcoeff);

    //gl_FragColor = vec4(rgb, 1.0);
    gl_FragColor = vec4(rgb,1.0);
}
