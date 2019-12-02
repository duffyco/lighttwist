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
#include "graph.h"

int imguIntegralFcn(imgu **Idest,imgu *IA,imgu *IB,double (*fcn)(double a,double b),unsigned char component)
{
  int i,j,k;
  int xs,ys,cs;
  int index;
  double sumx,sumy,sumxy;

  if (component>1) return -1;

  if (IA==NULL || IA->data==NULL) return -1;
  if (IB!=NULL && IB->data==NULL) return -1;
  if (Idest==NULL) return -1;

  if (IB!=NULL)
  {
    if (IA->xs!=IB->xs) return -1;
    if (IA->ys!=IB->ys) return -1;
    if (IA->cs!=IB->cs) return -1;
  }

  xs=IA->xs;
  ys=IA->ys;
  cs=IA->cs;
 
  imguAllocateComplex(Idest,xs,ys,cs);

  for(k=0;k<cs;k++)
  {
    for(i=0;i<ys;i++)
    {
      for(j=0;j<xs;j++)
      {
        index=(i*xs+j)*cs+k;

        if (i==0 || j==0) sumxy=0;
        else sumxy=COMPLEX((*Idest),j-1,i-1,k,component);
        if (i==0) sumy=0;
        else sumy=COMPLEX((*Idest),j,i-1,k,component);
        if (j==0) sumx=0;
        else sumx=COMPLEX((*Idest),j-1,i,k,component);

        if (IB==NULL) (*Idest)->complex[index][component]=fcn((double)(IA->data[index]),(double)(IA->data[index]));
        else (*Idest)->complex[index][component]=fcn((double)(IA->data[index]),(double)(IB->data[index]));
        (*Idest)->complex[index][component]+=(sumx-sumxy+sumy);
      }
    }
  }

  return 0;
}

static double BoxFilter(imgu *I,int i,int j,int k,int L,unsigned char component)
{
  int xs,ys;
  double val;
  double sumx,sumy,sumxy;
 
  if (component!=0 && component!=1) return -1.0;
  if (I==NULL || I->complex==NULL) return -1.0;

  xs=I->xs;
  ys=I->ys;

  if (i-L<0 || j-L<0 || i+L>=ys || j+L>=xs)
  {
    val=-1.0;
  }
  else
  {
    if (i-L-1<0 || j-L-1<0) sumxy=0;
    else sumxy=COMPLEX(I,j-L-1,i-L-1,k,component);
    if (i-L-1<0) sumy=0;
    else sumy=COMPLEX(I,j+L,i-L-1,k,component);
    if (j-L-1<0) sumx=0;
    else sumx=COMPLEX(I,j-L-1,i+L,k,component);
    val=COMPLEX(I,j+L,i+L,k,component)+sumxy-sumx-sumy;
  }

  return val;
}

static double ab_a(double a,double b)
{
  return a;
}


static double ab_diff_abs(double a,double b)
{
  return fabs(a-b);
}

static double ab_diff_squared(double a,double b)
{
  return (a-b)*(a-b);
}

static double ab_mult(double a,double b)
{
  return a*b;
}

int imguSSDLocal(imgu **Idest,imgu *IA,imgu *IB,int wsize)
{
  int i,j,k,index;
  int xs,ys,cs;
  int L,N;
  imgu *Issd;

  if (IA==NULL || IA->data==NULL) return -1;
  if (IB==NULL || IB->data==NULL) return -1;
  if (Idest==NULL) return -1;

  if (IA->xs!=IB->xs) return -1;
  if (IA->ys!=IB->ys) return -1;
  if (IA->cs!=IB->cs) return -1;

  Issd=NULL;

  //imguIntegralFcn(&Issd,IA,IB,ab_diff_abs,0);
  imguIntegralFcn(&Issd,IA,IB,ab_diff_squared,0);

  if (wsize<0) return -1;
  if (wsize%2==0) wsize++;
  L=wsize/2;
  N=wsize*wsize;

  xs=IA->xs;
  ys=IA->ys;
  cs=IA->cs;
 
  imguAllocateComplex(Idest,xs,ys,cs);

  for(k=0;k<cs;k++)
  {
    for(i=0;i<ys;i++)
    {
      for(j=0;j<xs;j++)
      {
        index=(i*xs+j)*cs+k;
        (*Idest)->complex[index][0]=BoxFilter(Issd,i,j,k,L,0)/N;
        if ((*Idest)->complex[index][0]<0) (*Idest)->complex[index][0]=DEPTHMAP_COST_INF;
        //(*Idest)->complex[index][1]=0;
      }
    }
  }

  imguFree(&Issd);

  return 0;
}


