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

#include "imgu.h"

int imguConcat(imgu **I,imgu *seq,int offset,int nbimgs)
{
    int i,j,k,l,index;
    int xsize,ysize,csize;
    int firstxs,firstys;
    imgu *currimg;
    int csum,ic;
    imgu *seqcpy;
    unsigned char realloc;

    if (I==NULL) return -1;
    if (seq==NULL || seq->data==NULL) return -1;

    for(i=0;i<offset;i++)
    {
      if (seq!=NULL) seq=seq->next;
      else return -1;
    }

    firstxs=seq->xs;
    firstys=seq->ys;

    csum=0;
    i=0;
    currimg=seq;
    while(currimg!=NULL && i<nbimgs)
    {
       csum+=currimg->cs;
       if (currimg->xs!=firstxs) return -1;
       if (currimg->ys!=firstys) return -1;
       currimg=currimg->next;
       i++;
    }

    if (i<nbimgs) return -1; //not enough images

    xsize=firstxs;
    ysize=firstys;
    csize=csum;

    realloc=0;
    seqcpy=seq;
    if ((*I)==seq)
    {
      realloc=1;
      seqcpy=NULL;
      imguCopyMulti(&seqcpy,seq);
      imguFreeMulti(I);
    }

    if (seq->data!=NULL)
    {
      if (imguAllocate(I,xsize,ysize,csize)) {if (realloc) imguFreeMulti(&seqcpy);return -1;}
    }
    if (seq->complex!=NULL)
    {
      if (imguAllocateComplex(I,xsize,ysize,csize)) {if (realloc) imguFreeMulti(&seqcpy);return -1;}
    }
    imguCopyText(*I,seqcpy);

    currimg=seqcpy;
    i=0;
    ic=0;
    while(currimg!=NULL && currimg->data!=NULL && i<nbimgs)
    {
        index=0;
        for(j=0;j<ysize;j++)
        {
            for(k=0;k<xsize;k++)
            {
                for(l=0;l<currimg->cs;l++)
                {
                    if (currimg->data!=NULL) (*I)->data[(j*xsize+k)*csize+ic+l]=currimg->data[index];
                    if (currimg->complex!=NULL)
                    {
                      (*I)->complex[(j*xsize+k)*csize+ic+l][0]=currimg->complex[index][0];
                      (*I)->complex[(j*xsize+k)*csize+ic+l][1]=currimg->complex[index][1];
                    }
                    index++;
                }
            }
        }
        ic+=currimg->cs;
        currimg=currimg->next;
        i++;
    }

    if (realloc) imguFreeMulti(&seqcpy);

    return 0;
}

int imguSplit(imgu **seq,imgu *I,int nb_channels_per_img)
{
    int i;
    imgu *currimg;
    imgu *Icpy;
    unsigned char realloc;

    if (I==NULL) return -1;
    if (seq==NULL) return -1;
    if (nb_channels_per_img<1) return -1;

    realloc=0;
    Icpy=I;
    if ((*seq)==I)
    {
      realloc=1;
      Icpy=NULL;
      imguCopy(&Icpy,I);
    }

    imguFreeMulti(seq);
    currimg=NULL;

    i=0;
    while(1)
    {
      if (imguSelectChannels(&currimg,Icpy,i,nb_channels_per_img)) break;
      imguAddLastMulti(seq,currimg);
      //imguCopyText(currimg,Icpy);
      currimg=NULL;
      i+=nb_channels_per_img;
    }

    if (realloc) imguFree(&Icpy);

    //no images where split
    if (i==0) return -1;

    return 0;
}

int imguSelect(imgu **Iselect,imgu *I,int start_index,int nb_imgs)
{
    int i;
    imgu *currI,*prevI;

    if (Iselect==NULL) return -1;
    if (I==NULL) return -1;

    imguCopyMulti(Iselect,I);

    prevI=NULL;
    for (i=0;i<start_index;i++)
    {
      prevI=(*Iselect);
      if ((*Iselect)!=NULL) (*Iselect)=(*Iselect)->next;
      imguFree(&prevI);
    }

    currI=(*Iselect);
    for (i=0;i<nb_imgs-1;i++)
    {
      if (currI!=NULL) currI=currI->next;
    }
   
    if (currI!=NULL) imguFreeMulti(&(currI->next));

    return 0;
}

