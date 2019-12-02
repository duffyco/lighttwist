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
#include <math.h>

#include "imgu.h"
#include <cctype>

//#define VERBOSE

#include "sift.hpp"

#ifdef HAVE_ANN
  #include <ANN/ANN.h>

  using namespace std;

 typedef struct{
   ANNkd_tree* kdt;                                // search structure
   ANNpointArray kdt_data;                         // data points
   ANNidxArray kdt_nn;                             // near neighbor indices
   ANNdistArray kdt_dist;                          // near neighbor distances
   int kdt_nbmatches;
 }kdtree;
#endif

#define MINIMUM_DISTANCE	30000
#define NUMERATOR		8
#define DENOMINATOR		10

#define BUF_SIZE	4096

/*@cond SIFT */
typedef struct {
    double px,py;
    double sz,dir;
    double v[128]; // en pratique, c'est 128. il faudrait verifier...
} siftinfo;
/*@endcond*/

int imguSift(imgu *I)
{
    int i,k,imgsize;
    char key[30];
    char b[40];
    imgu *Igray;
    if (I==NULL) return -1;

    int    first          = -1 ;
    int    octaves        = -1 ;
    int    levels         = 3 ;
    float  threshold      = 0.04f / levels / 2.0f ;
    float  edgeThreshold  = 10.0f;
    float  magnif         = 1.5 ;
    int    nodescr        = 0 ;
    int    noorient       = 1 ; //output features at different angles
    //int    stableorder    = 0 ;
    //int    savegss        = 0 ;
    //int    verbose        = 0 ;
    //int    binary         = 0 ;
    //int    haveKeypoints  = 0 ;
    int    unnormalized   = 0 ;
    //int    fp             = 0 ;

    // ---------------------------------------------------------------
    //                                                  Load image
    // ---------------------------------------------------------------    

    Igray=NULL;

    imguRemoveAlphaLayer(&Igray,I);
    imguConvertToGray(&Igray,Igray);

    VL::PgmBuffer buffer;
    buffer.width=I->xs;
    buffer.height=I->ys;
    imgsize=buffer.width*buffer.height;
    buffer.data=new VL::pixel_t[imgsize];
    for (i=0;i<imgsize;i++) buffer.data[i]=Igray->data[i]/(double)(IMGU_MAXVAL);

    imguFree(&Igray);

    // ---------------------------------------------------------------
    //                                            Gaussian scale space
    // ---------------------------------------------------------------          
    int         O      = octaves ;    
    int const   S      = levels ;
    int const   omin   = first ;
    float const sigman = .5 ;
    float const sigma0 = 1.6 * powf(2.0f, 1.0f / S) ;

    // optionally autoselect the number number of octaves
    // we downsample up to 8x8 patches
    if(O < 1) {
        O = std::max(int(std::floor(log2(std::min(buffer.width,buffer.height))) - omin -3), 1) ;
    }

    // initialize scalespace
    VL::Sift sift(buffer.data, buffer.width, buffer.height, sigman, sigma0,O, S,omin, -1, S+1) ;      
    sift.detectKeypoints(threshold, edgeThreshold) ;

    // -------------------------------------------------------------
    //            Run detector, compute orientations and descriptors
    // -------------------------------------------------------------

    char *buf=(char *)malloc(BUF_SIZE);
    sprintf(buf," %d",128);
    imguReplaceAddText(I,"sift",buf);

    k=0;
    /* set descriptor options */
    sift.setNormalizeDescriptor( ! unnormalized ) ;
    sift.setMagnification( magnif ) ;

    for( VL::Sift::KeypointsConstIter iter = sift.keypointsBegin();iter != sift.keypointsEnd() ; ++iter ) {
        // detect orientations
        VL::float_t angles [4] ;
        int nangles ;
        if( ! noorient ) {
            nangles = sift.computeKeypointOrientations(angles, *iter) ;
        } else {
            nangles = 1;
            angles[0] = VL::float_t(0) ;
        }

        // compute descriptors
        for(int a = 0 ; a < nangles ; ++a) {
            /* compute descriptor */
            VL::float_t descr_pt [128] ;
            sift.computeKeypointDescriptor(descr_pt, *iter, angles[a]) ;

            /* save descriptor to to appropriate file */	      
            if( ! nodescr ) {
                sprintf(key,"sift_%08d",k);
                sprintf(buf," %f %f %f %f\n",iter->x,iter->y,iter->sigma,angles[a]);
                for (i=0;i<128;i++)
                {
                    sprintf(b," %f",descr_pt[i]);
                    strcat(buf,b);
                }
                imguReplaceAddText(I,key,buf);
                k++; //save all angles for each keypoint ??
            }
        } // next angle
    } // next keypoint

    sprintf(buf,"%d",k);
    imguPushText(I,"sift",buf);

    delete buffer.data;

    free(buf);
    return 0 ;
}