int imguZeroMeanNormalizedCrossCorrelation(imgu **Idest,imgu *IA,imgu *IB,int wsize)
{
  int i,j,k,l,index;
  int xs,ys,cs;
  int L,N;
  imgu *IAsum,*IBsum,*Icov;
  double avg[2],dev[2],cov;

  if (IA==NULL || IA->data==NULL) return -1;
  if (IB==NULL || IB->data==NULL) return -1;
  if (Idest==NULL) return -1;

  if (IA->xs!=IB->xs) return -1;
  if (IA->ys!=IB->ys) return -1;
  if (IA->cs!=IB->cs) return -1;

  IAsum=NULL;
  IBsum=NULL;
  Icov=NULL;

  imguIntegralFcn(&IAsum,IA,NULL,ab_a,0);
  imguIntegralFcn(&IAsum,IA,NULL,ab_mult,1);
  imguIntegralFcn(&IBsum,IB,NULL,ab_a,0);
  imguIntegralFcn(&IBsum,IB,NULL,ab_mult,1);
  imguIntegralFcn(&Icov,IA,IB,ab_mult,0);

  if (wsize<0) return -1;
  if (wsize%2==0) wsize++;
  L=wsize/2;
  N=wsize*wsize;

  xs=IA->xs;
  ys=IA->ys;
  cs=IA->cs;
 
  imguAllocateComplex(Idest,xs,ys,cs);

  for(k=0;k<cs;k++)
  {
    for(i=0;i<ys;i++)
    {
      for(j=0;j<xs;j++)
      {
        index=(i*xs+j)*cs+k;
        avg[0]=BoxFilter(IAsum,i,j,k,L,0);
        dev[0]=BoxFilter(IAsum,i,j,k,L,1);
        avg[1]=BoxFilter(IBsum,i,j,k,L,0);
        dev[1]=BoxFilter(IBsum,i,j,k,L,1);
        for (l=0;l<2;l++)
        {
          if (avg[l]>=0)
          {
            //mean
            avg[l]/=N;
            //variance
            dev[l]=(dev[l]-N*avg[l]*avg[l])/(N-1);
            if (dev[l]>0) dev[l]=sqrt(dev[l]);
            else dev[l]=0;
          }
        }
        cov=(BoxFilter(Icov,i,j,k,L,0)-N*avg[0]*avg[1])/N;
        //(*Idest)->complex[index][0]=avg[0];
        //(*Idest)->complex[index][1]=dev[0];
        (*Idest)->complex[index][0]=cov/(dev[0]*dev[1]);
        //(*Idest)->complex[index][1]=0;
      }
    }
  }

  imguFree(&IAsum);
  imguFree(&IBsum);
  imguFree(&Icov);

  return 0;
}

static double veksler_cost(imgu *Ideriv,int x,int y)
{
    if (Ideriv==NULL || Ideriv->data==NULL) return 1.0;
    if (imguCheck(Ideriv,(float)(x),(float)(y))) return 1.0;

    if (PIXEL(Ideriv,x,y,0)<8.0) return 4.0;
    else return 1.0;    
}