int imguSelectChannels(imgu **Iselect,imgu *img,int start_channel,int nb_channels)
{
    int i,j,k;
    int xsize,ysize,csize;
    int index;

    if (Iselect==NULL) return -1;
    if (img==NULL) return -1;

    xsize=img->xs;
    ysize=img->ys;
    csize=img->cs;

    if (start_channel<0 || start_channel>=csize) return -1;

    if (start_channel+nb_channels>csize) nb_channels=csize-start_channel; 

    if ((*Iselect)!=img)
    {
      if (img->data!=NULL) imguAllocate(Iselect,xsize,ysize,nb_channels);
      if (img->complex!=NULL) imguAllocateComplex(Iselect,xsize,ysize,nb_channels);
    }
    else (*Iselect)->cs=nb_channels;
    index=0;
    for (i=0;i<ysize;i++)
    {
        for (j=0;j<xsize;j++)
        {
            for (k=0;k<nb_channels;k++)
            {
              if (img->data!=NULL) (*Iselect)->data[index]=img->data[(i*xsize+j)*csize+start_channel+k];
              if (img->complex!=NULL)
              {
                (*Iselect)->complex[index][0]=img->complex[(i*xsize+j)*csize+start_channel+k][0];
                (*Iselect)->complex[index][1]=img->complex[(i*xsize+j)*csize+start_channel+k][1];
              }
              index++;
            }
        }
    }

    imguCopyText(*Iselect,img);

    return 0;
}

int imguSubtract(imgu **Isub,imgu *img1,imgu *img2,pix_t offset)
{
    int i,size;
    int temp;
    int xsize,ysize,csize;

    if (Isub==NULL) return -1;
    if (img1==NULL || img1->data==NULL) return -1;
    if (img2==NULL || img2->data==NULL) return -1;

    if (img1->xs!=img2->xs) return -1;
    if (img1->ys!=img2->ys) return -1;
    if (img1->cs!=img2->cs) return -1;

    xsize=img1->xs;
    ysize=img1->ys;
    csize=img1->cs;

    if ((*Isub)!=img1 && (*Isub)!=img2) imguAllocate(Isub,xsize,ysize,csize);

    size=xsize*ysize*csize;
    for (i=0;i<size;i++)
    {
        temp=offset;
        temp+=img1->data[i];
        temp-=img2->data[i];
        if (temp<0) temp=0;
        if (temp>IMGU_MAXVAL) temp=IMGU_MAXVAL;
        (*Isub)->data[i]=(pix_t)(temp);
    }

    return 0;
}

static int compare_pixels(const void *a, const void *b){
    const pix_t *a_val = (const pix_t *)a;
    const pix_t *b_val = (const pix_t *)b;
    return a_val[0] < b_val[0] ? -1 : (a_val[0] == b_val[0] ? 0 : 1);
}

int imguMedianFilter(imgu **Idest,imgu *Isrc, int wsize)
{
    int i,j,k,l,m,c;
    int mindex,msize,med;
    int b; //boundary
    pix_t *mdata;
    imgu *Icpy;
    unsigned char realloc;

    if (wsize<0 || wsize%2==0) return -1;

    if(Idest==NULL || Isrc==NULL || Isrc->data==NULL) return -1;

    Icpy=Isrc;
    realloc=0;

    if( *Idest!=Isrc ) { if(imguAllocate(Idest,Isrc->ys,Isrc->xs,Isrc->cs)) return -1; }
    else
    {
        Icpy=NULL;
        imguCopy(&Icpy,Isrc);
        realloc=1;
        if(imguAllocate(Idest,Isrc->xs,Isrc->ys,Isrc->cs)) {imguFree(&Icpy);return -1;}
    }

    b=wsize/2;
    msize=wsize*wsize;
    med=msize/2;
    mdata=(pix_t *)(malloc(sizeof(pix_t)*msize));

    for (i=b;i<Icpy->ys-b;i++)
    {
      for (j=b;j<Icpy->xs-b;j++)
      {
        for (c=0;c<Icpy->cs;c++)
        {
          mindex=0;
          for (l=-b;l<=b;l++)
          { 
            for (m=-b;m<=b;m++)
            { 
              mdata[mindex]=PIXEL(Icpy,j+l,i+m,c);
              mindex++;
            }
          }
          qsort(mdata, msize, sizeof(pix_t), compare_pixels);          
          PIXEL((*Idest),j,i,c)=mdata[med];
        }
      }
    }

    if (realloc) imguFree(&Icpy);
    free(mdata);

    return 0;
}

