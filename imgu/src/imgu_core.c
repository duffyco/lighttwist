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

/**
 * @mainpage IMGU image library
 * @author Sebastien Roy
 * @author Vincent Chapdelaine-Couture
 * @version 1.1
 * @date 2007
 * @bug aucun
 * @warning un petit avertissement
 *
 * IMGU is a lightweight 16-bit image library written in C that aims to provide useful image analysis functions,
 * especially in the field of Computer Vision. 
 * Although this library doesn't require any external packages, it is strongly recommended to install
 * the PNG library to enable saving multiple images in a single file and saving text in images (IMGU takes advantage of this feature in a lot of
 * situations; storing SIFT features, for instance).
 *
 * IMGU was build to manage lists of images of arbitrary size, with careful attention to minimize the number of memory allocations
 * (see \ref imgugeneral to get a brief overview of the workflow). IMGU usually works on images with 1,2,3 or 4 color channels. However, it
 * also provides means for fast temporal analysis by giving the option of concatenating a list of images into
 * a single image (the color channels are simply stacked). 
 *
 * IMGU also supports operations on complex data. The complex format used is compatible with the (optional)
 * FFTW library. Because image data is 16-bit, complex (or real) data can 
 * be saved as images without introducing much rounding errors.
 *
 * Various other operations like convolution or windowing are also supported in both image and complex formats.
 * The library also supports multiscale structures.
 *
 * Finally, a camera structure is also available.
 *
 * Uses mplayer for capturing, OSG for visualising.
 *
 */

/**
 * @file
 *
 * This file contains the core functions of the library.
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#ifdef HAVE_LIBPNG
  #include <png.h>
#endif
#ifdef HAVE_ZLIB
  #include <zlib.h>
#endif

#include "imgu.h"

//////////////////////////////////////////////
//////////////////////////////////////////////
//////////////////////////////////////////////

#define INIT_NB_TEXT	20
#define INIT_TEXT_STEP	40

/**
 * internal and external representation are only
 * used when loading, saving, and creating textures
 */
int InternalFormat=Y_ORIGIN_IS_AT_TOP;
int ExternalFormat=Y_ORIGIN_IS_AT_TOP;



/**
 * Internal allocation of more space for text
 */
//static
int imguAllocateMoreText(imgu *I,int n);

/**
 * Change a (key,text) pair inside an image
 */
//static
int imguSetText(imgu *I,const char *key,const char *txt,int i);

// pour le debuggage
char *Ctype[]={ "GRAY", "?", "RGB", "PALETTE", "GA", "?", "RGBA", "?" };


//////////////////////////////////////////////////
//////////////////////////////////////////////////
////
//// Global settigns functions
////
//////////////////////////////////////////////////
//////////////////////////////////////////////////

/** Changes the internal row-format of an image.
 *
 * This function defines where is the Y origin of the image in memory.
 * By default, it is Y_ORIGIN_IS_AT_TOP, which means that we assume that the row Y=0,
 * which is at the beginning of memory, refers to the top of the image.
 *
 * @param[in] flag One of Y_ORIGIN_IS_AT_TOP or Y_ORIGIN_IS_AT_BOTTOM
 */
void imguSetInternalFormat(int flag) { InternalFormat=flag; }

/** Change the external row-format of an image.
 *
 * This functions defines the Y origin of an image, for the purpose of saving the image.
 * It does not affect how the image is stored.
 * Default value is Y_ORIGIN_IS_AT_TOP
 *
 * @param[in] flag One of Y_ORIGIN_IS_AT_TOP or Y_ORIGIN_IS_AT_BOTTOM
 */
void imguSetExternalFormat(int flag) { ExternalFormat=flag; }


//////////////////////////////////////////////////

// PNG is network byte order (most significant first) (little-endian)
// donc HL avec n=H*256+L
// retourne 1 si on est low-hi, et 0 si on est hi-low
static int is_big_endian(void)
{
    unsigned short v;
    unsigned char *b;
    b=(unsigned char *)&v;
    b[0]=12;
    b[1]=234;
    //printf("TEST v=%d  (normal=%d swap=%d) -> %d\n",v,b[0]*256+b[1],b[1]*256+b[0], (v==b[0]*256+b[1])?0:1);
    if( v==b[0]*256+b[1] ) return(0); /* Hi-Low */
    return(1); /* Low-Hi */
}



#define SIG_CHECK_SIZE 4


static int read_sig_buf(FILE *ifP)
{
#ifndef HAVE_LIBPNG
    fprintf(stderr,"PNG LIBRARY NOT AVAILABLE!\n");
    return(-1);
#else
    unsigned char sig_buf[SIG_CHECK_SIZE];
    size_t bytesRead;

    bytesRead = fread(sig_buf, 1, SIG_CHECK_SIZE, ifP);
    if (bytesRead != SIG_CHECK_SIZE) {
        //fprintf(stderr,"input file is empty or too short %d!=%d\n",bytesRead,SIG_CHECK_SIZE);
        return(-1);
    }

    if (png_sig_cmp(sig_buf, (png_size_t) 0, (png_size_t) SIG_CHECK_SIZE) != 0) {
        fprintf(stderr,"input file is not a PNG file\n");
        return(-2);
    }
    return(0);
#endif
}


#ifdef SKIP
static void readpng_version_info(void)
{
#ifndef HAVE_LIBPNG
    fprintf(stderr,"PNG LIBRARY NOT AVAILABLE!\n");
    return;
#else
    fprintf(stderr, "   Compiled with libpng %s; using libpng %s.\n",
            PNG_LIBPNG_VER_STRING, png_libpng_ver);
    fprintf(stderr, "   Compiled with zlib %s; using zlib %s.\n",
            ZLIB_VERSION, zlib_version);
#endif
}


static void writepng_version_info(void)
{
#ifndef HAVE_LIBPNG
    fprintf(stderr,"PNG LIBRARY NOT AVAILABLE!\n");
    return;
#else
    fprintf(stderr, "   Compiled with libpng %s; using libpng %s.\n",
            PNG_LIBPNG_VER_STRING, png_libpng_ver);
    fprintf(stderr, "   Compiled with zlib %s; using zlib %s.\n",
            ZLIB_VERSION, zlib_version);
#endif
}
#endif


/**
 * Output as a raw 8bit stream
 *
 * @attention All 16bit values are shifted to 8 bits
 *
 * @param[in] I Image
 * @param[in] F File descriptor. It will not be closed after writing.
 * @param[in] option Only SAVE_8_BITS_HIGH and SAVE_8_BITS_LOW are supported.
 *
 */
int imguSaveRAW8ToFile(imgu *I,FILE *F,unsigned char option)
{
    int i,sz;
    unsigned char *buf;
    if (option==SAVE_16_BITS) return -1;
    if (I==NULL || I->data==NULL) return -1;
    if (F==NULL) return -1;

    sz=I->xs*I->ys*I->cs;
    buf=(unsigned char *)malloc(sz);
    for(i=0;i<sz;i++) {
        if (option==SAVE_8_BITS_HIGH) buf[i]=(unsigned char)(I->data[i]>>8);
        else buf[i]=(unsigned char)(I->data[i]);
    }
    fwrite(buf,1,sz,F);
    free(buf);
    return(0);
}

/**
 * Save image data and text in PNG format to an opened file
 *
 * Suggestion: Use this function and imguLoadFromFile() to pipe image sequences between programs instead of saving to disk.
 *
 * @param[in] I Image
 * @param[in] F File descriptor (not closed after saving). Use 'stdout' to pipe image I to another program.
 * @param[in] compress Compression level from 0 (no compression) to 9 (best compression). Available constants are NO_COMPRESSION, FAST_COMPRESSION and BEST_COMPRESSION.
 * @param[in] option Supported options are SAVE_16_BITS, SAVE_8_BITS_HIGH (8 most significant bits) and SAVE_8_BITS_LOW (8 least significant bits).
 *
 * @return 0 : success; <0 : error
 * 
 */
int imguSaveToFile(imgu *I,FILE *F,int compress,unsigned char option)
{
#ifndef HAVE_LIBPNG
    fprintf(stderr,"SAVE: PNG LIBRARY NOT AVAILABLE!\n");
    return(-1);
#else
    png_structp png_ptr;
    png_infop info_ptr;
    int y;
    int i,index;
	int save16bpp;

    if (I==NULL || I->data==NULL) return -1;
    if (F==NULL) return -1;  

	save16bpp=(option==SAVE_16_BITS || option==SAVE_16_BITS_HIGH || option==SAVE_16_BITS_LOW
#ifndef IMGU8
		|| option==SAVE_AS_IS
#endif
#ifdef IMGU8
	|| I->cs==6 || I->cs==8
#endif
	 );

#ifdef IMGU8
	if( I->cs==6 || I->cs==8 ) { printf("SaveImage : saving cs=%d as 16bit %d channels\n",I->cs,I->cs/2); }
#endif


    //writepng_version_info();

    if( F==NULL ) return(-1);

    png_ptr = png_create_write_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
    if( png_ptr==NULL ) {
        //fclose(F);
        //unlink(name);
        return(-2);
    }

    info_ptr = png_create_info_struct(png_ptr);
    if( info_ptr==NULL || setjmp(png_ptr->jmpbuf)) {
        //fclose(F);
        //unlink(name);
        png_destroy_write_struct(&png_ptr, &info_ptr);
        return(-3);
    }

    png_init_io(png_ptr, F);
    png_set_filter(png_ptr, 0, PNG_ALL_FILTERS);
    if( compress<0 ) compress=0;
    if( compress>9 ) compress=9;

    png_set_compression_level(png_ptr,compress);

    /***
      png_set_IHDR(png_ptr, info_ptr,
      img->w, img->h,
      8, PNG_COLOR_TYPE_PALETTE | PNG_COLOR_MASK_PALETTE,
      PNG_INTERLACE_NONE, PNG_COMPRESSION_TYPE_DEFAULT,
      PNG_FILTER_TYPE_DEFAULT);
     ***/

    int ct;
    if( I->cs==1 ) ct=PNG_COLOR_TYPE_GRAY;
    else if( I->cs==2 ) ct=PNG_COLOR_TYPE_GRAY_ALPHA;
    else if( I->cs==3 ) ct=PNG_COLOR_TYPE_RGB;
    else if( I->cs==4 ) ct=PNG_COLOR_TYPE_RGB_ALPHA;
#ifdef IMGU8
    else if( I->cs==6 ) ct=PNG_COLOR_TYPE_RGB;
    else if( I->cs==8 ) ct=PNG_COLOR_TYPE_RGB_ALPHA;
#endif
    else {
        printf("illegal number of channels: %d (1..4 only, or 6,8 in 8bit)\n",I->cs);
        //fclose(F);
        //unlink(name);
        png_destroy_write_struct(&png_ptr, &info_ptr);
        return(-4);
    }

    if( save16bpp ) {
        png_set_IHDR(png_ptr, info_ptr,
                I->xs,I->ys,
                16,    // bitdepth
                ct,
                PNG_INTERLACE_NONE, PNG_COMPRESSION_TYPE_DEFAULT,
                PNG_FILTER_TYPE_DEFAULT);
    } else {
        png_set_IHDR(png_ptr, info_ptr,
                I->xs,I->ys,
                8,    // bitdepth
                ct,
                PNG_INTERLACE_NONE, PNG_COMPRESSION_TYPE_DEFAULT,
                PNG_FILTER_TYPE_DEFAULT);
    }

    //	printf("Saving as colortype %d (%s) cs=%d\n",ct,Ctype[ct],I->cs);


    ////// compte le vrai nombre de textes
    int nbT=0;
    for(i=0;i<I->ts;i++) if( I->key[i] && I->txt[i] ) nbT++;

    //	printf("%d textes\n",nbT);
    if( nbT>0 ) {
        int j;
        ////// output le texte
        png_text *text=(png_text *)malloc(nbT*sizeof(png_text));

        j=0;
        for(i=0;i<I->ts;i++) {
            if( !I->key[i] || !I->txt[i] ) continue;
			// anything below 100 char is not compressed
			if( strlen(I->txt[i])<100 ) text[j].compression = PNG_TEXT_COMPRESSION_NONE;
            else text[j].compression = PNG_TEXT_COMPRESSION_zTXt;
            text[j].key=I->key[i];
            text[j].text=I->txt[i];
            j++;
        }
        png_set_text(png_ptr, info_ptr, text, nbT);
        free(text);
    }

    png_write_info(png_ptr, info_ptr);


    int flipY=(InternalFormat!=ExternalFormat);
    int yy;

    unsigned short *onerow=NULL; // when fliping the short, allocate a row
    unsigned char *charrow=NULL;

    if( save16bpp )
    {
	if( I->cs==6 || I->cs==8 ) {
		// special 8bit image already contains 16bit data
		onerow=(unsigned short *)malloc(I->xs*I->cs/2*sizeof(unsigned short));
	}else{
		onerow=(unsigned short *)malloc(I->xs*I->cs*sizeof(unsigned short));
	}
    }else{
        charrow=(unsigned char *)(malloc(sizeof(unsigned char)*I->xs*I->cs));
    }

    // output all rows
    for (y = 0; y < I->ys; y++) {
        if( save16bpp )
        {
            if( flipY ) yy=(I->ys-1-y); else yy=y;
#ifdef IMGU8
	// we are promoting unsigned char to 16 bits
	// normally, we need an option SAVE_16_BITS_HI and LOW
        if( I->cs==6 || I->cs==8 ) {
            unsigned short *p=(unsigned short *)(I->data+(yy*I->xs*I->cs));
            if( is_big_endian() ) {
		for(i=0;i<I->xs*I->cs/2;i++) onerow[i] = (p[i]>>8)|(p[i]<<8);
                png_write_row(png_ptr, (png_bytep)onerow);
	    }else{
		png_write_row(png_ptr, (png_bytep)p);
	    }
	}else{
            unsigned char *p=I->data+(yy*I->xs*I->cs);
            if( is_big_endian()^(option==SAVE_16_BITS_LOW) ) { // are we flipping shorts?
                for(i=0;i<I->xs*I->cs;i++) onerow[i] = (p[i]); // HI, big-endian
            }else{
                for(i=0;i<I->xs*I->cs;i++) onerow[i] = (p[i]<<8); // HI, little-endian
	    }
	    png_write_row(png_ptr, (png_bytep)onerow);
	}
#else
            if( is_big_endian() ) { // are we flipping shorts?
                unsigned short *p=I->data+(yy*I->xs*I->cs);
                for(i=0;i<I->xs*I->cs;i++) onerow[i] = (p[i]>>8)|(p[i]<<8);
                png_write_row(png_ptr, (png_bytep)onerow);
            }else{
				// directly write the row since it is endian-ok
                png_write_row(png_ptr, (png_bytep)(I->data+yy*I->xs*I->cs));
            }
#endif
        }
        else // we are saving 8 bits
        {
            if( flipY ) yy=(I->ys-1-y); else yy=y;
            index=yy*I->xs*I->cs;
#ifdef IMGU8
		// uchar image to uchar byte...
		for(i=0;i<I->xs*I->cs;i++) charrow[i]=I->data[index+i];
#else
            if (option==SAVE_8_BITS_HIGH) {
                for(i=0;i<I->xs*I->cs;i++) charrow[i]=(unsigned char)(I->data[index+i]>>8);
            }else{
				// low byte only
                for(i=0;i<I->xs*I->cs;i++) charrow[i]=(unsigned char)(I->data[index+i]);
            }
#endif
            png_write_row(png_ptr, (png_bytep)(charrow));
        }
    }

    if( charrow ) free(charrow);
    if( onerow ) free(onerow);

    png_write_end(png_ptr, NULL);
    png_destroy_write_struct(&png_ptr, &info_ptr);

    return(0);
#endif
}