int imguSiftDelete(imgu *I)
{
    int i;
    char *A;
    int nb,sz;
    char buf[100];

    if (I==NULL) return -1;

    A=imguGetText(I,"sift");
    sscanf(A," %d %d",&nb,&sz);
    imguRemoveKey(I,"sift",1);

    for(i=0;i<nb;i++) {
        sprintf(buf,"sift_%08d",i);
        imguRemoveKey(I,buf,1);
    }

    return 0;
}


static double distSquared(double *a,double *b,int len)
{
    int i;
    double sum=0;
    for(i=0;i<len;i++) sum+=(a[i]-b[i])*(a[i]-b[i]);
    return(sum);
}

int siftinfo_copy(siftinfo *src,siftinfo *dest)
{
  int i;

  if (src==NULL || dest==NULL) return -1;

  dest->px=src->px;
  dest->py=src->py;
  dest->sz=src->sz;
  dest->dir=src->dir;
  
  for (i=0;i<128;i++)
  {
    dest->v[i]=src->v[i];
  }

  return 0;
}

static void siftinfo_shuffle(siftinfo *si, int size)
{
   int rnd_index, last_index;
   siftinfo temp;

   for (last_index = size; last_index > 1; last_index--)
   {
      rnd_index = (int)(matRandNumber()*last_index); //random number between 0 and last_index-1
      siftinfo_copy(&(si[rnd_index]),&temp);
      siftinfo_copy(&(si[last_index-1]),&(si[rnd_index]));
      siftinfo_copy(&temp,&(si[last_index-1]));
   }
}