int imguEqualize(imgu **dest,imgu *src,imgu *src_goal,int hsize)
{
    int i,j,c;
    int index;
    matrix *hist;
    matrix *hist2;
    matrix *goalhist;
    matrix *goalhist2;
    double dev,dist,temp;
    vector4 sum,count;
    int xsize,ysize,csize;
    int val;

    if (dest==NULL || src==NULL || src->data==NULL) return -1;
    if (src_goal!=NULL && (src_goal->data==NULL || src->cs!=src_goal->cs)) return -1;
    if (src->cs>4) return -1;
    if ((*dest)!=src)
    {
      if (imguAllocate(dest,src->xs,src->ys,src->cs)) return -1;
    }

    hist=NULL;
    hist2=NULL;
    goalhist=NULL;
    goalhist2=NULL;     

    matAllocate(&hist,src->cs*hsize,1);
    matAllocate(&hist2,src->cs*hsize,1);
    matAllocate(&goalhist,src->cs*hsize,1);
    matAllocate(&goalhist2,src->cs*hsize,1);

    xsize=src->xs;
    ysize=src->ys;
    csize=src->cs;

    dev=(hsize)/6.0;
    for (c=0;c<csize;c++)
    {
      sum[c]=0;
      count[c]=0;
      for (i=0;i<hsize;i++)
      {
          hist->values[c*hsize+i]=0;
          hist2->values[c*hsize+i]=0;
          goalhist->values[c*hsize+i]=0;  
      }

      //define goal histogram
      //for now, this is gaussian centered at hsize/2 (this could also be made as an input parameter...)
      //these are relative weights, sum does not have to be 1           
      if (src_goal==NULL)
      {
        for (i=0;i<hsize;i++)
        {
          goalhist->values[c*hsize+i]=exp(-0.5*(i-hsize/2.0)*(i-hsize/2.0)/(dev*dev));
          //goalhist[i]=1.0;
        }
      }
      else //make histogram of src_goal
      {
        index=0;
        for (i=0;i<ysize;i++)
        {
          for (j=0;j<xsize;j++)
          {
            val=src_goal->data[index*csize+c];
            if (val) //0 values may mean hole in image...
            {
              goalhist->values[c*hsize+val*(hsize-1)/IMGU_MAXVAL]++;
            }
            index++;
          }
        }
      }
      for (i=0;i<hsize;i++)
      {
          sum[c]+=goalhist->values[c*hsize+i];
          goalhist2->values[c*hsize+i]=0;
      }
    }

    /*for(c=0;c<csize;c++)
    {
      printf("SUM: %f\n",sum[c]);
      for (i=0;i<hsize;i++)
      {
        printf("h(%d)\t%f\n",i,goalhist->values[c*hsize+i]);
      }
    }*/

    for(c=0;c<csize;c++)
    {
        index=0;
        for (i=0;i<ysize;i++)
        {
            for (j=0;j<xsize;j++)
            {
                val=src->data[index*csize+c];
                if (val) //0 values may mean hole in image...
                {
                  hist->values[c*hsize+val*(hsize-1)/IMGU_MAXVAL]++;
                  count[c]++;
                }
                index++;
            }
        }
    }

    for(c=0;c<csize;c++)
    {
      if (count[c]==0)
      {
        matFree(&hist);
        matFree(&hist2);
        matFree(&goalhist);
        matFree(&goalhist2);
        return 0;
      }
    }

    for(c=0;c<csize;c++)
    {
      for (i=0;i<hsize;i++)
      {
        for (j=0;j<=i;j++)
        {
            hist2->values[c*hsize+i]+=hist->values[c*hsize+j];
        }
        for (j=0;j<=i;j++)
        {
            goalhist2->values[c*hsize+i]+=goalhist->values[c*hsize+j];
        }
        //hist2->values[c*hsize+i]*=(((double)(IMGU_MAXVAL))/(xsize*ysize));
        hist2->values[c*hsize+i]*=(((double)(IMGU_MAXVAL))/count[c]);
        goalhist2->values[c*hsize+i]*=(((double)(hsize-1))/sum[c]);
      }
    }

    //from here, hist2 and goalhist2 are cumulative density functions going from 0 to IMGU_MAXVAL
    index=0;
    for (i=0;i<ysize;i++)
    {
        for (j=0;j<xsize;j++)
        {
            for(c=0;c<csize;c++)
            {
                (*dest)->data[index]=(pix_t)(hist2->values[c*hsize+src->data[index]*(hsize-1)/IMGU_MAXVAL]);
                index++;
            }
        }
    }
    //we could return here and histogram of pixel values would be uniform
    //we continue to get our goal histogram
    for(c=0;c<csize;c++)
    {
      for (i=0;i<hsize;i++)
      {
        for (j=0;j<hsize;j++)
        {
            temp=fabs(goalhist2->values[c*hsize+j]-i);
            if (j==0 || temp<dist+1e-8)
            {
                dist=temp;
            }
            else break;
        }
        goalhist->values[c*hsize+i]=j-1.0;
      }
    }

    /*for(c=0;c<csize;c++)
    {
      for (i=0;i<hsize;i++)
      {
        printf("h(%d)\t%f\t%f\n",i,hist2->values[c*hsize+i],goalhist->values[c*hsize+i]);
      }
    }
    exit(0);*/

    index=0;
    for (i=0;i<ysize;i++)
    {
        for (j=0;j<xsize;j++)
        {
            for(c=0;c<csize;c++)
            {
                (*dest)->data[index]=(pix_t)(goalhist->values[c*hsize+(*dest)->data[index]*(hsize-1)/IMGU_MAXVAL]*IMGU_MAXVAL/(hsize-1));
                index++;
            }
        }
    }

    imguSaveMatrix(hist2,(*dest),"EQUALIZE_HIST_LUT");
    imguSaveMatrix(goalhist,(*dest),"EQUALIZE_GOALHIST_LUT");

    matFree(&hist);
    matFree(&hist2);
    matFree(&goalhist);
    matFree(&goalhist2);

    return 0;
}