/**
 * Save multiple images (data and text) as concatenated PNG images in a single file
 *
 * Save a linked list of images (using the next field) to a single output file,
 * as concatenated PNG images. Otherwise, similar to imguSaveToFile().
 *
 * @param[in] I Image
 * @param[in] name Output file name. File will be closed at end of saving
 * @param[in] compress Compression level from 0 (no compression) to 9 (best compression). Available constants are NO_COMPRESSION, FAST_COMPRESSION and BEST_COMPRESSION.
 * @param[in] option Supported options are SAVE_16_BITS, SAVE_8_BITS_HIGH (8 most significant bits) and SAVE_8_BITS_LOW (8 least significant bits).
 *
 * @return 0: on success
 * @return -1: Unable to open file for writing
 * @return -2: Error while writing
 *
 */
int imguSaveMulti(imgu *I,const char *name,int compress,unsigned char option)
{
    FILE *F;
    int k;

    F=fopen(name,"wb");
    if( F==NULL ) return(-1);

    while( I ) {
        k=imguSaveToFile(I,F,compress,option);
        if( k ) {
            fclose(F);
            unlink(name);
            return(-2);
        }
        I=I->next;
    }

    fclose(F);

    return(0);
}


typedef enum img_type {UNKNOWN_TYPE,PNG_TYPE,JPG_TYPE,PNM_TYPE} IMGU_TYPE;


static enum img_type imguTypeByExtension(const char *name)
{
    char *c;

    if (name==NULL) return UNKNOWN_TYPE;

    c=strrchr(name,'.');
    if (c==NULL)
    {
        fprintf(stderr,"[imgu] Error parsing extension in filename '%s'.\n",name);return UNKNOWN_TYPE;
    }

    if (!strcasecmp(c,".jpg")) return JPG_TYPE;
    if (!strcasecmp(c,".jpeg")) return JPG_TYPE;
    if (!strcasecmp(c,".jif")) return JPG_TYPE;
    if (!strcasecmp(c,".jpe")) return JPG_TYPE;
    if (!strcasecmp(c,".jp2")) return JPG_TYPE;
    if (!strcasecmp(c,".j2c")) return JPG_TYPE;
    if (!strcasecmp(c,".j2k")) return JPG_TYPE;
    if (!strcasecmp(c,".jpc")) return JPG_TYPE;
    if (!strcasecmp(c,".jpx")) return JPG_TYPE;
    if (!strcasecmp(c,".ppm")) return PNM_TYPE;
    if (!strcasecmp(c,".pgm")) return PNM_TYPE;
    if (!strcasecmp(c,".pbm")) return PNM_TYPE;
    if (!strcasecmp(c,".png")) return PNG_TYPE;

    return UNKNOWN_TYPE;
}


/**
 * Counts the number of images in the image list @p I
 *
 * Suggestion: Use imguAddToMulti() to add an image to the end of a list.
 *
 * @param[in] I Image
 *
 * @return Number of images in the image list I
 *
 */
int imguCount(imgu *I)
{
    imgu *currI;
    int count=0;

    for(currI=I;currI!=NULL;currI=currI->next)
    {
        count++;
    }

    return count;
}

/**
 * Checks if image coordinates @p x and @p y are inside bounds of image @p I.
 *
 * Checks if x is within [0,I->xs) and y within [0,I->ys)
 *
 * @param[in] I Image
 * @param[in] x Pixel position x
 * @param[in] y Pixel position y
 *
 * @return 0 if inside
 * @return -1 if outside of if an error occured
 *
 */
int imguCheck(imgu *I,float x,float y)
{
    int fx,fy;//,cx,cy;
    int r;

    if (I==NULL) return -1;

    /*if( isnanf(x) || isnanf(y) ) return(0);*/

    fx=(int)floor(x);//cx=(int)ceil(x);
    fy=(int)floor(y);//cy=(int)ceil(y);

    if( fx<0 || fy<0 || fx>=I->xs || fy>=I->ys ) r=-1;
    else r=0;

    /*printf("Check (%f,%f) [%d,%d] -> %d\n",x,y,I->xs,I->ys,r);*/

    return(r);
}

/**
 * Save a single image. Try to infer the output format from the file extension.
 *
 * @attention If no external library is installed (PNG,JPEG,PNM), then only raw .pgm (gray) and .ppm (color) files are supported.
 * @attention If the PNG library is installed, then the .png extension is supported.
 * @attention If the JPEG library is installed, then the following extensions are supported: jpg,jpeg,jif,jpe,jp2,j2c,j2k,jpc,jpx.
 * @attention If the PNM library is installed, then the following extensions are supported: ppm,pgm,pbm.
 * @attention PNM images (ppm,pgm,pbm) are saved as raw and uncompressed images.
 * @warning Only the PNG format supports saving text information in images.
 * @warning If the file type is JPEG, then the option SAVE_16_BITS is not supported and save will fail.
 *
 * @param[in] I Image
 * @param[in] name File name
 * @param[in] compress Compression level from 0 (no compression) to 9 (best compression). Available constants are NO_COMPRESSION, FAST_COMPRESSION and BEST_COMPRESSION.
 * @param[in] option Supported options are SAVE_16_BITS, SAVE_8_BITS_HIGH (8 most significant bits) and SAVE_8_BITS_LOW (8 least significant bits).
 *
 * @return 0: sucess; -1: error (image format not supported for instance)
 */
int imguSave(imgu *I,const char *name,int compress,unsigned char option)
{
    enum img_type type;

    if (compress<0 || compress>9) return -1;
    if( !I ) return(-1);
    if( !name ) return(-1);

    type=imguTypeByExtension(name);
    if (type==JPG_TYPE)
    {
		int comp;
        if (compress<=0) comp=99;
        else if (compress==1) comp=95;
        else if (compress==2) comp=90;
        else if (compress==3) comp=85;
        else if (compress==4) comp=80;
        else if (compress==5) comp=75;
        else if (compress==6) comp=70;
        else if (compress==7) comp=65;
        else if (compress==8) comp=60;
        else if (compress==9) comp=55;
        else comp=50;
#ifdef HAVE_JPEG
        return imguSaveJPEG(I,name,comp,option);
#else
		printf("imguSave: No JPEG support.\n");
		return(-1);
#endif
    }
    if (type==PNM_TYPE) {
#ifdef HAVE_NETPBM
		return imguSavePNM(I,name,option);
#else
		printf("imguSave: No PNM support.\n");
		return(-1);
#endif
    }
    if (type==PNG_TYPE) return imguSavePNG(I,name,compress,option);

    //type of image is UNKNOWN
    return -1;
}

int imguSavePNG(imgu *I,const char *Name,int compress,unsigned char option)
{
    FILE *F;
    int k;

    if( !I ) return(-1);
    if( !Name ) return(-1);

    F=fopen(Name,"wb");
    if( F==NULL ) return(-1);

    k=imguSaveToFile(I,F,compress,option);
    fclose(F);
    if( k ) {
        unlink(Name);
        return(-1);
    }
    return(0);
}


/**
 * Load a PNG image from an opened file.
 * 
 * The Load operation fails, it is attempted @p nb_read_attempts times (up to IMGU_MAX_NB_READ_ATTEMPTS).
 * 
 * @see imguLoadFromFile()
 */
int imguLoadOrWaitFromFile(imgu **I,FILE *F,int nb_read_attempts,unsigned char option)
{
    int k;

    if (I==NULL) return -1;
    if (F==NULL) return -1;

    k=-1;     
    do
    {
        if (!imguLoadFromFile(I,F,option)) break;
#ifdef WIN32
        //Sleep(1000);
#else
        sleep(1);
#endif
        k++;
    }while(k<IMGU_MAX_NB_READ_ATTEMPTS && k<nb_read_attempts);
    if (k==IMGU_MAX_NB_READ_ATTEMPTS || k==nb_read_attempts)
    {
        //fprintf(stderr,"Maximum number of buffer reads attempted\n");
        return -1;
    }

    return 0;
}

/**
 * Load a single PNG image from an opened file.
 *
 * Load an image as 16bit PNG. If the image was encoded using 8 bits, and @p option is set to LOAD_16_BITS, then image data is rescaled for the 16-bit format.
 * We strongly suggest to use the LOAD_16_BITS option as IMGU is specifically written for 16-bit images.
 * The LOAD_AS_IS option can be used to load the data as is (without rescaling). This can be useful if the pixel values have a specific meaning (labels for instance).
 * Use LOAD_AS_KEEP16 to load a 16bit image as an 8bit 6 or 8 channels rgb or rgba.
 *
 * Memory is allocated or reused if possible.
 *
 * @warning The PNG library has to be installed to use this function.
 *
 * Suggestion: Use this function and imguSaveToFile() to pipe image sequences between programs instead of saving to disk.
 *
 * @param[in,out] I Image pointer
 * @param[in] F Open file descriptor. Use 'stdout' to read images from pipe.
 * @param[in] option Supported options are LOAD_16_BITS and LOAD_AS_IS.
 * @return 0: success; <0: error
 */