int imguDepthMapDirectSearch(imgu **depthmap,imgu *Ileft,imgu *Iright,double *disps,int nbdisps)
{
  int i,j;
  int wsize;
  imgu *Ishifted;
  imgu *Jleft,*Jright;

  if (Ileft==NULL || Ileft->data==NULL) return -1;
  if (Iright==NULL || Iright->data==NULL) return -1;
  if ((*depthmap)==Ileft || (*depthmap)==Iright) return -1;

  if (nbdisps<1) return -1;

  Jleft=NULL;
  Jright=NULL;

  imguRemoveAlphaLayer(&Jleft,Ileft);
  imguConvertToLuminance(&Jleft,Jleft);
  imguEqualize(&Jleft,Jleft,NULL,IMGU_MAXVAL);
  //imguCannyBasic(&Jleft,Jleft,1.5);

  imguRemoveAlphaLayer(&Jright,Iright);
  imguConvertToLuminance(&Jright,Jright);
  imguEqualize(&Jright,Jright,NULL,IMGU_MAXVAL);
  //imguCannyBasic(&Jright,Jright,1.5);

  Ishifted=NULL;

  wsize=9;
  imguAllocate(depthmap,Jleft->xs,Jleft->ys,Jleft->cs);

  fprintf(stderr,"Computing costs");
  for (i=0;i<nbdisps;i++)
  {
    imguExtractRectangleSubPixel(&Ishifted,Jright,disps[i],0,Jright->xs,Jright->ys);
    //imguZeroMeanNormalizedCrossCorrelation(depthmap,Jleft,Ishifted,wsize);
    imguSSDLocal(depthmap,Jleft,Ishifted,wsize);
    fprintf(stderr,".");
    for (j=0;j<(*depthmap)->xs*(*depthmap)->ys;j++)
    {
      if (i==0 || (*depthmap)->complex[j][0]<(*depthmap)->complex[j][1])
      {
        (*depthmap)->complex[j][1]=(*depthmap)->complex[j][0];
        (*depthmap)->data[j]=i;
      }
    }
  }
  fprintf(stderr,"done\n");

  imguFree(&Ishifted);
  imguFree(&Jleft);
  imguFree(&Jright);

  return 0;
}

int imguDepthMapDynamicProgramming(imgu **depthmap,imgu *Ileft,imgu *Iright,double *disps,int nbdisps,double lambda,unsigned char xmodulo)
{
  int i,j,k,c;
  double cost;
  double xpos;
  imgu *Idsi;
  matrix *scosts;
  vector4 val;
  imgu *Jleft,*Jright;
  imgu *Jleft_edges;
  imgu *derivFilter;

  if (Ileft==NULL || Ileft->data==NULL) return -1;
  if (Ileft->cs>4 || Iright->cs>4) return -1;
  if (Iright==NULL || Iright->data==NULL) return -1;
  if ((*depthmap)==Ileft || (*depthmap)==Iright) return -1;

  if (nbdisps<1) return -1;

  Jleft=NULL;
  Jleft_edges=NULL;
  Jright=NULL;
  derivFilter=NULL;

  imguRemoveAlphaLayer(&Jleft,Ileft);
  //imguConvertToLuminance(&Jleft,Jleft);
  //imguEqualize(&Jleft,Jleft,IMGU_MAXVAL);
  //imguCannyBasic(&Jleft,Jleft,1.5);

  imguFirstDerivFilter2Pixels(&derivFilter);
  imguConvertToLuminance(&Jleft_edges,Jleft);
  imguConvertToComplexComponent(&Jleft_edges,Jleft_edges,0);
  imguConvolveComplex(&Jleft_edges,Jleft_edges,derivFilter,CONVOLVE_BLANK_MARGIN);
  imguMagnPhase(&Jleft_edges,Jleft_edges);
  imguConvertFromComplexComponent(&Jleft_edges,Jleft_edges,0,COMPLEX_AS_IS);

  imguRemoveAlphaLayer(&Jright,Iright);
  //imguConvertToLuminance(&Jright,Jright);
  //imguEqualize(&Jright,Jright,IMGU_MAXVAL);
  //imguCannyBasic(&Jright,Jright,1.5);

  scosts=NULL;
  scosts=NULL;
  matAllocate(&scosts,nbdisps,1);
  for (i=0;i<nbdisps;i++)
  {
    scosts->values[i]=(double)(i);
    if (scosts->values[i]>nbdisps/4.0) scosts->values[i]=nbdisps/4.0;
    scosts->values[i]*=lambda;
  }
  imguAllocate(depthmap,Jleft->xs,Jleft->ys,1);

  Idsi=NULL;
  imguAllocate(&Idsi,Jleft->xs,nbdisps,1);
  imguAllocateComplex(&Idsi,Jleft->xs,nbdisps,1);

  for (i=0;i<Jleft->ys;i++)
  {
    fprintf(stderr,"Disparity space image (%d).\n",i);
    for (j=0;j<Jright->xs;j++)
    {
      for (k=0;k<nbdisps;k++)
      {  
        xpos=j+disps[k];
        if (xmodulo)
        {
          xpos+=Jleft->xs;
          xpos=fmod(xpos,Jleft->xs);
        }
        if (imguInterpolateBilinear(Jright,(double)(xpos),(double)(i),val)) COMPLEX(Idsi,j,k,0,0)=DEPTHMAP_COST_INF;
        else
        {
          cost=0;
          for (c=0;c<Jleft->cs;c++)
          {
            cost+=ab_diff_abs((double)(PIXEL(Jleft,j,i,c)),val[c]);
          }
          //cost/=Jleft->cs;
          COMPLEX(Idsi,j,k,0,0)=cost;
        }
      }
    }
    imguDynamicProgramming(Idsi,-1,scosts->values,Jleft_edges,i,10.0);
    for (j=0;j<Jleft->xs;j++)
    {
      PIXEL((*depthmap),j,i,0)=Idsi->data[j];
    }
  }

  matFree(&scosts);
  imguFree(&Idsi);
  imguFree(&Jleft);
  imguFree(&Jleft_edges);
  imguFree(&Jright);
  imguFree(&derivFilter);

  return 0;
}