// return the number of matches. Setup a pointer to the matches
int imguMatchFromSift(imgu *IA,imgu *IB,match **pm,int max_nb_matches)
{
    char *A,*B;
    int nba,nbb,sza,szb;
    siftinfo *sa,*sb;
    char buf[100];
    int i,j;
    //int val;
    double fval;
    char *q;
    match *m;
    int *bestIdx0; // les 2 plus proches match dans l'autre image (index)
    double *bestDist0; // les 2 plus proches match dans l'autre image (distance)
    int *bestIdx1; // les 2 plus proches match dans l'autre image (index)
    double *bestDist1; // les 2 plus proches match dans l'autre image (distance)
#ifdef HAVE_ANN
    kdtree ann_tree;
    ANNpoint ann_query;
#endif

    A=imguGetText(IA,"sift");
    B=imguGetText(IB,"sift");

    if( A==NULL ) return(-1);
    if( B==NULL ) return(-2);

    sscanf(A," %d %d",&nba,&sza);
    sscanf(B," %d %d",&nbb,&szb);

#ifdef VERBOSE
    printf("A: %d %d, B: %d %d\n",nba,sza,nbb,szb);
#endif
    printf("Number of features: (%d,%d)\n",nba,nbb);

    if( sza>128 || szb>128 ) { printf("too many values in the sift vectors\n");return(-3); }
    if( sza!=szb ) { printf("sift vector size mismatch\n");return(-4); }

    sa=(siftinfo *)malloc(nba*sizeof(siftinfo));
    sb=(siftinfo *)malloc(nbb*sizeof(siftinfo));

#ifdef HAVE_ANN
    printf("Matching using ANN.\n");
    ann_tree.kdt_nbmatches=2;
    //printf("Allocating data points (%dx%d)...",nbb,szb);
    ann_tree.kdt_data = annAllocPts(nbb, szb);                 // allocate data points
    ann_tree.kdt_nn = new ANNidx[ann_tree.kdt_nbmatches];      // allocate near neighbor indices
    ann_tree.kdt_dist = new ANNdist[ann_tree.kdt_nbmatches];   // allocate near neighbor dists
    ann_query = annAllocPt(sza);                               // allocate query point
#endif

    /// extract the sift vectors
    for(i=0;i<nba;i++) {
        sprintf(buf,"sift_%08d",i);

        A=imguGetText(IA,buf);
        if( A==NULL ) { printf("ou est le point %d (A)?\n",i);free(sa);free(sb);return(-5); }
        sscanf(A," %lf %lf %lf %lf",&sa[i].px,&sa[i].py,&sa[i].sz,&sa[i].dir);
#ifdef VERBOSE
        printf("A %d: %f %f %f %f\n",i,sa[i].px,sa[i].py,sa[i].sz,sa[i].dir);
#endif
        q=A;while( *q!=0 && *q!='\n' ) q++;q++; // juste apres le 1er '\n'
        for(j=0;j<sza;j++) {
            while( *q!=0 && isspace(*q) ) q++;
            sscanf(q,"%lf",&fval);sa[i].v[j]=fval;
            // printf("    A %3d : %3d : %12.6f\n",i,j,fval);
            while( *q!=0 && (isdigit(*q) || *q=='.' || *q=='-') ) q++;
        }
    }
    for(i=0;i<nbb;i++) {
        sprintf(buf,"sift_%08d",i);
        B=imguGetText(IB,buf);
        if( B==NULL ) { printf("ou est le point %d (B)?\n",i);free(sa);free(sb);return(-6); }
        sscanf(B," %lf %lf %lf %lf",&sb[i].px,&sb[i].py,&sb[i].sz,&sb[i].dir);
#ifdef VERBOSE
        printf("B %d: %f %f %f %f\n",i,sb[i].px,sb[i].py,sb[i].sz,sb[i].dir);
#endif
        q=B;while( *q!=0 && *q!='\n' ) q++;q++; // juste apres le 1er '\n'
        for(j=0;j<szb;j++) {
            while( *q!=0 && isspace(*q) ) q++;
            sscanf(q,"%lf",&fval);sb[i].v[j]=fval;
#ifdef HAVE_ANN
            ann_tree.kdt_data[i][j]=fval;
#endif
            //printf("B %3d : %3d : %12.6f\n",i,j,fval);
            while( *q!=0 && (isdigit(*q) || *q=='.' || *q=='-') ) q++;
        }
    }

#ifdef HAVE_ANN
  ann_tree.kdt = new ANNkd_tree(                   // build search structure
                    ann_tree.kdt_data,              // the data points
                    nbb,                    // number of points
                    szb);                // dimension of space
#endif

    //shuffle the features to ensure random distribution
    siftinfo_shuffle(sa, nba);
    // now match!!!
    // from the match algorithm in siftdemo

    bestIdx0=(int *)malloc(nba*sizeof(int));
    bestIdx1=(int *)malloc(nba*sizeof(int));
    bestDist0=(double *)malloc(nba*sizeof(double));
    bestDist1=(double *)malloc(nba*sizeof(double));

    /* This searches through the keypoints in klist for the two closest
     *    matches to key.  If the closest is less than 0.6 times distance to
     *       second closest, then return the closest match.  Otherwise, return
     *          NULL.
     *          */
    for(i=0;i<nba;i++) {
        bestDist0[i]=10000000.0;
        bestDist1[i]=10000000.0;
    }

#ifdef VERBOSE
    printf("init done\n");
#endif

    double d2=0;
    int nbmatch=0;
    for(i=0;i<nba;i++) {
#ifdef HAVE_ANN
        for (j=0;j<sza;j++)
        {
          ann_query[j]=sa[i].v[j];
        }

        ann_tree.kdt->annkSearch(                  // search
                ann_query,                         // query point
                ann_tree.kdt_nbmatches,            // number of near neighbors
                ann_tree.kdt_nn,                   // nearest neighbors (returned)
                ann_tree.kdt_dist,                 // distance (returned)
                0.0);                              // error bound    
        bestDist0[i]=ann_tree.kdt_dist[0];   //squared distances
        bestDist1[i]=ann_tree.kdt_dist[1];
        bestIdx0[i]=ann_tree.kdt_nn[0];
        bestIdx1[i]=ann_tree.kdt_nn[1];
#else
        for(j=0;j<nbb;j++) {
            d2=distSquared(sa[i].v,sb[j].v,sza);
            //printf("dist %3d:%3d = %12.6f\n",i,j,d2);
            // update distances
            if( d2<bestDist0[i] ) {
                bestIdx1[i]=bestIdx0[i];
                bestDist1[i]=bestDist0[i];
                bestIdx0[i]=j;
                bestDist0[i]=d2;
            } else if( d2<bestDist1[i] ) {
                bestIdx1[i]=j;
                bestDist1[i]=d2;
            }
        }
#endif
        if( DENOMINATOR*DENOMINATOR*bestDist0[i] < NUMERATOR*NUMERATOR*bestDist1[i] ) {
#ifdef VERBOSE
            printf("match A %d -> B %d\n",i,bestIdx0[i]);
#endif
            nbmatch++;
        }
        if (max_nb_matches>0 && nbmatch>=max_nb_matches) break;
    }

    /// reselect the matches, but store the info...
    m=(match *)malloc((nbmatch+1)*sizeof(match));
    nbmatch=0;
    for(i=0;i<nba;i++) {
        //    if( bestDist0[i]>=MINIMUM_DISTANCE ) continue;
        if( DENOMINATOR*DENOMINATOR*bestDist0[i] < NUMERATOR*NUMERATOR*bestDist1[i] ) {
            j=bestIdx0[i];
#ifdef VERBOSE
            printf("match A %d -> B %d\n",i,j);
#endif
            m[nbmatch].x1=sa[i].px;
            m[nbmatch].y1=sa[i].py;
            m[nbmatch].s1=sa[i].sz;
            m[nbmatch].d1=sa[i].dir;
            m[nbmatch].x2=sb[j].px;
            m[nbmatch].y2=sb[j].py;
            m[nbmatch].s2=sb[j].sz;
            m[nbmatch].d2=sb[j].dir;
            m[nbmatch].dist=bestDist0[i];
            nbmatch++;
        }
        if (max_nb_matches>0 && nbmatch>=max_nb_matches) break;
    }

    // cas special : <0 = fini!
    m[nbmatch].x1=-100000.0;
    m[nbmatch].y1=-100000.0;
    m[nbmatch].x2=-100000.0;
    m[nbmatch].y2=-100000.0;

#ifdef VERBOSE
    printf("nbmatch=%d\n",nbmatch);
#endif

#ifdef HAVE_ANN
    annDeallocPt(ann_query);
    if (ann_tree.kdt_data!=NULL) annDeallocPts(ann_tree.kdt_data);    
    delete [] ann_tree.kdt_nn;
    delete [] ann_tree.kdt_dist;
    delete ann_tree.kdt;
    annClose();
#endif


    free(sa);
    free(sb);
    free(bestIdx0); free(bestDist0);
    free(bestIdx1); free(bestDist1);

    if( pm!=NULL ) *pm=m; else free(m);

    return(nbmatch);
}