int imguLoadFromFile(imgu **I,FILE *F,unsigned char option)
{
#ifndef HAVE_LIBPNG
    fprintf(stderr,"LOAD: PNG LIBRARY NOT AVAILABLE!\n");
    return(-1);
#else
    png_struct *png_ptr;
    png_info *info_ptr;
    int bit_depth,color_type;
    png_uint_32 width,height;
    int channels;

    if (I==NULL) return -1;
    if (F==NULL) return -2;

    //readpng_version_info();

    if( read_sig_buf(F) ) return -3;   // signature

    png_ptr = png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
    if (png_ptr == NULL) {
        //printf("cannot allocate main libpng structure (png_ptr)\n");
        return -4;
    }

    info_ptr = png_create_info_struct (png_ptr);
    if (info_ptr == NULL) {
        png_destroy_read_struct(&png_ptr, NULL, NULL);
        //printf("cannot allocate LIBPNG structures");
        return -5;
    }

    if (setjmp(png_ptr->jmpbuf)) {
        png_destroy_read_struct(&png_ptr, &info_ptr, NULL);
        //printf("ERR\n");
        return -6;
    }


    png_init_io (png_ptr, F);
    png_set_sig_bytes (png_ptr, SIG_CHECK_SIZE);
    png_read_info (png_ptr, info_ptr);

    png_get_IHDR(png_ptr, info_ptr, &width, &height, &bit_depth, &color_type,
            NULL, NULL, NULL);
    channels = (int)png_get_channels(png_ptr, info_ptr);

    //printf("LOAD (option=%d): w=%d h=%d channels=%d depth=%d color_type=%d (%s)\n",option,width,height,channels,bit_depth,color_type,Ctype[color_type]);

    // 8 pixels de 1 bit en 1 seul byte -> 8 bytes
    // pas necessaire a cause du gray_1_2_4
    //png_set_packing(png_ptr);

    /// transformations
    if (color_type == PNG_COLOR_TYPE_PALETTE) {
        png_set_palette_to_rgb(png_ptr);
        channels=3;
    }
    if (color_type == PNG_COLOR_TYPE_GRAY && bit_depth < 8)
        png_set_gray_1_2_4_to_8(png_ptr);

    if (png_get_valid(png_ptr, info_ptr, PNG_INFO_tRNS)) {
        // ceci va transformer un gray en gray+alpha
        png_set_tRNS_to_alpha(png_ptr);
        // ajoute un channel si on avait pas de alpha avant...
        if( color_type==PNG_COLOR_TYPE_GRAY
                ||  color_type==PNG_COLOR_TYPE_RGB
                ||  color_type==PNG_COLOR_TYPE_PALETTE ) channels+=1;
    }

    // convert RGB -> GRAY or RGBA -> GRAY_A
    /*
       if (color_type == PNG_COLOR_TYPE_RGB ||
       color_type == PNG_COLOR_TYPE_RGB_ALPHA)
       png_set_rgb_to_gray_fixed(png_ptr, error_action,-1,-1);
       */

    /* swap bytes of 16 bit files to least significant byte first */
    // load
    // swapping makes the image bad. (tested on a big-endian system)
    // do not use
    if( is_big_endian() ) png_set_swap(png_ptr);

    // enleve le 16bit
    // if (bit_depth == 16) png_set_strip_16(png_ptr);
    //if (color_type == PNG_COLOR_TYPE_GRAY || color_type == PNG_COLOR_TYPE_GRAY_ALPHA)
    //	png_set_gray_to_rgb(png_ptr);

    // pas de alpha
    //png_set_strip_alpha(png_ptr);

    png_uint_32  i, rowbytes;
    png_bytep  *row_pointers;

    row_pointers=(png_bytep *)malloc(height*sizeof(png_bytep));

    png_read_update_info(png_ptr, info_ptr);

    unsigned char *image_data;

	// this is the number of bytes needed per row
    rowbytes = png_get_rowbytes(png_ptr, info_ptr);

    // facteur multiplicateur de l'espace bitmap
#ifdef IMGU8
    // (depth=8->1, depth=16->1) Dans le cas de depth=16, on est 1 grace au allocExtra.
    int mult=1;
#else
    //  (depth=8->2, depth=16->1)
    int mult=(bit_depth==16?1:2);
#endif

#ifdef IMGU8
	if( bit_depth==16 ) {
		// we need more space to load the 16bit image into a 8 bit image...
    		imguAllocateExtra(I,width,height,channels,width*height*channels);
	}else{
			// 8 bits dans un 8 bits
    		imguAllocate(I,width,height,channels);
	}
#else
	// 16 bits-> 16 bit, normal allocate
    imguAllocate(I,width,height,channels);
#endif

    imguResetText((*I));
    image_data=(unsigned char *)((*I)->data);

    if( image_data == NULL) {
        png_destroy_read_struct(&png_ptr, &info_ptr, NULL);
        free(row_pointers);
        return -7;
    }

    int flipY=(InternalFormat!=ExternalFormat);

	// On utilise l'image imgu comme buffer de lecture... deux cas:
    // si depth=8, et imgu 16bits, les lignes sont 2x trop grande
    //                (On pourra copier surplace plus tard)
    // si depth=16 et imgu 8bits, les lignes sont 2x trop courtes (on doit utiliser extra)
    for (i=0;  i<height; i++) {
        if( flipY )	row_pointers[i] = image_data + (height-1-i)*rowbytes*mult;
        else row_pointers[i] = image_data + i*rowbytes*mult;
    }

    png_read_image(png_ptr, row_pointers);
    png_read_end(png_ptr, NULL);

#ifdef IMGU8
	if( bit_depth==16 ) {
	    if( option==LOAD_AS_HIGH_LOW ) {
		// retourne une image High et une image Low
		imgu* ILow=NULL;
    		imguAllocate(&ILow,width,height,channels);
		unsigned char *p;
		unsigned char *qH,*qL;
		int j;
		int shiftH=is_big_endian()?1:0;
		int shiftL=is_big_endian()?0:1;
		for (i=0;i<height; i++) {
			p=row_pointers[i];
			qH=image_data+i*(*I)->xs*(*I)->cs;
			qL=ILow->data+i*(*I)->xs*(*I)->cs;
			for(j=width*channels-1;j>=0;j--) { *qH++ = p[shiftH]; *qL++=p[shiftL]; p+=2; }
		}
		(*I)->next=ILow;
	    }else if( option==LOAD_AS_KEEP16 ) {
		// we know the image is already with the correct buffer. simply adjust cs
		(*I)->cs*=2;
	    }else{
		// image is 16 bits... we are 8 bits
		unsigned char *p;
		unsigned char *q;
		int j;
		int shift = ( is_big_endian()^(option==LOAD_8_BITS_LOW || option==LOAD_AS_IS || option==LOAD_16_BITS) );
		/// pourquoi ca ne sert a rien? je ne sait pas...
		for (i=0;i<height; i++) {
			p=row_pointers[i];
			if( shift ) p++;
			q=image_data+i*(*I)->xs*(*I)->cs;
			for(j=width*channels-1;j>=0;j--) { *q++ = *p;p+=2; }
		}
	    }
	}
#else
    if(bit_depth<16 ) {
		// image is 8 bits... we are 16bits
        unsigned char *p;
        unsigned short *q;
        int j;
        if( option==LOAD_16_BITS ) {
        	// copy inplace, 8 bits to 16 bits. Shift values.
            for (i=0; i<height; i++) {
                j=width*channels-1;
                p=row_pointers[i]+j;
                q=(unsigned short *)row_pointers[i]+j;
                // multiply every value by 257 (x<<8)+x
                for(;j>=0;j--,p--) *q-- = ((unsigned short)*p<<8)+*p;
            }
        }else{
			// copy inplace, 8 bits to 16 bits, as is.
            for (i=0; i<height; i++) {
                j=width*channels-1;
                p=row_pointers[i]+j;
                q=(unsigned short *)row_pointers[i]+j;
                for(;j>=0;j--) *q-- = (unsigned short)*p--;
            }
        }
    }
#endif

    /// lit le texte...
    png_textp text_ptr;
    int nbt;
    int j;
    png_get_text(png_ptr,info_ptr,&text_ptr,&nbt);
    for(j=0;j<nbt;j++) {
        imguAddText((*I),text_ptr[j].key,text_ptr[j].text);
    }
    if( bit_depth==16 && option==LOAD_AS_HIGH_LOW ) {
	    for(j=0;j<nbt;j++) {
		imguAddText((*I)->next,text_ptr[j].key,text_ptr[j].text);
	    }
    }

    png_destroy_read_struct(&png_ptr, &info_ptr, NULL);
    free(row_pointers);

    return 0;
#endif
}

/**
 * Load a single PNG image from a filename. The format is guessed from filename extension.
 *
 * @attention If no external library is installed (PNG,JPEG,PNM), then only raw .pgm (gray) and .ppm (color) files are supported.
 * @attention If the PNG library is installed, then the .png extension is supported.
 * @attention If the JPEG library is installed, then the following extensions are supported: jpg,jpeg,jif,jpe,jp2,j2c,j2k,jpc,jpx.
 * @attention If the PNM library is installed, then the following extensions are supported: ppm,pgm,pbm.
 *
 * Load an image as 16bit PNG. If the image was encoded using 8 bits, and @p option is set to LOAD_16_BITS, then image data is rescaled for the 16-bit format.
 * We strongly suggest to use the LOAD_16_BITS option as IMGU is specifically written for 16-bit images.
 * The LOAD_AS_IS option can be used to load the data as is (without rescaling). This can be useful if the pixel values have a specific meaning (labels for instance).
 *
 * Memory for @p I is allocated or reused if possible.
 *
 * @attention If it can not guess the file format, it assumes PNG.
 *
 * @param[in,out] I Image pointer
 * @param[in] name Filename
 * @param[in] option Supported options are LOAD_16_BITS and LOAD_AS_IS.
 * @return 0: success; <0: error
 *
 * \example
 * \code
 * imgu *IA=NULL;
 * if( imguLoad(&IA,"input.png",LOAD_AS_IS) ) { printf("Unable to load\n"); }
 * \endcode
 *
 */
int imguLoad(imgu **I,const char *name,unsigned char option)
{
    FILE *F;
    int ret;
    enum img_type type;

    if (I==NULL) return -1;
    if (name==NULL) return -1;

    type=imguTypeByExtension(name);
    //fprintf(stderr,"%d\n",type);
    if (type==JPG_TYPE) {
#ifdef HAVE_JPEG
		return imguLoadJPEG(I,name,option);
#else
		printf("imguLoad: no JPEG support\n");
		return(-1);
#endif
	}
    if (type==PNM_TYPE) {
#ifdef HAVE_NETPBM
		return imguLoadPNM(I,name,option);
#else
		printf("imguLoad: No PNM support.\n");
		return(-1);
#endif
	}

    F=fopen(name,"rb");
    if( F==NULL )  return -1;

    ret=imguLoadFromFile(I,F,option);

    fclose(F);

    if (ret==0) return 0; //image loaded successfully (it's a PNG file)

    //PNG or by extension failed, try to load by file header
    //THIS DOESN'T WORK: each of the following functions EXITS when not the right image type
    /*if (imguLoadPNM(I,name,option))
      {
    //error loading PNM
    if (imguLoadJPEG(I,name,option))
    { //error loading JPEG
    return -1;
    }
    }*/

    return -1;
}

int imguLoadPNG(imgu **I,const char *name,unsigned char option)
{
    FILE *F;

    if (I==NULL) return -1;
    if (name==NULL) return -1;

    F=fopen(name,"rb");
    if( F==NULL )  return -1;

    imguLoadFromFile(I,F,option);

    fclose(F);
    return 0;
}


/**
 * Load multiple PNG images from a single file
 *
 * Load a series of PNG images, assumed to be concatenated in a single file.
 * The images are linked using @c next.
 *
 * @attention If @p I was already allocated and part of a list of images, then subsequent images in the list are reused (i.e. changed) and new images are created
 * if required. If the number of images pointed by @p I is larger than the number of images loaded, then remaining images in the list are freed.
 *
 * @param[out] I Image pointer.
 * @param[in] name File name.
 * @param[in] option Supported options are LOAD_16_BITS and LOAD_AS_IS. @see imguLoad().
 * @return 0: success; <0: error
 *
 */
int imguLoadMulti(imgu **I,const char *name,unsigned char option)
{
    int n; // nombre d'images
    imgu **currI;
    FILE *F;

    if (I==NULL || name==NULL) return -1;

    // On fait une liste d'images, c'est plus simple...
    currI=I;

    n=0;

    F=fopen(name,"rb");
    if( F==NULL )  return -1;

    for(;;) {
        //printf("At position %d\n",ftell(F));
        if (imguLoadFromFile(currI,F,option)) break;
        //printf("got I=0x%08lx\n",I);
        if( (*currI)==NULL ) break;
        n++;

        currI=&((*currI)->next);
    }

    //printf("loaded %d images\n",n);

    fclose(F);

    imguFreeMulti(currI); //free rest of images, in case LoadMulti was already called on this pointer with a longer list of images

    return 0;
}



////////////////////////////////////////////////////////
////////////////////////////////////////////////////////
////////////////////////////////////////////////////////

/**
 * Allocates memory to store text information.
 *
 * @attention This function keeps any text information that @p I already has. To reset text information, use imguResetText().
 *
 * @attention This function should only be used to allocate an empty image with text information. Otherwise, this function should not be used since
 * memory for text information is always allocated using imguAllocate() or imguLoad().
 *
 * @param[in,out] I Address of an image pointer.
 * @return 0: success; <0: error
 *
 */