int imguDepthMapGraphCuts(imgu **depthmap,imgu *Ileft,imgu *Iright,double *disps,int nbdisps,double lambda,unsigned char xmodulo,int nb_iter)
{
  int i,j,k,c;
  int iter;
  int cost;
  imgu *Idsi;
  imgu *dsis;
  matrix *scosts;
  vector4 val;
  double xpos;
  imgu *Jleft,*Jright;
  imgu *Jderivs[2];
  imgu *derivFilter;
  imgu *dtmp;
  char filename[256];

  if (Ileft==NULL || Ileft->data==NULL) return -1;
  if (Iright==NULL || Iright->data==NULL) return -1;
  if (Ileft->cs>4 || Iright->cs>4) return -1;
  if (Iright==NULL || Iright->data==NULL) return -1;
  if ((*depthmap)==Ileft || (*depthmap)==Iright) return -1;

  if (nbdisps<1) return -1;

  dsis=NULL;
  Jleft=NULL;
  Jright=NULL;
  dtmp=NULL;
  derivFilter=NULL;

  imguRemoveAlphaLayer(&Jleft,Ileft);
  //imguCannyBasic(&Jleft,Jleft,1.5);
  //imguEqualize(&Jleft,Jleft,IMGU_MAXVAL);
  //imguConvertToLuminance(&Jleft,Jleft);

  imguRemoveAlphaLayer(&Jright,Iright);
  //imguCannyBasic(&Jright,Jright,1.5);
  //imguEqualize(&Jright,Jright,IMGU_MAXVAL);
  //imguConvertToLuminance(&Jright,Jright);

  //compute derivatives in left image
  imguFirstDerivFilter2Pixels(&derivFilter);
  Jderivs[0]=NULL;
  Jderivs[1]=NULL;
  imguConvertToLuminance(&Jderivs[0],Jleft);
  imguConvertToLuminance(&Jderivs[1],Jleft);

  for (i=0;i<2;i++)
  {
    imguConvertToComplexComponent(&Jderivs[i],Jderivs[i],0);
    imguConvolveComplex(&Jderivs[i],Jderivs[i],derivFilter,CONVOLVE_BLANK_MARGIN);
    //switch filter orientation to -y for next iteration
    derivFilter->ys=derivFilter->xs;
    derivFilter->xs=1;
    imguMagnPhase(&Jderivs[i],Jderivs[i]);
    imguConvertFromComplexComponent(&Jderivs[i],Jderivs[i],0,COMPLEX_AS_IS);
  }

  scosts=NULL;
  matAllocate(&scosts,nbdisps,1);
  for (i=0;i<nbdisps;i++)
  {
    scosts->values[i]=(double)(i);
    if (scosts->values[i]>nbdisps/4.0) scosts->values[i]=nbdisps/4.0;
    scosts->values[i]*=lambda;
  }
  imguAllocate(depthmap,Jleft->xs,Jleft->ys,1);
  imguClear(*depthmap);

  Idsi=NULL;

  for (i=0;i<Jleft->ys;i++)
  {
    imguAllocate(&Idsi,Jleft->xs,nbdisps,1);
    fprintf(stderr,"Disparity space image (%d).\n",i);

    for (j=0;j<Jleft->xs;j++)
    {
      for (k=0;k<nbdisps;k++)
      {  
        xpos=j+disps[k];
        if (xmodulo)
        {
          xpos+=Jleft->xs;
          xpos=fmod(xpos,Jleft->xs);
        }
        if (imguInterpolateBilinear(Jright,(double)(xpos),(double)(i),val)) PIXEL(Idsi,j,k,0)=IMGU_MAXVAL;
        else
        {
          cost=0;
          for (c=0;c<Jleft->cs;c++)
          {
            cost+=(int)(ab_diff_abs((double)(PIXEL(Jleft,j,i,c)),val[c]));
          }
          //cost/=Jleft->cs;
          if (cost>IMGU_MAXVAL) cost=IMGU_MAXVAL;
          PIXEL(Idsi,j,k,0)=(pix_t)(cost);
        }
      }
    }
    imguAddLastMulti(&dsis,Idsi);
    Idsi=NULL;
  }

  for (iter=0;iter<nb_iter;iter++)
  {
    if (iter==0) imguGraphCut(*depthmap,dsis,scosts->values,NULL,NULL,xmodulo,1);
    else imguGraphCut(*depthmap,dsis,scosts->values,Jderivs[0],Jderivs[1],xmodulo,1);
sprintf(filename,"dm_%d.png",iter);
imguMapRange(&dtmp,(*depthmap),0,IMGU_MAXVAL);
imguSave(dtmp,filename,1,SAVE_8_BITS_LOW);
  }

  matFree(&scosts);
  imguFreeMulti(&dsis);
  imguFree(&Jleft);
  imguFree(&Jright);
  imguFree(&dtmp);
  imguFree(&Jderivs[0]);
  imguFree(&Jderivs[1]);

  return 0;
}

