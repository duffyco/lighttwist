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

#include <string.h>

#include "imgu.h"

#ifdef HAVE_LIBNETPBM
#include <pam.h>
#endif


int imguLoadPNM(imgu **I,const char *name,unsigned char option)
{
#ifndef HAVE_LIBNETPBM
    fprintf(stderr,"imguLoadPNM: netpbm not available.\n");

    FILE *fimg;
    unsigned char *readdata;
    unsigned short *data;
    /* data : [XSize*YSize*ZSize)], Acces=[(Y*XSize+X)(*Zsz)+(z)] */
    int P,x,y,c,index;
    int max;
    int XSize,YSize,CSize;
    char TBuf[100];
    char c1,c2;
    /* default values */

    if (I==NULL) return -1;

    fimg=fopen(name,"rb");
    if( fimg==NULL ) {
        fprintf(stderr,"LoadImage: Unable to open '%s'\n",name);return -1;
    }

    /* Read header */
    if( fgets(TBuf,100,fimg)!=TBuf ) {
        fprintf(stderr,"LoadImage: Unable to read header of '%s'\n",name);
        fclose(fimg);
        return -1;
    }
    if( strncmp(TBuf,"P5",2)==0 ) CSize=1;      /* pgm */
    else if( strncmp(TBuf,"P6",2)==0 ) CSize=3; /* ppm */
    else {
        //fprintf(stderr,"LoadImage: Unsupported PBM format for '%s'\n",name);
        fclose(fimg);
        return -1;
    }
    if (TBuf[2]=='\n' || (TBuf[2]==' ' && TBuf[3]=='\n'))
    {
        /* Skip comments */
        do {
            if( fgets(TBuf,100,fimg)!=TBuf ){fclose(fimg); return -1;}
        } while( TBuf[0]=='#' || TBuf[0]=='\n');
        /* xres/yres */
        sscanf(TBuf,"%d %d",&XSize,&YSize);
        if (TBuf[4]==' ')
        {
            sscanf(TBuf,"%d      %d",&XSize,&YSize);
        }

        /* Number of gray , usually 255, Just skip it */
        if( fgets(TBuf,100,fimg)!=TBuf ) {fclose(fimg);return -1;}
        sscanf(TBuf,"%d",&max);
    }
    else
    {
        sscanf(TBuf,"%c%c %d %d %d",&c1,&c2,&XSize,&YSize,&CSize);
    }
    /* print a little info about the image */
#ifdef VERBOSE
    printf("Image x and y size in data: %d %d\n",XSize,YSize);
    printf("Image zsize in channels: %d\n",CSize);
    printf("Image pixel min and max: %d %d\n",0,255);
#endif
    imguAllocate(I,XSize,YSize,CSize);
    data=(*I)->data;

    if (max>255) readdata=(unsigned char *)(malloc(sizeof(unsigned char)*XSize*CSize*2));
    else readdata=(unsigned char *)(malloc(sizeof(unsigned char)*XSize*CSize));

    int flipY=(InternalFormat!=ExternalFormat);
    int k;

    if( readdata==NULL ) { fclose(fimg); return -1; }
    for(y=0;y<YSize;y++) {
        if( flipY ) P=(YSize-1-y)*XSize; else P=y*XSize;
        if (max>255) k=fread(readdata,1,XSize*CSize*2,fimg);
        else k=fread(readdata,1,XSize*CSize,fimg);
        index=0;
        for (x=0;x<XSize;x++)
        {
            for (c=0;c<CSize;c++)
            {
                if (max>255) 
                {
                  (*I)->data[(P+x)*CSize+c]=readdata[2*index]<<8;
                  (*I)->data[(P+x)*CSize+c]+=readdata[2*index+1];
                }
                else (*I)->data[(P+x)*CSize+c]=readdata[index];
                index++;
            }
        }
    }
    free(readdata);

    (*I)->data=data;

    (*I)->xs=XSize;
    (*I)->ys=YSize;
    (*I)->cs=CSize;

    if (max<=255 && option==LOAD_16_BITS)
    {
      imguConvert8bitTo16bit(I,(*I));
    }

    fclose(fimg);
    return 0;

#else //NETPBM liB
    struct pam inpam;
    FILE *F;
    char *argv[10];
    int argc=1;

    if (I==NULL) return -1;

    F=fopen(name,"rb");
    if( F==NULL ) return -1;

    argv[0]="allo";
    argv[1]=NULL;

    pnm_init(&argc,argv);

    pnm_readpaminit(F, &inpam, sizeof(inpam));

    /**
      printf("Yo! w=%d h=%d\n",inpam.width,inpam.height);
      printf("tupe_type='%s'  format=%d\n",inpam.tuple_type,inpam.format);
      printf("depth=%d maxval=%d\n",inpam.depth,inpam.maxval);
      printf("bytespersample=%d\n",inpam.bytes_per_sample);
      printf("size=%d len=%d sizeof=%d\n",inpam.size,inpam.len,sizeof(inpam));
      printf("plainformat=%d\n",inpam.plainformat);
     **/

    imguAllocate(I,inpam.width,inpam.height,inpam.depth);

    tuple *tuplerow;

    tuplerow = pnm_allocpamrow(&inpam);

    int x,y,yy,c;
    int flipY=(InternalFormat!=ExternalFormat);


    for (y=0; y< inpam.height; y++) {
        if( flipY ) yy=(*I)->ys-1-y; else yy=y;
        pnm_readpamrow(&inpam, tuplerow);
        for (x = 0; x < inpam.width; x++) {
            for (c=0; c<inpam.depth;c++) {
                (*I)->data[(yy*(*I)->xs+x)*(*I)->cs+c]=tuplerow[x][c];
            }
        }
        //pnm_writepamrow(&outpam, tuplerow);
    }

    if (option==LOAD_16_BITS && inpam.bytes_per_sample==1) imguConvert8bitTo16bit(I,(*I));

    pnm_freepamrow(tuplerow);
    return 0;
#endif
}