static int imguAllocateText(imgu **I)
{
    int i;
    char **key,**txt;
    int *txtsz;

    if (I==NULL) return -1;

    if ((*I)==NULL)
    {
		printf("AllocateText on an empty image is not permitted anymore. Allocate first.\n");
        return(-1);
		/***
        (*I)=(imgu *)(malloc(sizeof(imgu)));
        if ((*I)==NULL) return -1;
		***/
    }
    else
    {
        if( (*I)->key && (*I)->txt && (*I)->txtsz )  //reuse text space
        {
            //imguResetText((*I));
            return 0;
        }
    }

	if( (*I)->ts == 0 ) {
		key=(char **)malloc(INIT_NB_TEXT*sizeof(char *));
		if( !key ) { imguFree(I);return -1; }
		txt=(char **)malloc(INIT_NB_TEXT*sizeof(char *));
		if( !txt ) { free(key);imguFree(I);return -1; }
		txtsz=(int *)malloc(INIT_NB_TEXT*sizeof(int));
		if( !txtsz ) { free(key);free(txt);imguFree(I);return -1; }

		(*I)->ts=INIT_NB_TEXT;
		(*I)->key=key;
		(*I)->txt=txt;
		(*I)->txtsz=txtsz;
    	for(i=0;i<(*I)->ts;i++) { (*I)->key[i]=(*I)->txt[i]=NULL; (*I)->txtsz[i]=0; }
	}else{
		// verify that we have valid pointers...
		if( (*I)->key==NULL || (*I)->txt==NULL || (*I)->txtsz==NULL ) {
			printf("WARNING!!! key,txt,txtsz are NULL but ts is not 0!!!!!!!!!!\n");
			exit(0);
		}
	}

    return 0;
}



/**
 * Print some information about a single image
 *
 * @param[in] I Image
 */
void imguDump(imgu *I)
{
    int i;
    if (I==NULL) return;
    printf("------ Image %d x %d x %d (%d text space) -----",I->xs,I->ys,I->cs,I->ts);
    if( I->data==NULL ) printf("(no data)");
    if( I->complex==NULL ) printf("(no complex)");
    printf("\n");

    for(i=0;i<I->ts;i++) {
        if( I->key[i]==NULL && I->txt[i]==NULL ) continue;
        if( I->key[i]==NULL || I->txt[i]==NULL )  {
            printf("TXT[%d]: Single NULL : illegal.\n",i);
            continue;
        }
        printf("Txt[%s]: %s\n",I->key[i],I->txt[i]);
    }

#ifdef LONG_DEBUG
    int x,y,c;
    for(y=0;y<I->ys;y++) {
        printf("y=%3d: ",y);
        for(x=0;x<I->xs;x++) {
            printf("[");
            for(c=0;c<I->cs;c++) {
                printf("%d",I->data[(y*I->xs+x)*I->cs+c]);
                if( c<I->cs-1 ) printf(",");
            }
            printf("] ");
        }
        printf("\n");
    }
#endif

    printf("---------------------------\n");
}


/**
 * Print some information about a linked list of images
 *
 * @param[in] I Image
 */
void imguDumpMulti(imgu *I)
{
    while( I ) {
        imguDump(I);
        I=I->next;
    }
    printf("========\n");
}

/**
 * Allocates text and data memory, with extra shorts at the end of data space.
 *
 * Image data is allocate and NOT initialised to 0. 
 *
 * @attention This function keeps any text information that @p I already has. To reset text information, use imguResetText().
 * @warning Any complex information should be assumed lost, except if @p xs,ys,cs correspond exactly to the complex dimensions.
 *
 * @param[in,out] I Address of an image pointer.
 * @param[in] xs Width of image
 * @param[in] ys Height of image
 * @param[in] cs Number of color channels
 * @param[in] extra Number of extra short to add to the buffer (not bytes, shorts!)
 * @return 0: success; <0: error
 *
 */
int imguAllocateExtra(imgu **I,int xs,int ys,int cs,int extra)
{
    int size=xs*ys*cs+extra;
    if (imguAllocate(I,size,1,1)) return -1;
    return imguAllocate(I,xs,ys,cs);
}


/**
 * Allocates extra memory after an image, and preserve the image in all cases.
 *
 * @param[in,out] I Address of an image pointer.
 * @param[in] extra Number of extra short to add to the buffer (not bytes, shorts!)
 * @return 0: success; <0: error
 *
 */
int imguExtendExtra(imgu *I,int extra)
{
    int sz;
    pix_t *data;

    if (I==NULL) return -1;
    if( I->data==NULL ) return(-1);

    sz=I->xs*I->ys*I->cs+extra;

    if( sz <= I->ds ) {
			//printf("imguExtendExtra nothing to do\n");
			return 0; // nothing to do
		}
    pix_t *odata=I->data;

    data=(pix_t *)malloc(sz*sizeof(pix_t));
    if( data==NULL ) { return -1; }

    I->data=data;
    I->ds=sz;

    memcpy(I->data,odata,I->xs*I->ys*I->cs*sizeof(pix_t));

    free(odata);

    return 0;
}





/**
 * Allocates text and data memory.
 *
 * Image data is allocated and NOT initialized to 0. 
 *
 * @attention This function keeps any text information that @p I already has. To reset text information, use imguResetText().
 * @warning Any complex information should be assumed lost, except if @p xs,ys,cs correspond exactly to the complex dimensions.
 * @attention The image is not cleared. (@see imguClear)
 *
 * @param[in,out] I Address of an image pointer.
 * @param[in] xs Width of image
 * @param[in] ys Height of image
 * @param[in] cs Number of color channels
 * @return 0: success; <0: error
 *
 */
int imguAllocate(imgu **I,int xs,int ys,int cs)
{
    int sz;
    pix_t *data;

    if( I==NULL ) return -1;

    if( (*I)==NULL )
    {
		// reserve new space for the structure itself.
        (*I)=(imgu *)(malloc(sizeof(imgu)));
        if ((*I)==NULL) { printf("OutOfMem in imguAllocate\n");return -1; }

		// initialize some fields
    	(*I)->xs=(*I)->ys=(*I)->cs=0;
		(*I)->ds=0;
		(*I)->ts=0;
		(*I)->data=NULL;
		(*I)->complex=NULL;
		(*I)->key=NULL;
		(*I)->txt=NULL;
		(*I)->txtsz=NULL;
		(*I)->next=NULL;
#ifdef HAVE_RQUEUE
		(*I)->Qrecycle=NULL;
#endif
    }

    if( imguAllocateText(I) ) return -1;

    sz=xs*ys*cs;

    if( sz <= (*I)->ds && (*I)->data!=NULL) {
        (*I)->xs=xs;
        (*I)->ys=ys;
        (*I)->cs=cs;        
        return 0;
    }
    if (sz > (*I)->ds) imguFreeComplex(*I);
    else if ((*I)->complex!=NULL) sz=(*I)->ds; //allocate possibly more memory to be consistent with complex memory (although complex info is probably useless because of dimension change)

    imguFreeData(*I);

    if( sz==0 ) { printf("********* imguAllocate size 0 ***********\n");exit(0); }

    data=(pix_t *)malloc(sz*sizeof(pix_t));
    if( data==NULL ) { imguFree(I);return -1; }

    (*I)->xs=xs;
    (*I)->ys=ys;
    (*I)->cs=cs;
    (*I)->data=data;
    (*I)->ds=sz;
	(*I)->compressed_data_size = 0;

    return 0;
}

/**
 * Sets image data to 0.
 *
 */
int imguClear(imgu *I)
{
    int sz,i;

    if (I==NULL || I->data==NULL) return -1;

    sz=I->xs*I->ys*I->cs;

    for (i=0;i<sz;i++)
    {
        I->data[i]=0;
    }

    return 0;
}  

/**
 * Allocates text and data memory.
 *
 * Image data is allocate and NOT initialised to 0. 
 *
 * @attention This function keeps any text information that @p I already has. To reset text information, use imguResetText().
 * @warning Any complex information should be assumed lost, except if @p xs,ys,cs correspond exactly to the complex dimensions.
 *
 * @param[in] xs Width of image
 * @param[in] ys Height of image
 * @param[in] cs Number of color channels
 * @return Pointer to a new imgu image, NULL if there was a problem
 *
 */
imgu *imguCreate(int xs,int ys,int cs)
{
    imgu *I=NULL;
    int k;
    k=imguAllocate(&I,xs,ys,cs);
    if( k ) { imguFree(&I); return(NULL); }
    return(I);
}




// make sure there is enough space to add n texts
//static
int imguAllocateMoreText(imgu *I,int n)
{
    int newTs;
    char **newKey;
    char **newTxt;
    int *newTxtsz;
    int i;

    if (I==NULL) return -1;

    // compte le nb de textes disponibles si >=n, OK!
    for(i=0;n>0 && i<I->ts;i++) { if( I->key[i]==NULL ) n--; }

    if( n<=0 ) return(0); // assez de texte!

    // we need 'n' more textes

    //printf("Allocating %d more\n",n);

    newTs=I->ts+n+INIT_TEXT_STEP;
    newKey=(char **)malloc(newTs*sizeof(char *));
    newTxt=(char **)malloc(newTs*sizeof(char *));
    newTxtsz=(int *)malloc(newTs*sizeof(int));

    if( newKey==NULL || newTxt==NULL || newTxtsz==NULL ) {
        if( newKey ) free(newKey);
        if( newTxt ) free(newTxt);
        if( newTxtsz ) free(newTxtsz);
        return(-1);
    }

    // copy!!
    for(i=0;i<I->ts;i++) {
        newKey[i]=I->key[i];
        newTxt[i]=I->txt[i];
        newTxtsz[i]=I->txtsz[i];
    }
    for(;i<newTs;i++) { newKey[i]=newTxt[i]=NULL;newTxtsz[i]=0; }

    // move data!
    free(I->key);
    free(I->txt);
    free(I->txtsz);

    I->key=newKey;
    I->txt=newTxt;
    I->txtsz=newTxtsz;

    I->ts=newTs;

    return(0);

}

/**
 * Copy text information.
 *
 * @param[out] dest Address of destination image.
 * @param[in] src Pointer to source image.
 * @return 0: success; <0: error
 *
 */
int imguCopyText(imgu *dest,imgu *src)
{
    int i;

    if (dest==NULL) return -1;
    if (src==NULL) return -1;

    if( dest==src ) return 0;

    //if( imguAllocateText(dest) ) return -1;

    imguResetText(dest);
    imguAllocateMoreText(dest,src->ts);

    for(i=0;i<src->ts;i++)
    {
        imguSetText(dest,src->key[i],src->txt[i],i);
    }

    return 0;
}

/**
 * Copy text, data and complex information.
 *
 * @param[out] dest Address of destination image.
 * @param[in] src Pointer to source image.
 * @return 0: success; <0: error
 *
 */
int imguCopy(imgu **dest,imgu *src)
{
    int size;
    int i;

    if (dest==NULL) return -1;
    if (src==NULL) return -1;

    if( *dest==src ) return 0;

    size=src->xs*src->ys*src->cs;
    if (src->data!=NULL)
    {
        imguAllocate(dest,src->xs,src->ys,src->cs);
        memcpy((*dest)->data,src->data,size*sizeof(pix_t));
    }
    if (src->complex!=NULL)
    {
        imguAllocateComplex(dest,src->xs,src->ys,src->cs);
        for (i=0;i<size;i++)
        {
            (*dest)->complex[i][0]=src->complex[i][0];
            (*dest)->complex[i][1]=src->complex[i][1];
        }
    }
    imguCopyText(*dest,src);

    (*dest)->next=NULL;

    return 0;
}

/**
 * Copy text, data and complex information of a list of images.
 *
 * @attention If @p dest is already part of a list of images, then subsequent images in the list are reused (i.e. changed) and new images are created
 * if required. If the number of images pointed by @p dest is larger than the number of images pointed by @p src, then remaining images in the @p dest list are freed.
 *
 * @param[out] dest Address of destination image.
 * @param[in] src Pointer to source image.
 * @return 0: success; <0: error
 *
 */
int imguCopyMulti(imgu **dest,imgu *src)
{
    imgu **currdest,*currsrc;
    imgu *dest_next;

    if (dest==NULL) return -1;
    if (src==NULL) return -1;

    if( *dest==src ) return 0;

    currdest=dest;
    currsrc=src;
    while( currsrc!=NULL ) {
        dest_next=NULL;
        if ((*currdest)!=NULL) dest_next=(*currdest)->next;
        imguCopy(currdest,currsrc);
        (*currdest)->next=dest_next;
        currsrc=currsrc->next;
        currdest=&((*currdest)->next);
    }

    imguFreeMulti(currdest); //in case dest already contained a longer list

    return 0;
}


// place le key/txt a l'index i
// free le txt ou cle si necessaire
// si key est NULL, preserve la cle
// si txt est NULL, preserve le txt
//static
int imguSetText(imgu *I,const char *key,const char *txt,int i)
{
    if (I==NULL) return -1;

    //printf("Text set [%d] as key=>%s< txt=>%s<\n",i,key,txt);
    if( i<0 || i>=I->ts ) return(-1);
    if( key==NULL && I->key[i]==NULL ) return(-1);
    if( txt==NULL && I->txt[i]==NULL ) return(-1);

    // reset key and txt
    if( key!=NULL && I->key[i]!=NULL ) { free(I->key[i]);I->key[i]=NULL; }
    if( txt!=NULL && I->txt[i]!=NULL ) { free(I->txt[i]);I->txt[i]=NULL;I->txtsz[i]=0; }

    // si la cle est nulle, on remplace!
    if( I->key[i]==NULL ) {
        I->key[i]=(char *)malloc((strlen(key)+1)*sizeof(char));
        if( I->key[i] ) strcpy(I->key[i],key);
    }

    // si le texte est null, on remplace!
    if( I->txt[i]==NULL ) {
        I->txtsz[i]=strlen(txt)+1;
        I->txt[i]=(char *)malloc(I->txtsz[i]*sizeof(char)); // taille exacte
        if( I->txt[i] ) strcpy(I->txt[i],txt);
    }

    // OOM?
    if( I->key[i]==NULL || I->txt[i]==NULL ) {
        if( I->key[i] ) { free(I->key[i]);I->key[i]=NULL; }
        if( I->txt[i] ) { free(I->txt[i]);I->txt[i]=NULL;I->txtsz[i]=0; }
        return(-1); // OOM!
    }

    //printf("[%d] : key='%s' txt='%s' txtsz=%d\n",i,I->key[i],I->txt[i],I->txtsz[i]);
    return(0);

}