//Isrc represents an DSI, where Idsi->xs is the size of the scanline and Idsi->ys is the number of disparities
//matching costs are assumed to be stored in Idsi->complex[0] as input
//cost of current best path is stored in Idsi->complex[1]
//index of previous element in best path is stored in Idsi->data
//scosts has to be of length Idsi->ys (number of disparities)
//E=<cost term>+lambda*<smoothing term>
int imguDynamicProgramming(imgu *Idsi,int drange,double *scosts,imgu *Idx,int y,double ocost)
{
  int i,j,k,index;
  int crange;
  double cost;
  double mcost;
  imgu *Idisp_first;
  int mcostdisp,mcostdispprev;

  if (Idsi->complex==NULL) return -1;
  if (Idsi->cs!=1) return -1;
  if (scosts==NULL) return -1;

  //for wrap around cost
  Idisp_first=NULL;
  imguAllocate(&Idisp_first,Idsi->xs,Idsi->ys,1);

  //initialisation
  for (i=0;i<Idsi->ys;i++)
  {
    //total cost for paths of length 1
    Idsi->complex[i*Idsi->xs][1]=Idsi->complex[i*Idsi->xs][0];
    Idisp_first->data[i*Idsi->xs]=i;
    for (j=0;j<Idsi->xs;j++)
    {
      Idsi->data[i*Idsi->xs+j]=Idsi->ys; //impossible value for previous best element
    }
  }

  for (j=1;j<Idsi->xs;j++)
  {
    for (i=0;i<Idsi->ys;i++) //current column
    {
      index=i*Idsi->xs+j;
      for (k=0;k<Idsi->ys;k++) //previous column
      {
        crange=i-k;
        //if (drange>=0 && abs(crange)>drange) continue;
        if ((drange>=0 && abs(crange)>drange) || crange<-1) continue;

        cost=Idsi->complex[k*Idsi->xs+j-1][1];
        cost+=Idsi->complex[index][0]+scosts[abs(crange)];
        if (j==Idsi->xs-1)
        {
//printf("%d %d %d %f\n",i,k,Idisp_first->data[k*Idsi->xs+j-1],scosts[abs(i-Idisp_first->data[k*Idsi->xs+j-1])]);
          cost+=scosts[abs(i-Idisp_first->data[k*Idsi->xs+j-1])];
        }
        //if (crange==-1) cost+=ocost+scosts[abs(crange)]*veksler_cost(Idx,j,y);
        //else if (crange==0) cost+=Idsi->complex[index][0];
        //else cost+=Idsi->complex[index][0]+scosts[abs(crange)]*veksler_cost(Idx,j,y)+crange*ocost;  
        if (Idsi->data[index]==Idsi->ys || cost<Idsi->complex[index][1])
        {
          Idsi->data[index]=k;
          Idisp_first->data[index]=Idisp_first->data[k*Idsi->xs+j-1];
          Idsi->complex[index][1]=cost;
        }
      }
    }
  }

//exit(0);

  //find minimum path
  mcost=Idsi->complex[Idsi->xs-1][1];
  mcostdisp=0;
  //find optimum path
  for (k=1;k<Idsi->ys;k++)
  {
    cost=Idsi->complex[k*Idsi->xs+Idsi->xs-1][1];
    if (cost<mcost)
    {
      mcost=cost;
      mcostdisp=k;
    }
  } 
  //follow path
  mcostdispprev=mcostdisp;
  for (j=Idsi->xs-1;j>0;j--)
  {
    mcostdisp=Idsi->data[mcostdisp*Idsi->xs+j];
    Idsi->data[j]=mcostdispprev;
    mcostdispprev=mcostdisp;
  }
  Idsi->data[j]=mcostdispprev;

  imguFree(&Idisp_first);
  
  return 0;
}