/**
 * Remap the image intensity range to the new range (min,max)
 *
 * @param[out] dest destination image pointer
 * @param[in] src source image (can be same as @p dest)
 * @param[in] min new value of minimum image intensity
 * @param[in] max new value of maximum image intensity
 * @return 0:sucess <0:error
 *
 */
int imguMapRange(imgu **dest,imgu *src, pix_t min, pix_t max)
{
    int i,size;
    double temp;
    pix_t valmax,valmin;

    if (dest==NULL || src==NULL || src->data==NULL) return -1;
    if ((*dest)!=src)
    {
      if (imguAllocate(dest,src->xs,src->ys,src->cs)) return -1;
    }

    size=src->xs*src->ys*src->cs;

    valmin=valmax=src->data[0];
    for(i=0;i<size;i++)
    {
	    if( valmin>src->data[i] ) valmin=src->data[i];
	    if( valmax<src->data[i] ) valmax=src->data[i];
    }

    if (valmin==valmax) return -1;

    for(i=0;i<size;i++)
    {
        temp=((double)(src->data[i]-valmin))/(valmax-valmin);
        (*dest)->data[i]=(pix_t)(temp*(max-min)+min);
        //(*dest)->data[i]=(pix_t)(((src->data[i]-valmin)*(max-min)*2+1)/(2*(valmax-valmin))+min);
    }

    return 0;
}