static void drawLine(imgu *I,double x1,double y1,double x2,double y2,unsigned short color)
{
    double dx,dy;
    int x,y;
    int nb,i;

    if (I==NULL || I->data==NULL) return;
   
#ifdef VERBOSE
    printf("LINE (%12.6f,%12.6f) (%12.6f,%12.6f)\n",x1,y1,x2,y2);
#endif

    dx=x2-x1;if( dx<0 ) dx=-dx;
    dy=y2-y1;if( dy<0 ) dy=-dy;
    if( dx<dy ) dx=dy;
    nb=dx*2;
    if( nb<2 ) nb=2; // longueur minimale d'une ligne
    // loop back to overwrite at 0
    for(i=nb-1;i>=0;i--) {
        x=(int)((x2-x1)*i/(nb-1.0)+x1+0.5);
        y=(int)((y2-y1)*i/(nb-1.0)+y1+0.5);
        if( x<0 || x>=I->xs || y<0 || y>=I->ys ) continue;
        I->data[(y*I->xs+x)*I->cs+0]=((i!=0)?color:(IMGU_MAXVAL-color));
    }
}

void imguDrawMatch(imgu *I,match *m,int nbmatch)
{
    int i;
    if (I==NULL || I->data==NULL || m==NULL) return;

    for(i=0;i<nbmatch;i++) {
        //drawLine(I,m[i].x1,I->ys-1-m[i].y1,m[i].x2,I->ys-1-m[i].y2,0);
        drawLine(I,m[i].x1,m[i].y1,m[i].x2,m[i].y2,0);
        printf("drawing line (%f,%f) + (%f,%f)\n",m[i].x1,m[i].y1,m[i].x2-m[i].x1,m[i].y2-m[i].y1);
    }
}