// retourne l'index de la permiere cle trouvee a partir de la position start
// ou -1 si rien trouve
static int imguFindKey(imgu *I,const char *key,int start)
{
    int i;
    if (I==NULL) return -1;

    for(i=start;i<I->ts;i++) {
        if( I->key[i]==NULL ) continue;
        if( strcmp(I->key[i],key)==0 ) return(i);
    }
    return(-1);
}

/**
 * Add a copy of the (key,txt) pair.
 *
 * The text @p txt will be added even if this key is already defined.
 *
 * @param I imgu image
 * @param key Key string. Must be non null and a regular keyword. See @ref reservedkeys
 * @param txt Text string. Must ne non null.
 * @return 0 on success; -1,-2,-3 on failure
 *
 */
int imguAddText(imgu *I,const char *key,const char *txt)
{
    int i;
    if (I==NULL) return -1;

    //printf("adding key='%s' txt='%s'\n",key,txt);
    if( imguAllocateMoreText(I,1) ) return(-1); // unable to add more text

    // trouve un txt vide (on est sur d'en trouver un)
    for(i=0;i<I->ts;i++) { if( I->key[i]==NULL ) break; }
    if( i==I->ts ) return(-2); // impossible

    if( imguSetText(I,key,txt,i) ) return(-3);

    return(0);
}


/**
 * Append text to a key
 *
 * @param I
 * 	imgu Image
 * @param key
 * 	key string. Must be non null and a regular keyword. See @ref reservedkeys
 * @param txt
 * 	text string. Must ne non null.
 *
 * Add the text @p txt at the end of the @p key value,
 * or create a new text if @p key does not exists.
 *
 */
int imguAppendText(imgu *I,const char *key,const char *txt)
{
    int i,LT,LnT,nL;
    char *ntxt;
    if (I==NULL) return -1;

    i=imguFindKey(I,key,0);
    if( i<0 || I->txt[i]==NULL ) return( imguAddText(I,key,txt) );

    LT=strlen(I->txt[i]); // texte actuel
    LnT=strlen(txt); // texte a ajouter

    // est-ce que la longueur actuelle est suffisante?
    if( I->txtsz[i]-LT > LnT ) {
        // on a assez de place!
        strcpy(I->txt[i]+LT,txt);
        return(0);
    }

    // non! On doit allouer +, disons 2x plus...
    nL=(LT+LnT+1)*2;
    ntxt=(char *)malloc(nL);
    //printf("malloc %9d bytes\n",nL);

    strcpy(ntxt,I->txt[i]);
    strcpy(ntxt+LT,txt);

    free(I->txt[i]);
    I->txt[i]=ntxt;
    I->txtsz[i]=nL;
    return(0);
}

/** Push text
 *
 * ajoute au debut du texte courant, sinon creer un nouveau texte
 */
int imguPushText(imgu *I,const char *key,const char *txt)
{
    int i,LT,LnT,nL;
    char *ntxt;
    if (I==NULL) return -1;

    i=imguFindKey(I,key,0);
    if( i<0 || I->txt[i]==NULL ) return( imguAddText(I,key,txt) );

    LT=strlen(I->txt[i]); // texte actuel
    LnT=strlen(txt); // texte a ajouter

    // est-ce que la longueur actuelle est suffisante?
    if( I->txtsz[i]-LT > LnT ) {
        // on a assez de place!
        nL=(LT+LnT+1);
        ntxt=(char *)malloc(nL);
        //printf("malloc %9d bytes\n",nL);

        strcpy(ntxt,txt);
        strcpy(ntxt+LnT,I->txt[i]);

        free(I->txt[i]);
        I->txt[i]=ntxt;
        I->txtsz[i]=nL;

        return(0);
    }

    // non! On doit allouer +, disons 2x plus...
    nL=(LT+LnT+1)*2;
    ntxt=(char *)malloc(nL);
    //printf("malloc %9d bytes\n",nL);

    strcpy(ntxt,txt);
    strcpy(ntxt+LnT,I->txt[i]);

    free(I->txt[i]);
    I->txt[i]=ntxt;
    I->txtsz[i]=nL;

    return(0);
}


// remplace le texte de la cle key par txt
// Si la cle n'est pas trouvee, ajoute la cle
// Key et Txt sont obligatoires et non NULL
// remplace la permiere cle trouvee (si cle multiple)
int imguReplaceAddText(imgu *I,const char *key,const char *txt)
{
    int i;
    if (I==NULL) return -1;

    i=imguFindKey(I,key,0);
    if( i<0 ) return( imguAddText(I,key,txt) );

    if( imguSetText(I,NULL,txt,i) ) return(-1);

    return(0);
}

// remplace le texte de la cle key par txt
// Si la cle n'est pas trouvee, ajoute la cle
// Key et Txt sont obligatoires et non NULL
// remplace la permiere cle trouvee (si cle multiple)
int imguReplaceText(imgu *I,const char *key,const char *txt)
{
    int i;
    if (I==NULL) return -1;

    i=imguFindKey(I,key,0);
    if( i<0 ) return(-1);

    if( imguSetText(I,NULL,txt,i) ) return(-1);
    return(0);
}


// elimine le texte associé à une clé (all=1 : tout, sinon le premier seulement)
// retourne le nb de cle eliminees
int imguRemoveKey(imgu *I,const char *key,int all)
{
    int i,n;
    n=0;
    i=0;
    do {
        i=imguFindKey(I,key,i);
        if( i<0 ) return(n);
        // kill key
        if( I->txt[i]!=NULL ) { free(I->txt[i]);I->txt[i]=NULL;I->txtsz[i]=0; }
        if( I->key[i]!=NULL ) { free(I->key[i]);I->key[i]=NULL; }
        n++;
        i++;
    } while( all );
    return(n);
}


/**
 * return the text associated with a specific key
 * return a pointer to the text. PLEASE DO NOT free() THE TEXT!!!
 *
 * return NULL if key does not exist
*/
char *imguGetText(imgu *I,const char *key)
{
    int i;
    if (I==NULL) return NULL;

    i=imguFindKey(I,key,0);
    if( i<0 ) return(NULL);
    return(I->txt[i]);
}


/**
 * Get all key-text pairs associated to a specific key
 *
 * @return number of key-text pairs for the key
 *
 * Fill the table TxtTab up to max values. TxtTab[] must exist for [0..max-1]
 */
int imguGetAllText(imgu *I,const char *key,char **TxtTab,int max)
{
    int i,j;
    if (I==NULL) return -1;

    i=0;
    j=0;
    for(j=0;j<max;j++) {
        i=imguFindKey(I,key,i);
        if( i<0 ) break;
        TxtTab[j]=I->txt[i];
        i++;
    }
    return(j);
}


/**
 * Free image text, data and complex information without freeing the image structure.
 *
 * Reset all data and text associated with an image. Also reset the link of the image. Does not free the @c imgu structure itself.
 *
 * @param I Image
 *
 */
void imguReset(imgu *I)
{
    if (I==NULL) return;

    if( I->data ) { free(I->data);I->data=NULL; }
    if( I->complex ) { free(I->complex);I->complex=NULL; }

    imguResetText(I);
    /* free the text buffer itself */
    if( I->key ) { free(I->key);I->key=NULL; }
    if( I->txt ) { free(I->txt);I->txt=NULL; }
    if( I->txtsz ) { free(I->txtsz);I->txtsz=NULL; }
    I->xs=I->ys=I->cs=I->ds=0;
    I->ts=0;
    I->next=NULL;
}


/**
 * Reset all text information associated with an image.
 *
 * @param[in] I Image
 */
void imguResetText(imgu *I)
{
    int i;
    if (I==NULL) return;

    for(i=0;i<I->ts;i++) {
        if( I->key[i] ) { free(I->key[i]);I->key[i]=NULL; }
        if( I->txt[i] ) { free(I->txt[i]);I->txt[i]=NULL;I->txtsz[i]=0; }
    }
}

/**
 * Adds image @p I to the end of image list @p head
 *
 * @param[out] head Adress of image list
 * @param[in] I Image
 */
int imguAddLastMulti(imgu **head,imgu *I)
{
    imgu *currimg;

    if (head==NULL) return -1;
    if (I==NULL) return -1;

    if( *head==I ) return 0;

    if ((*head)==NULL) 
    {
        (*head)=I;
        return 0;
    }

    currimg=(*head);
    while(currimg->next!=NULL)
    {
        currimg=currimg->next;
    }

    currimg->next=I;

    return 0;
}

/**
 * Adds image @p I to the start of image list @p head
 *
 * @param[out] head Adress of image list
 * @param[in] I Image
 */
int imguAddFirstMulti(imgu **head,imgu *I)
{
    if (head==NULL) return -1;
    if (I==NULL) return -1;

    I->next=(*head);
    (*head)=I;

    return 0;
}

/**
 * Extract image @p I from the start of the image list @p head
 *
 * @param[out] head Adress of image list
 * @param[in] I Image
 */
imgu *imguRemoveFirstMulti(imgu **head)
{
    imgu *I;

    if (head==NULL || (*head)==NULL) return NULL;
    I=*head;
    *head=(*head)->next;
    I->next=NULL;
    return(I);
}

/**
 * Extract image @p I from the end of the image list @p head
 *
 * @param[out] head Adress of image list
 * @param[in] I Image
 */
imgu *imguRemoveLastMulti(imgu **head)
{
    imgu *I,*Iprev;

    if (head==NULL || (*head)==NULL) return NULL;
    I=(*head)->next;
    Iprev=(*head);
    if (I==NULL) //list has only one element
    {
      *head=NULL;
      return Iprev;
    }

    while(I->next!=NULL) //find last element
    {
      Iprev=I;
      I=I->next;
    }
    Iprev->next=NULL;
    return(I);
}

/**
 * Free an image completely
 *
 * Free all text, data and complex buffers associated with an image.
 * Then free the @c imgu structure itself and sets @p I to NULL.
 *
 * @param I Image pointer (will be changed)
 * @return nothing. But @c *I is set to NULL.
 */
void imguFree(imgu **I)
{
    if (I==NULL || (*I)==NULL) return; 
    imguReset((*I));
    free((*I));
    (*I)=NULL;
}

/**
 * Free completely a list of images
 *
 * see imguFree()
 */
void imguFreeMulti(imgu **I)
{
    if (I==NULL) return;

    imgu *J;

    while( *I!=NULL ) {
        // free *I, but save the next link before its too late...
        J=(*I)->next;
        imguFree(I);
        *I=J;
    }
}

////////////////////////////////////////

/**
 * Pack an regular image into a packed representation using bytes.
 * The hi or low part of the short can be kept, the other is lost.
 *
 * @param[in] I image to pack
 * @param[in] buffer Will store byte-packed image
 * @param[in] mode Which part to keep (KEEP_8_BITS_LOW or KEEP_8_BITS_HIGH)
 */
int imguPack8bit(imgu *I,unsigned char *buffer,int mode)
{
    int i;
    unsigned char *p;
    unsigned short *q;
    if (I==NULL || I->data==NULL) return -1;
    if (buffer==NULL) return -1;

    i=I->xs*I->ys*I->cs;
    p=(unsigned char *)buffer; // first byte of the byte buffer
    q=(unsigned short *)I->data; // first byte of the short buffer

    if( mode==SAVE_8_BITS_LOW ) {
        while( i-- ) *p++ = ((*q++)&255);
    }else if( mode==SAVE_8_BITS_HIGH ) {
        while( i-- ) *p++ = ((*q++)/256);
    }else return -1;
    return(0);
}

/**
 * Unpack a byte-packed image into a regular image represented by shorts.
 *
 * @param[in] I Will store unpacked buffer
 * @param[in] buffer Byte-packed image
 * @param[in] mode How to convert the byte data into short data (MAKE_16_BITS | KEEP_8_BITS)
 */