static inline int nodenum(int x, int y,int width)
{
  return ((y*width) + x + 2);
}

static double imgu_graphcut_scale;

static int ab_swap(imgu *dm,imgu *dcosts,double *scosts,imgu *Idx,imgu *Idy,int alpha,int beta,unsigned char xmodulo)
{
    int xs,ys;
    int x, y;
    imgu *dsi;
    int n_nodes;
    int disp,node;
    int ndisp; //disparity of neighbor
    float cost;
    int s_alpha,s_beta;

    xs=dcosts->xs;
    ys=imguCount(dcosts);

    n_nodes=xs*ys;
    typedef Graph<float,float,float> GraphType;
    GraphType *g = new GraphType(/*estimated # of nodes*/ n_nodes, /*estimated # of edges*/ n_nodes); 
    if (g==NULL)
    {
      //fprintf(stderr,"[imgu] Could not allocate graph-cut.\n");
      return -1;
    }
    g -> add_node(n_nodes); 

    n_nodes=0;
    dsi=dcosts;
    for (y = 0; y < ys; y++)
    {
        for (x = 0; x < xs; x++)
        {
	        disp = PIXEL(dm,x,y,0);
	        node = nodenum(x,y,xs);

	        if (disp==alpha || disp==beta)
	        {
	            n_nodes++;

                s_alpha=0;
                s_beta=0;
                //compute smoothing term with nodes not alpha or beta
                ndisp=PIXEL(dm,(x-1+xs)%xs,y,0);
                if (((xmodulo && x>=0) || ((!xmodulo) && x>0)) && ndisp!=alpha && ndisp!=beta)
                {
                  s_alpha+=(float)(scosts[abs(alpha-ndisp)]*veksler_cost(Idx,(x-1+xs)%xs,y));
                  s_beta+=(float)(scosts[abs(beta-ndisp)]*veksler_cost(Idx,(x-1+xs)%xs,y));
                }
                ndisp=PIXEL(dm,(x+1)%xs,y,0);
                if (((xmodulo && x <= xs-1) || ((!xmodulo) && x < xs-1)) && ndisp!=alpha && ndisp!=beta)
                {
                  s_alpha+=(float)(scosts[abs(alpha-ndisp)]*veksler_cost(Idx,x,y));
                  s_beta+=(float)(scosts[abs(beta-ndisp)]*veksler_cost(Idx,x,y));                                                                                                                                
                }
                ndisp=PIXEL(dm,x,(y-1+ys)%ys,0);
                if (y>0 && ndisp!=alpha && ndisp!=beta)
                {
                  s_alpha+=(float)(scosts[abs(alpha-ndisp)]*veksler_cost(Idy,x,y-1));
                  s_beta+=(float)(scosts[abs(beta-ndisp)]*veksler_cost(Idy,x,y-1));
                }
                ndisp=PIXEL(dm,x,(y+1)%ys,0);
                if (y<ys-1 && ndisp!=alpha && ndisp!=beta)
                {
                  s_alpha+=(float)(scosts[abs(alpha-ndisp)]*veksler_cost(Idy,x,y));
                  s_beta+=(float)(scosts[abs(beta-ndisp)]*veksler_cost(Idy,x,y));
                }
                g -> add_tweights( node, (float)((dsi->data[alpha*dsi->xs+x]+s_alpha)*imgu_graphcut_scale), 0 );
                g -> add_tweights( node, 0, (float)((dsi->data[beta*dsi->xs+x]+s_beta)*imgu_graphcut_scale));

                if (((xmodulo && x <= xs-1) || ((!xmodulo) && x < xs-1)) && (PIXEL(dm,(x+1)%xs,y,0)==alpha || PIXEL(dm,(x+1)%xs,y,0)==beta))
                {
                  cost=(float)(scosts[abs(alpha-beta)]*veksler_cost(Idx,x,y)*imgu_graphcut_scale);
                  g -> add_edge( node, nodenum((x+1)%xs,y,xs), cost, cost );
                }
                if (y < ys-1 && (PIXEL(dm,x,y+1,0)==alpha || PIXEL(dm,x,y+1,0)==beta))
                {
                  cost=(float)(scosts[abs(alpha-beta)]*veksler_cost(Idy,x,y)*imgu_graphcut_scale);
                  g -> add_edge( node, nodenum(x,y+1,xs), cost, cost );
                }
	        }
        }
        dsi=dsi->next;
	}
	if (n_nodes == 0)
    {
      delete g;
      return 0;
    }

    int flow = g -> maxflow();
	
    dsi=dcosts;
	for (y = 0; y < ys; y++)
	{
		for (x = 0; x < xs; x++)
		{
			disp = PIXEL(dm,x,y,0);
            node = nodenum(x,y,xs);
			if (disp == alpha || disp == beta)
			{
				if (g->what_segment(node) == GraphType::SOURCE)
                {
                  PIXEL(dm,x,y,0) = alpha; //link to source is cut
                }
				else
                {
                  PIXEL(dm,x,y,0) = beta; //link to sink is cut
                }
			}
		}
        dsi=dsi->next;
	}

    delete g;

    return flow;
}

