/*
 * This file is part of IMGU.
 * 
 * @Copyright 2008 Université de Montréal, Laboratoire Vision3D
 *   Sébastien Roy (roys@iro.umontreal.ca)
 *   Vincent Chapdelaine-Couture (chapdelv@iro.umontreal.ca)
 *   Lucie Bélanger
 *   Jamil Draréni 
 *
 * IMGU is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 * 
 * IMGU is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with IMGU.  If not, see <http://www.gnu.org/licenses/>.
 */


#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include  "imgu.h"

#include <jpeglib.h>

// si I est NULL, ou si I est de mauvaise dimension, on alloue une nouvelle image.
// // sinon on rempli le buffer de I
//
// // compress : 0 a 100 (normalement 75 .. 90)
// int compress_jpeg(imgu *I,FILE *F,int compress);
//
//

int imguLoadJPEG(imgu **I, const char *name,unsigned char option)
{
    // ouverture et decompression du jpeg
    int xs,ys,cs;
    FILE *F;

    if (I==NULL) return -1;

    int y;
    JSAMPROW r;
    struct jpeg_decompress_struct dinfo;
    struct jpeg_error_mgr jerr;

    F=fopen(name,"rb");
    if( F==NULL )  return -1;

    dinfo.err = jpeg_std_error(&jerr);
    jpeg_create_decompress(&dinfo);
    jpeg_stdio_src(&dinfo, F);

    int pos=ftell(F);
    fseek(F,0,SEEK_END);
    int epos=ftell(F);
    fseek(F,pos,SEEK_SET);

    if( pos==epos ) { fclose(F);return -1; } // fichier vide!

    //read header and prepare for decompression
    jpeg_read_header(&dinfo,FALSE);
    //printf("k=%d\n",k);
    jpeg_start_decompress(&dinfo);

    xs=dinfo.image_width;
    ys=dinfo.image_height;
    cs=dinfo.num_components;
    //printf("Image is %d x %d x %d\n",xs,ys,cs);

    unsigned char *image_data;

    imguAllocate(I,xs,ys,cs);
    image_data=(unsigned char *)((*I)->data);

    int flipY=(InternalFormat!=ExternalFormat);

    //scanline de l'image
    for(y=0;y<ys;y++)
    {
        //printf("reading line %d of %d\n",y,ys);
        r=image_data + (flipY?(ys-1-y):y)*xs*cs*sizeof(pix_t);
        jpeg_read_scanlines(&dinfo, &r, 1);
    }

    //jpeg_finish_decompress(&dinfo);
    jpeg_destroy_decompress(&dinfo);

    // un jpeg esst toujours 8 bits, alors on 'expand' a 16 bits
    // On a r0 g0 b0 r1 g1 b1 ... (p)
    // vers r0 0 g0 0 b0 0 r1 0 g1 0 b1 0 (q)
    // ou   0 r0 0 g0 0 b0 0 r1 0 g1 0 b1
#ifndef IMGU8
    if( 1 ) {
        unsigned char *p;
        unsigned short *q;
        int off;
        for (y=0;y<ys;y++) {
            off=xs*cs-1;
            p=image_data + (y*xs*cs*sizeof(pix_t)+off);
            q=(unsigned short *)(image_data + (y*xs*cs+off)*sizeof(pix_t));
            for(;off>=0;off--) *q-- = (unsigned short)*p--;
        }
    }
#endif

    if (option==LOAD_16_BITS) imguConvert8bitTo16bit(I,(*I));
    //printf("END LOADING JPEG, now at position %d\n",ftell(F));

    fclose(F);

    return 0;
}	


int imguSaveJPEG(imgu *I,const char *Name,int compress,unsigned char option)
{
    FILE *F;
    struct jpeg_compress_struct cinfo;
    struct jpeg_error_mgr jerr;
    int r,y;
    JSAMPROW row_pointer[1];

    if (option==SAVE_16_BITS) return -1;
    if (I==NULL || I->data==NULL) return -1;

    F=fopen(Name,"wb");
    if( F==NULL ) return(-1);

    cinfo.err = jpeg_std_error(&jerr);
    jpeg_create_compress(&cinfo);


    jpeg_stdio_dest(&cinfo, F);

    cinfo.image_width = I->xs;
    cinfo.image_height = I->ys;
    cinfo.input_components = I->cs;  // normalement 3. Tester 1???

    if( I->cs == 3 )
        cinfo.in_color_space = JCS_RGB; // peut etre GRAY si cs==1??
    else if( I->cs == 1 )
        cinfo.in_color_space = JCS_GRAYSCALE; // peut etre GRAY si cs==1??
    else {
        printf("Unable to save. Must have cs = 1 or 3.\n");
        fclose(F);
        unlink(Name);
        return(-1);
    }

    jpeg_set_defaults(&cinfo);
    jpeg_set_quality(&cinfo, compress, TRUE);

    jpeg_start_compress(&cinfo, TRUE);

    // une ligne de 8 bits
    unsigned char *buf=(unsigned char *)malloc(I->xs*I->cs);
    row_pointer[0] = buf;

    int x;
    int flipY=(InternalFormat!=ExternalFormat);
    y=0;
    while(y < I->ys){
        //printf("scanline %d h=%d\n",cinfo.next_scanline,cinfo.image_height);
        // copy la ligne ushort ans uchar
        if( flipY ) r = (I->ys-1-y)*I->xs*I->cs; else r = y*I->xs*I->cs;
        for(x=0;x<I->xs*I->cs;x++)
        {
          if (option==SAVE_8_BITS_HIGH) buf[x]=(unsigned char)(I->data[r+x]>>8);
          else buf[x]=(unsigned char)(I->data[r+x]);
        }
        jpeg_write_scanlines(&cinfo, row_pointer, 1);
        y++;
    }

    jpeg_finish_compress(&cinfo);
    jpeg_destroy_compress(&cinfo);
    fclose(F);
    free(buf);
    return(0);
}