void imguDrawMatchPoints(imgu *IA,imgu *IB,match *m,int nbmatch)
{
    int i;
    if (m==NULL) return;

    for(i=0;i<nbmatch;i++) {
        /**
          if( IA ) drawLine(IA,m[i].x1-1,IA->ys-1-m[i].y1,m[i].x1+1,IA->ys-1-m[i].y1,i);
          if( IA ) drawLine(IA,m[i].x1,IA->ys-1-m[i].y1-1,m[i].x1,IA->ys-1-m[i].y1+1,i);
          if( IB ) drawLine(IB,m[i].x2-1,IB->ys-1-m[i].y2,m[i].x2+1,IB->ys-1-m[i].y2,i);
          if( IB ) drawLine(IB,m[i].x2,IB->ys-1-m[i].y2-1,m[i].x2,IB->ys-1-m[i].y2+1,i);
         **/
        drawLine(IA,m[i].x1-1,m[i].y1,m[i].x1+1,m[i].y1,i);
        drawLine(IA,m[i].x1,m[i].y1-1,m[i].x1,m[i].y1+1,i);
        drawLine(IB,m[i].x2-1,m[i].y2,m[i].x2+1,m[i].y2,i);
        drawLine(IB,m[i].x2,m[i].y2-1,m[i].x2,m[i].y2+1,i);
    }
}


// sift: <nb> <vector size>
// sift_%08d: <x> <y> <dx> <dy> <size elements>
void imguDrawSiftPoints(imgu *I)
{
    char *A;
    int nb,sz,i,v,c;
    double x,y,scale,dir;
    int xi,yi;
    char key[20];
    unsigned short color[4];

    if (I==NULL || I->data==NULL) return;

    if( (A=imguGetText(I,"sift"))==NULL ) return;

    sscanf(A," %d %d",&nb,&sz);
    //printf("nbsift=%d sz=%d\n",nb,sz);

    for(i=0;i<nb;i++) {
        sprintf(key,"sift_%08d",i);
        if( (A=imguGetText(I,key))==NULL ) continue; // pas normal!
        // on veut seulement x,y,size,dir
        sscanf(A," %lf %lf %lf %lf",&x,&y,&scale,&dir);
        //printf("  (%f,%f) scale=%f dir=%f\n",x,y,scale,dir);

        v=scale/3.0*IMGU_MAXVAL;if( v>IMGU_MAXVAL ) v=IMGU_MAXVAL;
        color[0]=v;
        v=dir/2.0/M_PI*IMGU_MAXVAL;if( v>IMGU_MAXVAL ) v=IMGU_MAXVAL;
        color[1]=v;
        color[2]=IMGU_MAXVAL;
        color[3]=IMGU_MAXVAL;
        xi=(int)(x+0.5);
        yi=(int)(y+0.5);
        if( xi<0 || xi>=I->xs || yi<0 || yi>=I->ys ) continue;
        for(c=0;c<I->cs;c++) I->data[(yi*I->xs+xi)*I->cs+c]=color[c];
    }

    return;
}








