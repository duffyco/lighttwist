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
#include <string.h>
#include <math.h>

#include <imgu.h>

#define STR_LEN 255

int main(int argc,char *argv[])
{
    FILE *fp;
    int i;
    imgu *seq,*currI,*itemp;
    imgu *colorseq; //sequence that contains fft phase and magnitude info in color format (contains power info if option is selected)
    imgu *hwindow,*vwindow,*twindow; //horizontal,vertical and temporal window functions
    char inFilename[STR_LEN];
    char outFilename[STR_LEN];
    int power,fft3d;
    double mean;

    seq=NULL;
    itemp=NULL;
    colorseq=NULL;
    hwindow=NULL;
    vwindow=NULL;
    twindow=NULL;
    inFilename[0]='\0';
    outFilename[0]='\0';
    power=0;
    fft3d=0;
    for(i=1;i<argc;i++) {
        if( strcmp("-i",argv[i])==0 ) {
            strcpy(inFilename,argv[i+1]);
            i++;
            continue;
        }
        if( strcmp("-o",argv[i])==0 ) {
            strcpy(outFilename,argv[i+1]);
            i++;
            continue;
        }
        if( strcmp("-power",argv[i])==0 ) {
            power=1;
            continue;
        }
        if( strcmp("-3d",argv[i])==0 ) {
            fft3d=1;
            continue;
        }
    }

    if (inFilename[0]=='\0' || outFilename[0]=='\0'){ fprintf(stderr,"Syntax: pngfft -i <input filename> -o <output filename> [-power] [-3d]\n");exit(0); }

    imguLoadMulti(&seq,inFilename,LOAD_16_BITS);
    if (seq==NULL){ fprintf(stderr,"Error loading %s\n",inFilename);exit(0); }
    currI=seq;
    while(currI!=NULL)
    {
        imguConvertToGray(&currI,currI);
        currI=currI->next;
    }

    if (fft3d)
    {
        //put all images in a single image, each image is a color channel
        imguConcat(&seq,seq,0,imguCount(seq));
    }

    imguGaussianWindow(&hwindow,seq->xs);
    imguGaussianWindow(&vwindow,seq->ys);
    //make window vertical by changing dimensions
    vwindow->ys=vwindow->xs;
    vwindow->xs=1;
    imguGaussianWindow(&twindow,seq->cs);
    //make window temporal by changing dimensions
    twindow->cs=twindow->xs;
    twindow->xs=1;

    currI=seq;
    while(currI!=NULL)
    {
        imguConvertToComplex(&currI,currI);
        imguSubtractMean(&currI,currI,-1,&mean);
        imguWindowComplex(&currI,currI,hwindow);
        imguWindowComplex(&currI,currI,vwindow);
        imguWindowComplex(&currI,currI,twindow);
        //uncomment the following lines to view a frame with window
        //imguConvertFromComplex(&itemp,currI,REAL,COMPLEX_SCALE);
        //if (fft3d) imguSplit(&itemp,itemp,1);
        //imguSaveMulti(itemp,"windowimg.png",IMGU_NOCOMPRESSION,SAVE_16_BITS);

        imguFFTForward(&currI,currI); 
        //bandpass filter between frequencies 32 and 2.0*currI->xs
        imguSpatialHardBandPass(&currI,currI,32.0,2.0*currI->xs);
        imguAdjustQuadrants(&currI,currI);
        if (power)
        {        
            imguPower(&currI,currI);
            imguLog(&currI,currI,MAGN);
            imguConvertFromComplex(&itemp,currI,MAGN,COMPLEX_SCALE);
        }
        else
        {
            imguConvertFromComplexToColor(&itemp,currI);
        }
        imguAddToMulti(&colorseq,itemp);
        itemp=NULL;
        currI=currI->next;
    }

    if (fft3d)
    {
        imguSplit(&colorseq,colorseq,1);
    }

    imguSaveMulti(colorseq,outFilename,NO_COMPRESSION,SAVE_16_BITS);

    imguFreeMulti(&seq);
    imguFreeMulti(&colorseq);
    imguFree(&itemp);
    imguFree(&hwindow);
    imguFree(&vwindow);
    imguFree(&twindow);
}