int imguAddNoise(imgu **dest,imgu *src,double dev)
{
    int i,j,k,c,l,index;
    int xsize,ysize,csize,cs_notrans;
    double rd,noise;
    int newval;
    imgu *gaussianFilter,*gaussianFilterSum;

    if (dest==NULL || src==NULL || src->data==NULL) return -1;
    if ((*dest)!=src)
    {
      if (imguAllocate(dest,src->xs,src->ys,src->cs)) return -1;
    }

    gaussianFilter=NULL;
    gaussianFilterSum=NULL;

    xsize=src->xs;
    ysize=src->ys;
    csize=src->cs;
    cs_notrans=csize;
    if (cs_notrans==4 || cs_notrans==2) cs_notrans--;

    imguGaussianFilter(&gaussianFilter,dev);
    imguCopy(&gaussianFilterSum,gaussianFilter);
    for(i=1;i<gaussianFilter->xs;i++)
    {
        gaussianFilterSum->complex[i][0]+=gaussianFilterSum->complex[i-1][0];
    }

    for (j=0;j<ysize;j++)
    {
        for (k=0;k<xsize;k++)
        {
            if ((csize==2 || csize==4) && src->data[(j*xsize+k)*csize+csize-1]==0) continue;
            for (c=0;c<cs_notrans;c++)
            {
                index=(j*xsize+k)*csize+c;
                rd=matRandNumber();
                l=0;
                while(l<gaussianFilter->xs)
                {
                    if (rd<gaussianFilterSum->complex[l][0]) break;
                    l++;
                }
                if (l==gaussianFilter->xs) l=gaussianFilter->xs-1;

                noise=l-(gaussianFilter->xs-1)/2.0;
                noise=noise/255.0*IMGU_MAXVAL;
                newval=src->data[index]+noise;
                if (newval<0) newval=0;
                if (newval>IMGU_MAXVAL) newval=IMGU_MAXVAL;
                (*dest)->data[index]=(pix_t)(newval);
            }
        }
    }

    imguFree(&gaussianFilter);
    imguFree(&gaussianFilterSum);

    return 0;
}

/**
 * Mirror an image horizontally
 *
 * @param[out] dest destination image pointer
 * @param[in] src source image (can be same as @p dest)
 * @return 0:success <0:error
 *
 */
int imguMirror(imgu **dest,imgu *src)
{
    int x,y,c,i1,i2;
    int xsize,ysize,csize;
    pix_t temp;
    if (dest==NULL || src==NULL || src->data==NULL) return -1;
    xsize=src->xs;
    ysize=src->ys;
    csize=src->cs;
    if ((*dest)!=src)
    {
      if (imguAllocate(dest,xsize,ysize,csize)) return -1;
    }

    for(y=0;y<ysize;y++)
    {
        for(x=0;x<xsize/2;x++)
        {
            for(c=0;c<csize;c++)
            {
              i1=INDEX(src,x,y,c);
              i2=INDEX(src,xsize-1-x,y,c);

              temp=src->data[i1];
              (*dest)->data[i1]=src->data[i2];
              (*dest)->data[i2]=temp;
            }
        }
    }

    return 0;
}

/**
 * Flip an image vertically
 *
 * @param[out] dest destination image pointer
 * @param[in] src source image (can be same as @p dest)
 * @return 0:success <0:error
 *
 */
int imguFlip(imgu **dest,imgu *src)
{
    int x,y,i;
    pix_t temp;
	pix_t *p,*q;
    if (dest==NULL || src==NULL || src->data==NULL) return -1;

	if( *dest == src ) {
		// same image. We flip "inplace"
		for(y=0;y<src->ys/2;y++) {
			p=src->data+INDEX(src,0,y,0);
			q=src->data+INDEX(src,0,src->ys-1-y,0);
			for(i=0;i<src->xs*src->cs;i++) { temp=*p;*p++=*q;*q++=temp; }
		}
	}else{
		if( imguAllocate(dest,src->xs,src->ys,src->cs) ) return -1;
		// different images. We flip while copying
		for(y=0;y<src->ys;y++) {
			p=src->data+INDEX(src,0,y,0);
			q=(*dest)->data+INDEX((*dest),0,src->ys-1-y,0);
			for(i=0;i<src->xs*src->cs;i++) *p++=*q++;
		}
	}
    return 0;
}