/**
 * Save a PNM image (pgm or ppm format)
 *
 * @attention Only save an image with 1 or 3 channels
 *
 */
int imguSavePNM(imgu *I,const char *name,unsigned char option)
{
#ifndef HAVE_LIBNETPBM
    fprintf(stderr,"imguSavePNM: netpbm not available.\n");

    FILE *fimg;
    int P,x,y,c,index;
    unsigned char *writedata;
    unsigned short *data;

    if( I==NULL || I->data==NULL ) return(-1);

    if( I->cs!=1 && I->cs!=3 ) {
        fprintf(stderr,"SaveImage: ZSize=%d unsupported\n",I->cs);
        return(-1);
    }

#ifdef VERBOSE
    printf("Saving %d plane image '%s'\n",I->cs,Name);
#endif

    data=I->data;

    fimg=fopen(name,"wb");
    if( fimg==NULL ) {
        fprintf(stderr,"SaveImage: Unable to open '%s'\n",name);return(-1);
    }

    if( I->cs==1 ) fprintf(fimg,"P5\n"); /* pgm */
    else if( I->cs==3 ) fprintf(fimg,"P6\n");    /* ppm */
    else return(-1); /* impossible */

    fprintf(fimg,"%d %d\n",I->xs,I->ys);
    if (option==SAVE_16_BITS) fprintf(fimg,"%d\n",IMGU_MAXVAL);
    else fprintf(fimg,"255\n");

    if (option==SAVE_16_BITS) writedata=(unsigned char *)(malloc(sizeof(unsigned char)*I->xs*I->cs*2));
    else writedata=(unsigned char *)(malloc(sizeof(unsigned char)*I->xs*I->cs));
    if( writedata==NULL ) { fclose(fimg); return(-1); }

    int flipY=(InternalFormat!=ExternalFormat);

    for(y=0;y<I->ys;y++) {
        if( flipY ) P=(I->ys-1-y)*I->xs; else P=y*I->xs;
        index=0;
        for (x=0;x<I->xs;x++)
        {
            for (c=0;c<I->cs;c++)
            {
                if (option==SAVE_16_BITS) 
                {
                  writedata[2*index]=(unsigned char)(I->data[(P+x)*I->cs+c]>>8);
                  writedata[2*index+1]=(unsigned char)(I->data[(P+x)*I->cs+c]);
                }
                else if (option==SAVE_8_BITS_HIGH) writedata[index]=(unsigned char)(I->data[(P+x)*I->cs+c]>>8); 
                else writedata[index]=(unsigned char)(I->data[(P+x)*I->cs+c]);
                index++;
            }
        }
        if (option==SAVE_16_BITS) fwrite(writedata,1,I->xs*I->cs*2,fimg);
        else fwrite(writedata,1,I->xs*I->cs,fimg);
    }
    fclose(fimg);
    free(writedata);

    return(0);

#else //NETPBM lib
    FILE *F;
    struct pam outpam;
    tuple *tuplerow;
    int yy;
    int argc=1;
    char *argv[10];

    if( I==NULL || I->data==NULL ) return(-1);

    argv[0]="allo";
    argv[1]=NULL;


    pnm_init(&argc,argv);

    outpam.size=outpam.len=sizeof(outpam);
    if( I->cs==1 ) {
        outpam.format=RPGM_FORMAT;
        strcpy(outpam.tuple_type,"GRAYSCALE");
    }else if( I->cs==3 ) {
        outpam.format=RPPM_FORMAT;
        strcpy(outpam.tuple_type,"RGB");
    }else {
        printf("can only save 1 or 3 channels to pgm or ppm\n");
        return(-1);
    }
    //printf("outformat=%d\n",outpam.format);

    outpam.plainformat=0; // 1 for ascii, 0 for raw

    outpam.height=I->ys;
    outpam.width=I->xs;
    outpam.depth=I->cs;

    // trouve le max de I.
    if (option==SAVE_16_BITS) outpam.maxval=IMGU_MAXVAL;
    else outpam.maxval=255;

    outpam.bytes_per_sample=outpam.maxval==255?1:2;

    F=fopen(name,"wb");
    if( F==NULL ) return(-1);
    outpam.file=F;

    pnm_writepaminit(&outpam);
    tuplerow = pnm_allocpamrow(&outpam);

    int flipY=(InternalFormat!=ExternalFormat);

    int x,y,c;
    for(y=0;y<I->ys;y++) {
        yy=flipY?(I->ys-1-y):y;
        for(x=0;x<I->xs;x++) {
            for(c=0;c<I->cs;c++) {
                if (option==SAVE_16_BITS) tuplerow[x][c]=I->data[(yy*I->xs+x)*I->cs+c];
                else if (option==SAVE_8_BITS_HIGH) tuplerow[x][c]=(unsigned char)(I->data[(yy*I->xs+x)*I->cs+c]>>8);
                else tuplerow[x][c]=(unsigned char)(I->data[(yy*I->xs+x)*I->cs+c]); 
            }
        }
        pnm_writepamrow(&outpam, tuplerow);
    }

    fclose(F);
    pnm_freepamrow(tuplerow);
    return(0);
#endif
}