int imguUnpack8bit(imgu *I,unsigned char *buffer,int mode)
{
    int i;
    unsigned char *p;
    unsigned short *q;

    if (I==NULL || I->data==NULL) return -1;
    if (buffer==NULL) return -1;

    i=I->xs*I->ys*I->cs;
    p=(unsigned char *)buffer + i; // last byte +1 of the byte buffer
    q=(unsigned short *)I->data + i; // last byte +1 of the short buffer

    if( mode==LOAD_16_BITS ) {
        while( i-- ) *--q = (*--p)*257;
    }else if( mode==LOAD_AS_IS ) {
        while( i-- ) *--q = (*--p);
    }else return(-1);
    return(0);
}

////////////////////////////////////////

/**
 * Converts data from 0...255 to 0...IMGU_MAXVAL
 *
 * Data is multiplied by 257.
 *
 * \param[out] Idest Address of destination image
 * \param[in] Isrc Source image
 * \return 0 if ok, -1 if error
 */
int imguConvert8bitTo16bit(imgu **Idest,imgu *Isrc)
{
    int i;
    unsigned short val;
    imgu *J;

    if( Idest==NULL || Isrc==NULL || Isrc->data==NULL ) return(-1);
    if( *Idest!=Isrc ) { if( imguAllocate(Idest,Isrc->xs,Isrc->ys,Isrc->cs) ) return(-1); }
    J=*Idest;

    for(i=0;i<Isrc->xs*Isrc->ys*Isrc->cs;i++) {
        val=Isrc->data[i];
        J->data[i]=(val<<8)+val;
    }
    return(0);
}

/**
 * Converts data from 0...IMGU_MAXVAL to 0...255
 *
 * Data is divided by 256.
 *
 * \param[out] Idest Address of destination image
 * \param[in] Isrc Source image
 * \return 0 if ok, -1 if error
 */
int imguConvert16bitTo8bit(imgu **Idest,imgu *Isrc)
{
    int i;
    imgu *J;

    if( Idest==NULL || Isrc==NULL || Isrc->data==NULL ) return(-1);
    if( *Idest!=Isrc ) { if( imguAllocate(Idest,Isrc->xs,Isrc->ys,Isrc->cs) ) return(-1); }
    J=*Idest;

    for(i=0;i<Isrc->xs*Isrc->ys*Isrc->cs;i++) {
        J->data[i]=(Isrc->data[i]>>8);
    }
    return 0;
}

/**
 * Converts data to grayscale image.
 *
 * Color channels are averaged. If present, the alpha layer is preserved.
 * \attention Idest and Isrc CAN be the same for an "inplace" conversion
 *
 * \param[out] Idest Address of destination image
 * \param[in] Isrc Source image
 * \return 0 if ok, -1 if error
 */
int imguConvertToGray(imgu **Idest,imgu *Isrc)
{
    int i,c,size;
    int sum;
    int cdest;
    imgu *J;

    if( Idest==NULL || Isrc==NULL || Isrc->data==NULL) return(-1);
    if (Isrc->cs<1 || Isrc->cs>4) return -1;
    // deja en gris?
    if (Isrc->cs==1 || Isrc->cs==2) { imguCopy(Idest,Isrc); return 0; }
    cdest=Isrc->cs-2;
    if( *Idest!=Isrc ) { if( imguAllocate(Idest,Isrc->xs,Isrc->ys,cdest) ) return(-1); }
    J=*Idest;

    size=Isrc->xs*Isrc->ys;
    for(i=0;i<size;i++)
    {
        sum=0;
        for(c=0;c<3;c++)
        {
            sum+=Isrc->data[i*Isrc->cs+c];
        }
        J->data[i*cdest]=(sum*2+1)/(3*2);
        if (c<Isrc->cs) J->data[i*cdest+1]=Isrc->data[i*Isrc->cs+c];  //copy alpha
    }

    J->cs=cdest;

    return 0;
}

int imguConvertToLuminance(imgu **Idest,imgu *Isrc)
{
    int i,c,size;
    int cdest;
    vector3 rgb,hls;
    imgu *J;

    if( Idest==NULL || Isrc==NULL || Isrc->data==NULL) return(-1);
    if (Isrc->cs<1 || Isrc->cs>4) return -1;
    // deja en gris?
    if (Isrc->cs==1 || Isrc->cs==2) { imguCopy(Idest,Isrc); return 0; }
    cdest=Isrc->cs-2;
    if( *Idest!=Isrc ) { if( imguAllocate(Idest,Isrc->xs,Isrc->ys,cdest) ) return(-1); }
    J=*Idest;

    size=Isrc->xs*Isrc->ys;
    for(i=0;i<size;i++)
    {        
        for(c=0;c<3;c++)
        {
            rgb[c]=Isrc->data[i*Isrc->cs+c];
        }
        imguRGBtoHLS(rgb,hls);
        J->data[i*cdest]=(pix_t)(hls[1]+0.5);
        if (c<Isrc->cs) J->data[i*cdest+1]=Isrc->data[i*Isrc->cs+c];  //copy alpha
    }

    J->cs=cdest;

    return 0;
}

int imguConvertToRelativeLuminance(imgu **Idest,imgu *Isrc)
{
    int i,c,size;
    int cdest;
    int cval;
    imgu *J;

    if( Idest==NULL || Isrc==NULL || Isrc->data==NULL) return(-1);
    if (Isrc->cs<1 || Isrc->cs>4) return -1;
    // deja en gris?
    if (Isrc->cs==1 || Isrc->cs==2) { imguCopy(Idest,Isrc); return 0; }
    cdest=Isrc->cs-2;
    if( *Idest!=Isrc ) { if( imguAllocate(Idest,Isrc->xs,Isrc->ys,cdest) ) return(-1); }
    J=*Idest;

    size=Isrc->xs*Isrc->ys;
    for(i=0;i<size;i++)
    {        
        //relative luminance (http://en.wikipedia.org/wiki/Luminance_%28relative%29)
        cval=(int)(0.2126*Isrc->data[i*Isrc->cs]+0.7152*Isrc->data[i*Isrc->cs+1]+0.0722*Isrc->data[i*Isrc->cs+2]+0.5);
        if (cval>IMGU_MAXVAL) cval=IMGU_MAXVAL;
        J->data[i*cdest]=(pix_t)(cval);
        if (c<Isrc->cs) J->data[i*cdest+1]=Isrc->data[i*Isrc->cs+c];  //copy alpha
    }

    J->cs=cdest;

    return 0;
}

int imguConvertToHue(imgu **Idest,imgu *Isrc)
{
    int i,c,size;
    int cdest;
    vector3 rgb,hls;
    imgu *J;

    if( Idest==NULL || Isrc==NULL || Isrc->data==NULL) return(-1);
    if (Isrc->cs<1 || Isrc->cs>4) return -1;
    // deja en gris?
    if (Isrc->cs==1 || Isrc->cs==2) { imguCopy(Idest,Isrc); return 0; }
    cdest=Isrc->cs-2;
    if( *Idest!=Isrc ) { if( imguAllocate(Idest,Isrc->xs,Isrc->ys,cdest) ) return(-1); }
    J=*Idest;

    size=Isrc->xs*Isrc->ys;
    for(i=0;i<size;i++)
    {
        for(c=0;c<3;c++)
        {
            rgb[c]=Isrc->data[i*Isrc->cs+c];
        }
        imguRGBtoHLS(rgb,hls);
        J->data[i*cdest]=(pix_t)(hls[0]);
        if (c<Isrc->cs) J->data[i*cdest+1]=Isrc->data[i*Isrc->cs+c];  //copy alpha
    }

    J->cs=cdest;

    return 0;
}

int imguConvertToSaturation(imgu **Idest,imgu *Isrc)
{
    int i,c,size;
    int cdest;
    vector3 rgb,hls;
    imgu *J;

    if( Idest==NULL || Isrc==NULL || Isrc->data==NULL) return(-1);
    if (Isrc->cs<1 || Isrc->cs>4) return -1;
    // deja en gris?
    if (Isrc->cs==1 || Isrc->cs==2) { imguCopy(Idest,Isrc); return 0; }
    cdest=Isrc->cs-2;
    if( *Idest!=Isrc ) { if( imguAllocate(Idest,Isrc->xs,Isrc->ys,cdest) ) return(-1); }
    J=*Idest;

    size=Isrc->xs*Isrc->ys;
    for(i=0;i<size;i++)
    {
        for(c=0;c<3;c++)
        {
            rgb[c]=Isrc->data[i*Isrc->cs+c];
        }
        imguRGBtoHLS(rgb,hls);
        J->data[i*cdest]=(pix_t)(hls[2]);
        if (c<Isrc->cs) J->data[i*cdest+1]=Isrc->data[i*Isrc->cs+c];  //copy alpha
    }

    J->cs=cdest;

    return 0;
}

/**
 * Converts data to color image.
 *
 * Color channels are copied over 3 color channels. If present, the alpha layer is preserved.
 *
 * \param[out] Idest Address of destination image
 * \param[in] Isrc Source image
 * \return 0 if ok, -1 if error
 *
 * @note This function also works in place ( *Idest == Isrc )
 */
int imguConvertToRGB(imgu **Idest,imgu *Isrc)
{
    int i,c;
    imgu *J;
    imgu *Icpy;
    int cdest;
    unsigned char realloc;

    if( Idest==NULL || Isrc==NULL || Isrc->data==NULL ) return(-1);
    // deja en couleurs?
    if (Isrc->cs<1 || Isrc->cs>4) return -1;
    if (Isrc->cs==3 || Isrc->cs==4) { imguCopy(Idest,Isrc); return 0; }
    Icpy=Isrc;
    realloc=0;
    cdest=Isrc->cs+2;
    if( *Idest!=Isrc ) { if( imguAllocate(Idest,Isrc->xs,Isrc->ys,cdest) ) return(-1); }
    else if (Isrc->xs*Isrc->ys*cdest>Isrc->ds) //Idest==Isrc, not enough space, make local copy, reallocate
    {
        Icpy=NULL;
        imguCopy(&Icpy,Isrc);
        realloc=1;
        if( imguAllocate(Idest,Isrc->xs,Isrc->ys,cdest) ) {imguFree(&Icpy);return(-1);}
    }
    J=*Idest;


    for(i=Icpy->xs*Icpy->ys-1;i>=0;i--) {
        for(c=0;c<3;c++) J->data[i*cdest+c]=Icpy->data[i*Icpy->cs];
        if (c<cdest) J->data[i*cdest+c]=Icpy->data[i*Icpy->cs+1]; //copy alpha
    }

    J->xs=Isrc->xs;
    J->ys=Isrc->ys;
    J->cs=cdest;

    if (realloc) imguFree(&Icpy);

    return 0;
}

/**
 * Adds an alpha layer (transparency) to an image structure
 *
 * If this layer already exists in Isrc (i.e. Isrc has 2 or 4 color channels), 
 * only the values of the alpha layer are changed, and no new layer is added.
 *
 * @param Idest the destination image
 * @param Isrc the source image
 * @param alpha the transparency layer. If NULL, the transparency layer is initialized to IMGU_MAXVAL (i.e. white, i.e. opaque)
 * @return 0 on success, -1 on failure. Failure is returned if Idest or Isrc is NULL or if Isrc and alpha are not of the same size.
 */
int imguAddAlphaLayer(imgu **Idest,imgu *Isrc,imgu *alpha)
{
    int i,c,cdest;
    imgu *J;
    imgu *Icpy;
    unsigned char realloc;

    if (Idest==NULL || Isrc==NULL || Isrc->data==NULL) return -1;
    if (Isrc->cs<1 || Isrc->cs>4) return -1;
    if (alpha!=NULL && alpha->cs!=1) return -1;
    if (alpha!=NULL && alpha->xs!=Isrc->xs && alpha->ys!=Isrc->ys) return -1;
    Icpy=Isrc;
    realloc=0;
    if (Isrc->cs==2 || Isrc->cs==4) cdest=Isrc->cs;
    else cdest=Isrc->cs+1;
    if( *Idest!=Isrc) { if( imguAllocate(Idest,Isrc->xs,Isrc->ys,cdest) ) return(-1); }
    else if (Isrc->xs*Isrc->ys*cdest>Isrc->ds) //Idest==Isrc, not enough space, make local copy, reallocate
    {
        Icpy=NULL;
        imguCopy(&Icpy,Isrc);
        realloc=1;
        if( imguAllocate(Idest,Isrc->xs,Isrc->ys,cdest) ) {imguFree(&Icpy);return(-1);}
    }
    J=*Idest;

    for(i=Icpy->xs*Icpy->ys-1;i>=0;i--) {
        for(c=0;c<Icpy->cs;c++) J->data[i*cdest+c]=Icpy->data[i*Icpy->cs+c];
    }

    if (alpha==NULL) 
    {
        for(i=0;i<J->xs*J->ys;i++) {
            J->data[i*cdest+cdest-1]=IMGU_MAXVAL;
        }
    }
    else
    {
        for(i=0;i<J->xs*J->ys;i++) {
            J->data[i*cdest+cdest-1]=alpha->data[i];
        }
    }

    J->xs=Icpy->xs;
    J->ys=Icpy->ys;
    J->cs=cdest;

    if (realloc) imguFree(&Icpy);

    return 0;
}