static void shuffle(int *data, int size)
{
   int temp, rnd_index, last_index;

   for (last_index = size; last_index > 1; last_index--)
   {
      rnd_index = (int)(matRandNumber()*last_index); //random number between 0 and last_index-1
      temp = data[rnd_index];
      data[rnd_index] = data[last_index - 1];
      data[last_index - 1] = temp;
   }
}

static int sum_energy(imgu *dm,imgu *dcosts, double *scosts,imgu *Idx,imgu *Idy,double *Ed, double *Es, unsigned char xmodulo)
{
    int xs,ys;
    int x,y;
    int disp;
    imgu *dsi;

    xs=dcosts->xs;
    ys=imguCount(dcosts);

    (*Ed) = 0.0;
    (*Es) = 0.0;

    dsi=dcosts;
	// Compute the data term energy and smoothness energy 
    for (y=0;y<ys;y++)
    {
        for (x=0;x<xs;x++)
        {
	        disp=PIXEL(dm,x,y,0);
        	(*Ed) += (float)(dsi->data[disp*dsi->xs+x]);

			if (y < ys-1) (*Es) += (float)(scosts[abs(disp-PIXEL(dm,x,y+1,0))]*veksler_cost(Idy,x,y)); // vertical cost
            if ((xmodulo && x <= xs-1) || ((!xmodulo) && x < xs-1))
            {
              (*Es) += (float)(scosts[abs(disp-PIXEL(dm,(x+1)%xs,y,0))]*veksler_cost(Idx,x,y)); // horizontal cost
            }
		}
        dsi=dsi->next;
	}

    //this is a scale heuristic to avoid graphcut overflow
    imgu_graphcut_scale=(1 << 30)/((*Ed) + (*Es));
    if (imgu_graphcut_scale>1.0) imgu_graphcut_scale=1.0;

    return 0;
}