int imguTranspose(imgu **Idest,imgu *Isrc)
{
    int i,j,c;
    imgu *Icpy;
    unsigned char realloc;

    if( Idest==NULL || Isrc==NULL || Isrc->data==NULL) return(-1);

    Icpy=Isrc;
    realloc=0;

    if( *Idest!=Isrc ) { if(imguAllocate(Idest,Isrc->ys,Isrc->xs,Isrc->cs)) return(-1); }
    else
    {
        Icpy=NULL;
        imguCopy(&Icpy,Isrc);
        realloc=1;
        if(imguAllocate(Idest,Isrc->ys,Isrc->xs,Isrc->cs)) {imguFree(&Icpy);return(-1);}
    }

    for(i=0;i<Icpy->ys;i++)
    {
      for(j=0;j<Icpy->xs;j++)
      {
        for(c=0;c<Icpy->cs;c++)
        {
          PIXEL((*Idest),i,j,c)=PIXEL(Icpy,j,i,c);
        }
      }
    }

    if (realloc) imguFree(&Icpy);

    return 0;
}

int imguInvert(imgu **dest,imgu *src)
{
    int j;
    int xsize,ysize,csize,size;
    if (dest==NULL || src==NULL || src->data==NULL) return -1;
    xsize=src->xs;
    ysize=src->ys;
    csize=src->cs;
    if ((*dest)!=src)
    {
      if (imguAllocate(dest,xsize,ysize,ysize)) return -1;
    }

    size=xsize*ysize*csize;
    for(j=0;j<size;j++)
    {
        (*dest)->data[j]=(pix_t)(IMGU_MAXVAL-src->data[j]);
    }

    return 0;
}

int imguWindowGaussian(imgu **dest,imgu *src,double cx,double cy,double dx,double dy)
{
    double filter;
    int i,k,c;
    int xsize,ysize,csize;
    int index;

    if (dest==NULL || src==NULL || src->data==NULL) return -1;
    if ((*dest)!=src)
    {
      if (imguAllocate(dest,src->xs,src->ys,src->cs)) return -1;
    }

    xsize=src->xs;
    ysize=src->ys;
    csize=src->cs;

    index=0;
    for(k=0;k<ysize;k++)
    {
        for(i=0;i<xsize;i++)
        {
            filter=exp(-0.5*(i-cx)*(i-cx)/(dx*dx));
            filter*=exp(-0.5*(k-cy)*(k-cy)/(dy*dy));
            if (filter<0) filter=0;
            for(c=0;c<csize;c++)
            {
                index=k*xsize+i;
                (*dest)->data[index]=(pix_t)(src->data[index]*filter);
                index++;
            }
        }
    }

    return 0;
}

int imguPadd(imgu **Ipad,imgu *I,int paddleft,int paddright,int paddtop,int paddbottom)
{
    int i,j,k,l;
    int xsize,ysize,csize;
    int newxsize,newysize;
    imgu *Icpy;
    unsigned char realloc;

    if (Ipad==NULL) return -1;
    if (I==NULL || I->data==NULL) return -1;

    xsize=I->xs;
    ysize=I->ys;
    csize=I->cs;

    newxsize=xsize+paddleft+paddright;
    newysize=ysize+paddtop+paddbottom;

    Icpy=I;
    realloc=0;
    if ((*Ipad)!=I) imguAllocate(Ipad,newxsize,newysize,csize); 
    else if (newxsize*newysize*csize>I->ds) //Idest==Isrc, not enough space, make local copy, reallocate
    {
        Icpy=NULL;
        imguCopy(&Icpy,I);
        realloc=1;
        if( imguAllocate(Ipad,newxsize,newysize,csize) ) {imguFree(&Icpy);return(-1);}
    }

    for(j=ysize-1;j>=0;j--)
    {
        for(k=xsize-1;k>=0;k--)
        {
            for(l=0;l<csize;l++)
            {
                (*Ipad)->data[((j+paddtop)*newxsize+(k+paddleft))*csize+l]=Icpy->data[(j*Icpy->xs+k)*csize+l];
            }
        }
    }

    for(i=0;i<newysize;i++)
    {
      for(j=0;j<newxsize;j++)
      {
        if (j<paddleft || j>=newxsize-paddright || i<paddtop || i>=newysize-paddbottom)
        {
            for(l=0;l<csize;l++)
            {
                (*Ipad)->data[(i*newxsize+j)*csize+l]=0;
            }
        }
      }
    }

    (*Ipad)->xs=newxsize;
    (*Ipad)->ys=newysize;

    if (realloc) imguFree(&Icpy);

    return 0;
}