/**
 * Removes the alpha layer (transparency) of the image data.
 *
 * The alpha layer corresponds to the last channel of a 2 or 4 channel image.
 *
 * @param Idest the destination image
 * @param Isrc the source image
 * @return 0 on success, -1 on failure. Failure is returned if Idest or Isrc is NULL or if Isrc and alpha are not of the same size.
 */
int imguRemoveAlphaLayer(imgu **Idest,imgu *Isrc)
{
    int i,c,cdest;
    imgu *J;

    if (Idest==NULL || Isrc==NULL || Isrc->data==NULL) return -1;
    if (Isrc->cs<1 || Isrc->cs>4) return -1;
    if (Isrc->cs==1 || Isrc->cs==3) { imguCopy(Idest,Isrc); return 0; }
    cdest=Isrc->cs-1;
    if( *Idest!=Isrc) { if( imguAllocate(Idest,Isrc->xs,Isrc->ys,cdest) ) return(-1); }
    J=*Idest;

    for(i=0;i<J->xs*J->ys;i++) {
        for(c=0;c<cdest;c++) J->data[i*cdest+c]=Isrc->data[i*Isrc->cs+c];
    }

    //Isrc->cs=cdest;
    (*Idest)->cs=cdest; // in case *IDest==Isrc

    return 0;
}

////////////////////////////////////////


// scale par un scale direct (utile si sx<1 et sy<1)
static int imguScaleSmaller(imgu **Iscaled,imgu *I,double sx,double sy)
{
    int xs,ys;
    double *accum;
    double *vote;
    int val;
    int xi,yi,i,x,y,c;
    double xx,yy,dx,dy;
    imgu *J;
	// Le dernier point est (int)((xs-1)*sx), la taille est donc +1
    xs=(int)((I->xs-1)*sx)+1;
    ys=(int)((I->ys-1)*sy)+1;

    //printf("scale smaller %f %f -> size %d %d (%f %f)\n",sx,sy,xs,ys);

    accum=(double *)malloc(xs*ys*I->cs*sizeof(double));
    if( accum==NULL ) return -1;

    vote=(double *)malloc(xs*ys*sizeof(double));
    if( vote==NULL ) { free(accum);return -1; }

    for(i=0;i<xs*ys;i++) vote[i]=0.0;
    for(i=0;i<xs*ys*I->cs;i++) accum[i]=0.0;

    for(y=0;y<I->ys;y++)
        for(x=0;x<I->xs;x++) {
            xx=x*sx;
            yy=y*sy;
            xi=(int)(xx);
            yi=(int)(yy);
            dx=xx-xi;
            dy=yy-yi;

            vote[yi*xs+xi]+=(1-dx)*(1-dy);
            if( xi<xs-1 ) vote[yi*xs+xi+1]+=dx*(1-dy);
            if( yi<ys-1 ) vote[(yi+1)*xs+xi]+=(1-dx)*dy;
            if( xi<xs-1 && yi<ys-1 ) vote[(yi+1)*xs+xi+1]+=dx*dy;

            for(c=0;c<I->cs;c++) {
                val=I->data[(y*I->xs+x)*I->cs+c];
                accum[(yi*xs+xi)*I->cs+c]+=(1-dx)*(1-dy)*val;
                if( xi<xs-1 ) accum[(yi*xs+xi+1)*I->cs+c]+=dx*(1-dy)*val;
                if( yi<ys-1 ) accum[((yi+1)*xs+xi)*I->cs+c]+=(1-dx)*dy*val;
                if( xi<xs-1 && yi<ys-1 ) accum[((yi+1)*xs+xi+1)*I->cs+c]+=dx*dy*val;
                //printf("vote val=%f (%f,%f)\n",val,dx,dy);
            }
        }

    //printf("voted\n");

    if (imguAllocate(Iscaled,xs,ys,I->cs)) { free(accum);free(vote);return -1; }
    J=*Iscaled;

    //int k=0;
    for(i=0;i<xs*ys;i++) {
        for(c=0;c<J->cs;c++) {
            if( vote[i]==0.0 ) { /*k++;*/J->data[i*J->cs+c]=0;continue; }
            val=accum[i*J->cs+c]/vote[i]+0.5;
            if( val<0 ) val=0;
            if( val>IMGU_MAXVAL ) val=IMGU_MAXVAL;
            J->data[i*J->cs+c]=val;
        }
    }
    //if( k ) { printf("Scale Smaller was unable to set %d pixels (outside)\n",k); }

    //printf("done\n");

    free(accum);
    free(vote);
    return 0;
}



// scale par un scale inverse (utile si sx>1 et sy>1)
static int imguScaleLarger(imgu **Iscaled,imgu *I,double sx,double sy)
{
    int xs,ys;
    double xx,yy;
    double val[4]; // cs maximum 4
    int v;
    int x,y,c;
    imgu *Isrc,*J;
    unsigned char realloc;
    xs=(int)(I->xs*sx+0.5);
    ys=(int)(I->ys*sy+0.5);
    //printf("Scale Larger (%d,%d) scale=(%f,%f) -> (%f,%f),(%d,%d)\n",I->xs,I->ys,sx,sy,I->xs*sx,I->ys*sy,xs,ys);

    Isrc=NULL;

    if (*Iscaled!=I) {
        // scale to a new image
        Isrc=I;
        realloc=0;
        if (imguAllocate(Iscaled,xs,ys,I->cs)) return -1;
    }else{
        // scale to a the same image : need to make local copy of I
        imguCopy(&Isrc,I);
        realloc=1;
        if( imguAllocate(Iscaled,xs,ys,I->cs) ) {imguFree(&Isrc);return(-1);}
    }
    J=*Iscaled;

    //k=0;
    for(y=0;y<J->ys;y++)
        for(x=0;x<J->xs;x++)
        {
            xx=x/sx;
            yy=y/sy;
            //if( imguInterpolateBicubic(Isrc,xx,yy,val) )
            if( imguInterpolateBilinear(Isrc,xx,yy,val) )  
            {
                if (imguInterpolateClosest(Isrc,xx,yy,val))
                {
                    //k++;
                    for(c=0;c<J->cs;c++) J->data[(y*J->xs+x)*J->cs+c]=0;
                    continue;
                }
            }
            for(c=0;c<J->cs;c++)
            {
                v=(int)(val[c]+0.5);
                if( v<0 ) v=0;
                if( v>IMGU_MAXVAL ) v=IMGU_MAXVAL;
                J->data[(y*J->xs+x)*J->cs+c]=v;
            }
        }

    if (realloc) imguFree(&Isrc);

    //if( k ) { printf("Scale was unable to set %d pixels\n",k); }

    return 0;
}


/**
 * Scales image by factors @p (sx,sy)
 *
 * @param Iscaled the scaled
 * @param I the source image
 * @param sx horizontal scale factor
 * @param sy vertical scale factor
 * @return 0 on success, -1 on failure.
 */
int imguScale(imgu **Iscaled,imgu *I,double sx,double sy)
{
    imgu *J;

    if (Iscaled==NULL) return -1;
    if (I==NULL || I->data==NULL) return -1;

    J=NULL;
    if( sx>=1.0 && sy>=1.0 ) return( imguScaleLarger(Iscaled,I,sx,sy) );
    if( sx<1.0 && sy<1.0 ) return( imguScaleSmaller(Iscaled,I,sx,sy) );
    if( sx<1.0 ) {
        // cas sx<1 et sy>1
        imguScaleSmaller(&J,I,sx,1.0);
        if( J==NULL && J->data==NULL) return -1;
        imguScaleLarger(Iscaled,J,1.0,sy);
        imguFree(&J);
        return 0;
    }else{
        // cas sx>1 et sy<1
        imguScaleSmaller(&J,I,1.0,sy);
        if( J==NULL && J->data==NULL) return -1;
        imguScaleLarger(Iscaled,J,sx,1.0);
        imguFree(&J);
        return 0;
    }
    return(-1); // impossible d'arriver ici
}


//////////////////////////////////////////////////
//////////////////////////////////////////////////
//////////////////////////////////////////////////


/**
 * Extract a sub patch from an image. 
 *
 * If @p xmin,ymin,width,height are such that the patch @p S goes beyond image coordinates @p I,
 * then coordinates modulo I->xs or I->ys are used (for instance: -1 becomes I->xs-1)
 *
 * @param[in] S Patch Image pointer (will be changed)
 * @param[in] I Image from wich the patch is extracted
 * @param[in] xmin,ymin Upper left corner of patch rectangle in image I
 * @param[in] width,height Width and height of rectangle
 * @return 0: success; <0: error
 */
int imguExtractRectangle(imgu **S,imgu *I,int xmin,int ymin,int width,int height)
{
    int x,y,c,xx,yy,index;
    imgu *J;
    unsigned char realloc;
    imgu *srccpy;

    if( S==NULL ) return -1;
    if( I==NULL ) return -1;

    srccpy=I;
    realloc=0;
    if (*S!=I)
    {
        if (I->data!=NULL) {if (imguAllocate(S,width,height,I->cs)) return -1;}
        if (I->complex!=NULL) {if (imguAllocateComplex(S,width,height,I->cs)) return -1;}
    }
    else if (xmin<0 || ymin<0 || xmin+width>I->xs || ymin+height>I->ys)
    {
      srccpy=NULL;
      imguCopy(&srccpy,I);
      realloc=1;
      if (I->data!=NULL) {if (imguAllocate(S,width,height,I->cs)) return -1;}
      if (I->complex!=NULL) {if (imguAllocateComplex(S,width,height,I->cs)) return -1;}
    }
    J=*S;

    for(y=0;y<height;y++)
    {
        yy=ymin+y;
        //while (yy<0) yy+=I->ys;
        //if (modulo) yy%=I->ys;
        //if (yy<0 || yy>=I->ys) continue;
        for(x=0;x<width;x++)
        {
            xx=xmin+x; // pos in original image
            //while (xx<0) xx+=I->xs;
            //if (modulo) xx%=I->xs;
            //if (xx<0 || xx>=I->xs) continue;
            for(c=0;c<I->cs;c++)
            {
                index=(y*width+x)*J->cs+c;
                if (yy<0 || yy>=srccpy->ys || xx<0 || xx>=srccpy->xs)
                {
                  if (I->data!=NULL) J->data[index]=0;
                  if (I->complex!=NULL)
                  {
                    J->complex[index][0]=0;
                    J->complex[index][1]=0;
                  }
                }
                else
                {
                  if (I->data!=NULL) J->data[index]=srccpy->data[(yy*srccpy->xs+xx)*I->cs+c];
                  if (I->complex!=NULL)
                  {
                    J->complex[index][0]=srccpy->complex[(yy*srccpy->xs+xx)*I->cs+c][0];
                    J->complex[index][1]=srccpy->complex[(yy*srccpy->xs+xx)*I->cs+c][1];
                  }
                }
            }
        }
    }

    imguCopyText(*S,I);

    J->ys=height;
    J->xs=width;

    if (realloc) imguFree(&srccpy);

    return 0;
}

int imguExtractRectangleSubPixel(imgu **S,imgu *I,double xmin,double ymin,int width,int height)
{
    int x,y,c;
    double xx,yy;
    imgu *J;
    unsigned char realloc;
    double *vals;
    imgu *srccpy;

    if( S==NULL ) return -1;
    if( I==NULL ) return -1;

    if (*S!=I)
    {
        if (I->data!=NULL) {if (imguAllocate(S,width,height,I->cs)) return -1;}
        if (I->complex!=NULL) {if (imguAllocateComplex(S,width,height,I->cs)) return -1;}
    }
    J=*S;    

    srccpy=I;
    realloc=0;
    //need to copy Isrc if xmin<1 or ymin<1 (because of bilinear interpolation)
    if(xmin<1 || ymin<1)
    {
        srccpy=NULL;
        imguCopy(&srccpy,I);
        realloc=1;
    }

    vals=(double *)(malloc(sizeof(double)*I->cs));

    for(y=0;y<height;y++)
    {
        yy=ymin+y;
        for(x=0;x<width;x++)
        {
            xx=xmin+x; // pos in original image
            if (I->data!=NULL)
            {
              imguInterpolateBilinear(srccpy,xx+0.5,yy+0.5,vals);
              for(c=0;c<I->cs;c++)
              {
                J->data[(y*width+x)*J->cs+c]=(pix_t)(vals[c]+0.5);
              }
            }
            if (I->complex!=NULL)
            {
              imguInterpolateBilinearComplex(srccpy,xx+0.5,yy+0.5,vals,0);
              for(c=0;c<I->cs;c++)
              {
                J->complex[(y*width+x)*J->cs+c][0]=vals[c];
              }
              imguInterpolateBilinearComplex(srccpy,xx+0.5,yy+0.5,vals,1);
              for(c=0;c<I->cs;c++)
              {
                J->complex[(y*width+x)*J->cs+c][1]=vals[c];
              }
            }
        }
    }

    imguCopyText(*S,I);

    J->ys=height;
    J->xs=width;

    if (realloc) imguFree(&srccpy);
    free(vals);

    return 0;
}

