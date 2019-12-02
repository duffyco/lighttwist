uniform sampler2D mask0;
uniform sampler2D brush;
uniform samplerRect canvas;
uniform vec4 brushColor;
uniform vec4 canvasColor;
uniform float cw;
uniform float ch;
uniform int mode;

void main (void)
{

    float m0 = texture2D( mask0, gl_TexCoord[0].xy ).r;
    vec4 c0 = texture2D( brush, gl_TexCoord[0].xy );

    vec4 col = vec4(1,1,1,1);

    if (mode == 4) {
        // ********* 'brush' background .. img or video or whatever... canvas is revealed **********

        vec4 cnv = textureRect( canvas, vec2(gl_TexCoord[0].x * cw, gl_TexCoord[0].y * ch) );

        ///vec4 a = c0 * m0;
        //  float m = max(m0, max(m1, m2));
        // col = cnv * (1-m) + a+b+c;
        col = cnv * m0 + c0 *(1-m0);

    } else if (mode == 3) {
        // ********* 'canvas' background .. img or video or whatever **********

        vec4 cnv = textureRect( canvas, vec2(gl_TexCoord[0].x * cw, gl_TexCoord[0].y * ch) );

        vec4 a = c0 * m0;
        //vec4 b = c1 + (cnv - c1) * (cnv - m1);
        //vec4 c = c2 + (cnv - c2) * (cnv - m2);
        //float m = max(m0, max(m1, m2));

        col = cnv * (1-m0) + a; //+b+c;

    } else if (mode == 2) {
        // ******* use brush/canvasColor ***********************************
        col = m0 * brushColor + (1.0-m0) * canvasColor;

    } else if (mode == 1) {
        // ******** white background ***************************************

        vec4 a = c0 + (1.0 - c0) * (1.0 - m0);
        // vec4 b = c1 + (1.0 - c1) * (1.0 - m1);
        //vec4 c = c2 + (1.0 - c2) * (1.0 - m2);

        /////////////////////////float m = max(m0, max(m1, m2));
        /// float m = m0*m1*m2;

        col = a; // * b * c;


    } else {
        // ********* mode = 0 : black background **************************
        col = m0*c0; // m0*c0; // + m1*c1 + m2*c2;
    }

    col.a = 1.0;
    gl_FragColor = col;


}