int imguWindowGaussianChannels(imgu **dest,imgu *src,double ct,double dt)
{
    double filter;
    int i,j,k;
    int xsize,ysize,csize;
    int index;
    if (dest==NULL || src==NULL || src->data==NULL) return -1;
    xsize=src->xs;
    ysize=src->ys;
    csize=src->cs;

    if ((*dest)!=src)
    {
      if (imguAllocate(dest,src->xs,src->ys,src->cs)) return -1;
    }

    index=0;
    for(j=0;j<csize;j++)
    {
        for(k=0;k<ysize;k++)
        {
            for(i=0;i<xsize;i++)
            {
                filter=1.0-fabs((j-ct))/((csize-1)/2.0);      
                if (filter<0) filter=0;
                (*dest)->data[index]=(pix_t)(src->data[index]*filter);
                index++;
            }
        }
    }

    return 0;
}

int imguPaddChannels(imgu **dest,imgu *src, int ts)
{
    int i,j,k,index;
    int xsize,ysize,csize;
    int newtsize;
    imgu *Icpy;
    unsigned char realloc;

    if (dest==NULL || src==NULL || src->data==NULL) return -1;
    xsize=src->xs;
    ysize=src->ys;
    csize=src->cs;

    newtsize=csize+2*ts;

    Icpy=src;
    realloc=0;
    if ((*dest)!=src)
    {
      if (imguAllocate(dest,xsize,ysize,newtsize)) return -1;
    }
    else if (xsize*ysize*newtsize>src->ds) //Idest==Isrc, not enough space, make local copy, reallocate
    {
        Icpy=NULL;
        imguCopy(&Icpy,src);
        realloc=1;
        if( imguAllocate(dest,xsize,ysize,newtsize) ) {imguFree(&Icpy);return(-1);}
    }

    index=0;
    for(j=0;j<ysize;j++)
    {
        for(k=0;k<xsize;k++)
        {
            for(i=csize-1;i>=0;i--)
            {
              (*dest)->data[index*newtsize+i+ts]=Icpy->data[index*csize+i];
            }
            index++;
        }
    }

    (*dest)->cs=newtsize;

    if (realloc) imguFree(&Icpy);
 
    return 0;
}

int imguMosaic(imgu **mosaic,imgu *seq,int nb_x_patches,int nb_y_patches,int nb_t_patches)
{
    int i,ii,iii,j,k,l,fx,fy;
    int xsize,ysize,csize,seqsize;
    imgu *currimg,*currcopy,*currI;

    if (mosaic==NULL) return -1;
    if (seq==NULL || seq->data==NULL) return -1;
    if ((*mosaic)==seq) return -1;

    seqsize=imguCount(seq);
    if (nb_x_patches*nb_y_patches*nb_t_patches!=seqsize) return -1;

    seqsize=nb_t_patches;

    if (seqsize<=0) return -1;

    xsize=seq->xs*nb_x_patches;
    ysize=seq->ys*nb_y_patches;
    csize=seq->cs;

    currimg=NULL;
    currcopy=NULL;
    imguAllocate(&currimg,xsize,ysize,csize);
    currI=seq;

    for(i=0;i<seqsize;i++)
    {
        for(ii=0;ii<nb_y_patches;ii++)
        {
            for(iii=0;iii<nb_x_patches;iii++)
            {
                if (currI!=NULL)
                {
                  for(j=0;j<currI->ys;j++)
                  {
                    for(k=0;k<currI->xs;k++)
                    {
                        for(l=0;l<csize;l++)
                        {
                            fy=ii*seq->ys;
                            fx=iii*seq->xs;
                            currimg->data[((fy+j)*currimg->ys+(fx+k))*csize+l]=currI->data[(j*currI->xs+k)*csize+l];
                        }
                    }
                  }
                  currI=currI->next;
                }
            }
        }
        imguCopy(&currcopy,currimg);
        imguAddLastMulti(mosaic,currcopy);
        currcopy=NULL;
    }

    imguFree(&currimg);

    return 0;
}