int imguAddRectangle(imgu *J,imgu *I,int x,int y)
{
  int i,j,k;
  int ii,jj;
  int Jindex,Iindex;
  unsigned char realloc;
  imgu *Icpy;

  if (J==NULL || I==NULL) return -1;
  if (J->cs!=I->cs) return -1;

  Icpy=NULL;

  realloc=0;
  if (J==I)
  {
    imguCopy(&Icpy,I);
    realloc=1;
  }
  else Icpy=I;

  for (i=0;i<Icpy->ys;i++)  
  {
    ii=i+y;
    for (j=0;j<Icpy->xs;j++)  
    {
      jj=j+x;
      if (imguCheck(J,(double)(jj),(double)(ii))==0)
      {
        for (k=0;k<Icpy->cs;k++)  
        {
          Jindex=(ii*J->xs+jj)*J->cs+k;
          Iindex=(i*Icpy->xs+j)*Icpy->cs+k;
          if (J->data!=NULL && Icpy->data!=NULL)
          {
            J->data[Jindex]=Icpy->data[Iindex];
          }
          if (J->complex!=NULL && Icpy->complex!=NULL)
          {
            J->complex[Jindex][0]=Icpy->complex[Iindex][0];
            J->complex[Jindex][1]=Icpy->complex[Iindex][1]; 
          }
        }
      }
      else if (x>=0) break;
    }
  }

  if (realloc) imguFree(&Icpy);

  return 0;
}

/////////////////////////////////////
/////////////////////////////////////


/**
 * Image comparison with SSD
 *
 * Computes the SSD between an image I and a patch P. The patch is offset by @c (x0,y0) in the image I.
 * If Pmask is not NULL, than it is used as a mask on P. A pixel is counted in the SSD only if the Pmask has value 65535 at that location.
 *
 * @attention Assumes that P is smaller than I
 *
 * @param[in] I Image
 * @param[in] P Patch to compare with Image
 * @param[in] x0,y0 Offset of patch @p P in image @p I
 * @param[in] nb Number of pixels counted in the SSD. Can be NULL.
 * @param[in] Pmask Mask for patch P. Pixels of P must have PMask at 65535 to be counted. Must be the same size as P.
 * @return the SSD value
 *
 */
double imguSSDSubPixel(imgu *I,imgu *P,double x0,double y0,int *nb,imgu *Pmask)
{
    int x,y,c;
    double xx,yy;
    int n;
    int vp;
    double vi;
    double *vals;
    double s,dif;
    if (I==NULL || P==NULL) return -1.0;
    if (I->data==NULL || P->data==NULL) return -1.0;
    if( I->cs != P->cs ) return(-1.0);
    if( Pmask ) {
        if( Pmask->xs!=P->xs || Pmask->ys!=P->ys ) return(-2.0);
        if( Pmask->cs!=1 ) return(-3.0);
    }
    n=0;
    s=0.0;

    vals=(double *)(malloc(sizeof(double)*I->cs));
    for(y=0;y<P->ys;y++) {
        yy=y+y0;
        if( yy<0 || yy>=I->ys ) continue;
        for(x=0;x<P->xs;x++) {
            xx=x+x0;
            if( xx<0 || xx>=I->xs ) continue;
            if( Pmask!=NULL && Pmask->data[y*Pmask->xs+x]<IMGU_MAXVAL ) continue;
            imguInterpolateBilinear(I,xx,yy,vals);
            for(c=0;c<P->cs;c++) {
                vp=P->data[(y*P->xs+x)*P->cs+c];
                vi=vals[c];
                dif=(double)(vi-vp);
                s+=dif*dif;
            }
            n+=P->cs;
        }
    }
    if( nb ) *nb=n;
    free(vals);
    return(s);
}



/**
 * Image comparison with SSD (faster version)
 *
 * Computes the SSD between an image I and a patch P. The patch is offset by @c (x0,y0) in the image I.
 * If Pmask is not NULL, than it is used as a mask on P. A pixel is counted in the SSD only if the Pmask has value 65535 at that location.
 *
 * @attention Assumes that P is smaller than I
 *
 * @param[in] I Image
 * @param[in] P Patch to compare with Image
 * @param[in] x0,y0 Offset of patch @p P in image @p I
 * @param[in] nb Number of pixels counted in the SSD. Can be NULL.
 * @param[in] Pmask Mask for patch P. Pixels of P must have PMask at 65535 to be counted. Must be the same size as P.
 * @return the SSD value
 *
 */
double imguSSD(imgu *I,imgu *P,int x0,int y0,int *nb,imgu *Pmask)
{
    int x,y,yy,c,L,LC;
    int n;
    double s,dif;
    pix_t *p; // in the patch
    pix_t *q; // in the image
    pix_t *m;
    if (I==NULL || P==NULL) return -1.0;
    if (I->data==NULL || P->data==NULL) return -1.0;
    if( I->cs != P->cs ) return(-1.0);
    if( Pmask ) {
        if( Pmask->xs!=P->xs || Pmask->ys!=P->ys ) return(-2.0);
        if( Pmask->cs!=1 ) return(-3.0);
    }
    L=P->xs; // nb de pixels a traverser
    // (L-1)+x0 < I->xs
    // L < I->xs - x0 +1
    if( L >= I->xs - x0 + 1 ) L=I->xs-x0;
    n=0;
    s=0.0;
    for(y=0;y<P->ys;y++) {
        yy=y+y0;
        if( yy<0 || yy>=I->ys ) continue;
        p=P->data+(y*P->xs)*P->cs;
        q=I->data+(yy*I->xs+x0)*I->cs;

        if( Pmask==NULL ) {
            LC=L*P->cs;
            for(x=0;x<LC;x++) {
                dif=(double)((int)*p++ - (int)*q++);
                s+=dif*dif;
            }
            n+=LC;
        }else{
            m=Pmask->data+(y*Pmask->xs); // Pmask->xs same as P->xs
            for(x=0;x<L;x++) {
                if( *m++!=IMGU_MAXVAL ) { p+=P->cs;q+=P->cs;continue; }
                for(c=0;c<P->cs;c++) {
                    dif=(double)((int)*p++ - (int)*q++);
                    s+=dif*dif;
                }
                n+=P->cs;
            }
        }
    }
    if( nb ) *nb=n;
    return(s);
}


//////////////////////////////////////////////////
//////////////////////////////////////////////////
//////////////////////////////////////////////////

// tire de fast_blur de Jamil

/**
 * *IDest == ISrc accepted!
 */
int imguFastBlur5x5(imgu **IDest,imgu *ISrc,int nb_run)
{
int xs,ys,run;
int *SC;
int c,cs;
imgu *J;
        int SR0 , SR1 , SR2 , SR3;
        int tmp1 , tmp2;
        int *SC0 , *SC1, *SC2 , *SC3;
        int row , col;

	xs=ISrc->xs;
	ys=ISrc->ys;
	cs=ISrc->cs;

	if( *IDest == ISrc ) {
		J=ISrc;
	}else{
		imguCopy(IDest,ISrc);
		J=*IDest;
	}

	SC = (int*) malloc( sizeof(int) * xs * 4 );

        SC0 = SC;
        SC1 = SC + xs;
        SC2 = SC + 2 * xs;
        SC3 = SC + 3 * xs;


	for(run=0;run<nb_run;run++) {

	for(c=0;c<cs;c++) {

        memset( SC , 0 , sizeof(int) * xs * 4 );

        for( row = 2; row < ys - 2; row++){

            SR0 = SR1 = SR2 = SR3 = 0;

            for( col = 2; col < xs - 2; col++){
                        tmp1 = PIXEL(J,col,row,c);
                        tmp2 = SR0 + tmp1;
                        SR0 = tmp1;
                        tmp1 = SR1 + tmp2;
                        SR1 = tmp2;
                        tmp2 = SR2 + tmp1;
                        SR2 = tmp1;
                        tmp1 = SR3 + tmp2;
                        SR3 = tmp2;
                        tmp2 = SC0[ col ] + tmp1;
                        SC0[ col ] = tmp1;
                        tmp1 = SC1[ col ] + tmp2;
                        SC1[ col ] = tmp2;
                        tmp2 = SC2[ col ] + tmp1;
                        SC2[ col ] = tmp1;
             			PIXEL(J,col-2,row-2,c) = (128 + SC3[ col ] + tmp2 ) / 256;

                        SC3[ col ] = tmp2;
            }

        }

	} // for c
	} // for run

	free(SC);

    //last row,column pixels are not blurred
    //set them to 0
	for(c=0;c<cs;c++) 
    {
        for( row = 0; row < ys; row++)
        {
            for( col = 0; col < xs; col++)
            {
                if (row>=ys-4 || col>=xs-4) PIXEL(J,col,row,c) = 0;
            }

        }
	}

	return(0);
}


//////////////////////////////////////////////////
//////////////////////////////////////////////////
//////////////////////////////////////////////////

int imguCompress(imgu *I)
{
#ifndef HAVE_ZLIB
    fprintf(stderr,"ZLIB LIBRARY NOT AVAILABLE!\n");
    return -1;
#else
	int k;

	unsigned long img_size = I->xs * I->ys * I->cs * sizeof(short);
	unsigned long extra_size = 12 + 1.3 * img_size / 2;

	k = imguExtendExtra(I, extra_size);
	if(k) {
		fprintf(stderr, "imguCompress: imguAllocateExtra failed\n");
		return -1;
	}

	unsigned char* extra_ptr = (unsigned char *) (I->data + I->xs * I->ys * I->cs); // compressed buffer pointer

	/**printf("imguCompress: img->data = %p; extra_ptr = %p; img_size = %lu; extra_size = %lu\n",
				 I->data, extra_ptr, img_size, extra_size);**/
	//printf("extra_ptr should be %p\n", I->data + img_size);

	// zlib: int compress (Bytef *dest, uLongf *destLen, const Bytef *source, uLong sourceLen);
	k = compress2( extra_ptr, &extra_size, (Bytef*)I->data, img_size, 1 );

	//printf("imguCompress: true compressed_data_size = %lu\n", extra_size);

	switch(k) {	
	case Z_MEM_ERROR:// if there was not enough memory;
		fprintf(stderr, "imguCompress: compress returned Z_MEM_ERROR\n");
		return(-1);

	case Z_BUF_ERROR:// if there was not enough room in the output buffer.
		fprintf(stderr, "imguCompress: compress returned Z_BUF_ERROR\n");
		return(-1);

	case Z_OK:// if success;
		I->compressed_data_size = extra_size;
		return(0);

	default:
		fprintf(stderr, "imguCompress: compress returned %d... unexpected\n", k);
		return(-1);
	}

	return(0);
#endif
}

//////////////////////////////////////////////////
//////////////////////////////////////////////////
//////////////////////////////////////////////////

int imguUncompress(imgu *I)
{
#ifndef HAVE_ZLIB
    fprintf(stderr,"ZLIB LIBRARY NOT AVAILABLE!\n");
    return -1;
#else
	int k;

	if (I->compressed_data_size <= 0) {
		fprintf(stderr, "imguUncompress: no data to uncompress\n");
		return -1;
	}

	unsigned long img_size = I->xs * I->ys * I->cs * sizeof(short);
	unsigned char* extra_ptr = (unsigned char *) (I->data + I->xs * I->ys * I->cs); // compressed buffer pointer
	
	/**printf("imguUncompress: img->data = %p; extra_ptr = %p; img_size = %lu; extra_size = %lu\n",
				 I->data, extra_ptr, img_size, I->compressed_data_size);**/
	//printf("extra_ptr should be %p\n", I->data + img_size);

	// zlib: int uncompress (Bytef *dest, uLongf *destLen, const Bytef *source, uLong sourceLen);
	k = uncompress( (Bytef*)I->data, &img_size, extra_ptr, I->compressed_data_size );
	
	switch(k) {	
	case Z_DATA_ERROR: // if the input data was corrupted. 
		fprintf(stderr, "imguUncompress: uncompress returned Z_DATA_ERROR\n");
		return(-1);

	case Z_MEM_ERROR:// if there was not enough memory;
		fprintf(stderr, "imguUncompress: uncompress returned Z_MEM_ERROR\n");
		return(-1);

	case Z_BUF_ERROR:// if there was not enough room in the output buffer.
		fprintf(stderr, "imguUncompress: uncompress returned Z_BUF_ERROR\n");
		return(-1);

	case Z_OK:// if success;
		return(0);

	default:
		fprintf(stderr, "imguUncompress: compress returned %d... unexpected\n", k);
		return(-1);
	}

	return(0);
#endif
}

//////////////////////////////////////////////////
//////////////////////////////////////////////////
//////////////////////////////////////////////////
//
#ifdef SKIP

int main(int argc,char *argv[])
{
    char InName[100];
    imginfo IA;
    int k;
    strcpy(InName,"toto.png");
    if( argc>1 ) strcpy(InName,argv[1]);

    k=loadpng(InName,&IA);

    SaveImage("yo.ppm",&IA);

    k=savepng("yo.png",&IA);
}

#endif