int imguGraphCut(imgu *depthmap,imgu *dcosts,double *scosts,imgu *Idx,imgu *Idy,unsigned char xmodulo,int nbiter)
{
  int i,l;
  int nblabels,nbswaps;
  unsigned char randomize_pairings;
  unsigned char randomize_labels;
  unsigned char success;
  double oldE,oldEd,oldEs;
  double newE,newEd,newEs;
  int label1,label2;
  int alpha,beta,product;
  int *labels;

  if (depthmap==NULL) return -1;
  if (depthmap->data==NULL) return -1;
  if (dcosts==NULL) return -1;
  if (dcosts->data==NULL) return -1;
  if (scosts==NULL) return -1;
  //if (scosts->complex==NULL) return -1;

  randomize_pairings=0;
  randomize_labels=0;

  nblabels=dcosts->ys;
  if (randomize_pairings) nbswaps=nblabels*nblabels;
  else nbswaps=nblabels;

  labels=(int *)(malloc(sizeof(int)*nbswaps));

  sum_energy(depthmap,dcosts,scosts,Idx,Idy,&oldEd,&oldEs,xmodulo);
  oldE = oldEd + oldEs;
  fprintf(stderr,"Energy = %.2f (%.2f + %.2f))\n",oldE,oldEd,oldEs);

  fprintf(stderr,"Optimizing using graph-cuts...\n");
  i=0;
  while(i<nbiter || nbiter<0)
  {
    for(l=0;l<nbswaps;l++) labels[l]=l;

    //Optionally randomize the labels
    if (randomize_labels) shuffle(labels,nbswaps);

    //Compute the data energy and smoothness energy
    sum_energy(depthmap,dcosts,scosts,Idx,Idy,&oldEd,&oldEs,xmodulo);
    oldE = oldEd + oldEs;
    success = 0;

    //One loop of graph cut algorithm
    for (label1=0;label1<nbswaps;label1++)
    {
        if (randomize_pairings) label2=nblabels-1;
        else label2=label1+1;
        for (;label2<nblabels;label2++)
        {
			alpha = labels[label1]; 
			beta  = labels[label2];

            if (randomize_pairings)
            {
                product = alpha; // encoded value is product of alpha*beta
                alpha = product % nblabels;
                beta  = product / nblabels;
                if (alpha <= beta) continue; // wasted loop index, but who cares?
            }

            ab_swap(depthmap,dcosts,scosts,Idx,Idy,alpha,beta,xmodulo);

			sum_energy(depthmap,dcosts,scosts,Idx,Idy,&newEd,&newEs,xmodulo);
			newE = newEd + newEs;

			if (newE < oldE)
            {
              //fprintf(stdout, "[Graph-cuts] Energy = %.2f (%.2f + %.2f)\n", newE, newEd, newEs);
              success = 1;
            }
            else if (newE > oldE+1e-8)
            {
              fprintf(stderr,"[Graph-cuts] Warning: increase of energy detected (scale=%f).\n",imgu_graphcut_scale);
			  fprintf(stdout, "graph cut: alpha=%d, beta=%d\n", alpha, beta);
              fprintf(stdout, "old E = %.2f (%.2f + %.2f)\n", oldE, oldEd, oldEs);
              fprintf(stdout, "new  E = %.2f (%.2f + %.2f)\n", newE, newEd, newEs);
              //return -1;
              //exit(-1);
            }
			oldEd = newEd;
			oldEs = newEs;
			oldE  = newE;
	    }
        fprintf(stderr,"Iteration %d: %d%% (Energy = %.2f (%.2f + %.2f))\r",i+1,(label1*100)/(nbswaps-1),newE,newEd,newEs);
    }
    fprintf(stderr,"\n");
    if (!success) break;

    i++;
  }
  fprintf(stderr,"done\n");

  free(labels);

  return 0;
}



