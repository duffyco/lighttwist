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

int camAllocate(camera **cam,imgu *img)
{
    if ((*cam)==NULL)
    {
      (*cam)=(camera *)(malloc(sizeof(camera)));
      (*cam)->features=NULL;
      (*cam)->mfeatures=NULL;
      (*cam)->mcam=NULL;
      (*cam)->next=NULL;
    }

    (*cam)->img=img;

    mat4Identity((*cam)->M);
    mat4Identity((*cam)->Minv);
    mat4Identity((*cam)->R);
    mat4Identity((*cam)->T);
    mat3Identity((*cam)->K);
    mat3Identity((*cam)->Kinv);
    mat3Identity((*cam)->F);
    mat3Identity((*cam)->E);
    mat3Identity((*cam)->H);

    (*cam)->position[0]=0.0;
    (*cam)->position[1]=0.0;
    (*cam)->position[2]=0.0;
    (*cam)->orientation[0]=0.0;
    (*cam)->orientation[1]=0.0;
    (*cam)->orientation[2]=0.0;

    (*cam)->k1=(*cam)->k2=0;

    return 0;
}

int camAllocateMulti(camera **cam,imgu *ims)
{
  if (ims==NULL)
  {
    camFreeMulti(cam); //in case camAllocateMulti was called before with a longer ims list, we need to free the remaining structures
    return 0;
  }

  camAllocate(cam,ims);
  return  camAllocateMulti(&((*cam)->next),ims->next);
}

void camFree(camera **cam)
{
    if (cam!=NULL && (*cam)!=NULL)
    {
      matFree(&((*cam)->features));
      matFree(&((*cam)->mfeatures));
      free((*cam));
      (*cam)=NULL;
    }
}

void camFreeMulti(camera **cam)
{
    if (cam==NULL) return;

    camera *ctemp;

    while( *cam!=NULL ) {
  	  // free *cam, but save the next link before its too late...
	  ctemp=(*cam)->next;
	  camFree(cam);
	  *cam=ctemp;
    }
}

int camLoadParams(camera *cam)
{  
    matrix *mat;
    char *text;
 
    if (cam==NULL) return -1;
    if (cam->img==NULL) return -1;
 
    mat=NULL;
    matAllocate(&mat,3,3);
    if (imguLoadMatrix(&mat,cam->img,"CAM_INTERN")==0)
    {
      vectxCopy(mat->values,cam->K,9);
      mat3Inverse(cam->K,cam->Kinv);
    }
    if (imguLoadMatrix(&mat,cam->img,"CAM_FONDAMENTAL")==0)
    {
      vectxCopy(mat->values,cam->F,9);
    }
    if (imguLoadMatrix(&mat,cam->img,"CAM_ESSENTIAL")==0)
    {
      vectxCopy(mat->values,cam->E,9);
    }
    if (imguLoadMatrix(&mat,cam->img,"CAM_HOMOGRAPHY")==0)
    {
      vectxCopy(mat->values,cam->H,9);
    }
    text=imguGetText(cam->img,"CAM_ORIENTATION");
    if (text!=NULL) sscanf(text,"%lg %lg %lg",&(cam->orientation[0]),&(cam->orientation[1]),&(cam->orientation[2]));
    text=imguGetText(cam->img,"CAM_POSITION");
    if (text!=NULL) sscanf(text,"%lg %lg %lg",&(cam->position[0]),&(cam->position[1]),&(cam->position[2]));

    camSetExternalParams(cam,cam->position,cam->orientation);

    text=imguGetText(cam->img,"CAM_K1");
    if (text!=NULL) sscanf(text,"%lg",&(cam->k1));
    text=imguGetText(cam->img,"CAM_K2");
    if (text!=NULL) sscanf(text,"%lg",&(cam->k2));

    matFree(&mat);

    return 0;
}

int camSaveParams(camera *cam)
{
    matrix *mat;
    char buf[255];

    if (cam==NULL) return -1;
    if (cam->img==NULL) return -1;

    mat=NULL;
    matAllocate(&mat,3,3);
    vectxCopy(cam->K,mat->values,9);
    imguSaveMatrix(mat,cam->img,"CAM_INTERN");
    vectxCopy(cam->F,mat->values,9);
    imguSaveMatrix(mat,cam->img,"CAM_FONDAMENTAL");
    vectxCopy(cam->E,mat->values,9);
    imguSaveMatrix(mat,cam->img,"CAM_ESSENTIAL");
    vectxCopy(cam->H,mat->values,9);
    imguSaveMatrix(mat,cam->img,"CAM_HOMOGRAPHY");
    sprintf(buf,"%f %f %f",cam->orientation[0],cam->orientation[1],cam->orientation[2]);
    imguReplaceAddText(cam->img,"CAM_ORIENTATION",buf);
    sprintf(buf,"%f %f %f",cam->position[0],cam->position[1],cam->position[2]);
    imguReplaceAddText(cam->img,"CAM_POSITION",buf);

    sprintf(buf,"%f",cam->k1);
    imguReplaceAddText(cam->img,"CAM_K1",buf);
    sprintf(buf,"%f",cam->k2);
    imguReplaceAddText(cam->img,"CAM_K2",buf);

    matFree(&mat);

    return 0;
}

int camCopyParams(camera *dest,camera *src)
{
    if (dest==NULL) return -1;
    if (src==NULL) return -1;

    vectxCopy(src->M,dest->M,16);
    vectxCopy(src->K,dest->K,9);
    vectxCopy(src->Minv,dest->Minv,16);
    vectxCopy(src->Kinv,dest->Kinv,9);
    vectxCopy(src->F,dest->F,9);
    vectxCopy(src->E,dest->E,9);
    vectxCopy(src->H,dest->H,9);
    vectxCopy(src->orientation,dest->orientation,3);
    vectxCopy(src->position,dest->position,3);
    vectxCopy(src->T,dest->T,16);
    vectxCopy(src->R,dest->R,16);

    dest->k1=src->k1;
    dest->k2=src->k2;

    return 0;
}

void camDump(camera *cam)
{
    if (cam==NULL) return;
 
    printf("###################\n");
    printf("M:\n");
    mat4Print(cam->M);
    //mat4Print(cam->Minv);
    printf("K:\n");
    mat3Print(cam->K);
    //mat3Print(cam->Kinv);
    printf("Fundamental:\n");
    mat3Print(cam->F);
    printf("Essential:\n");
    mat3Print(cam->E);
    //printf("Homography:\n");
    //mat3Print(cam->H);
    printf("Position:\n");
    vect3Print(cam->position);
    printf("Orientation:\n");
    vect3Print(cam->orientation);
    //mat4Print(cam->T);
    //mat4Print(cam->R);
    printf("Radial distortion:\n");
    printf("%f %f\n",cam->k1,cam->k2);
    printf("###################\n");
}

int camCount(camera *cam)
{
    int count;
    camera *curr;
    curr=cam;
    count=0;
    while(curr!=NULL)
    {
        curr=curr->next;
        count++;
    }

    return count;
}

/* Assumes position Tx,Ty,Tz of camera and its rotation Rx,Ry,Rz quaternion format are known*/
int camSetExternalParams(camera *cam,vector3 t,vector3 w)
{
    matrix4 T,Tinv,R,RT;
    if (cam==NULL) return -1;

    vectxCopy(w,cam->orientation,3);
    vectxCopy(t,cam->position,3);
	mat4Translation(t,T);
    mat4Inverse(T,Tinv);

	mat4RotationFromAxis(w,R);
    mat4Transpose(R,RT);
	mat4Multiply(RT,Tinv,cam->M);

    //P=TR
    mat4Copy(RT,cam->R);
    mat4Identity(cam->T);
    cam->T[3]=cam->M[3];
    cam->T[7]=cam->M[7];
    cam->T[11]=cam->M[11];

	if( mat4Inverse(cam->M,cam->Minv) ) { printf("Impossible d'inverser.\n");return(-1); }

    return 0;
}

int camSetExternalParamsFromMatrix(camera *cam,matrix4 Mext)
{
  matrix4 Tinv,R,RT;
  vector3 t,w;

  if (cam==NULL) return -1;

  mat4Copy(Mext,RT);
  RT[3]=0;
  RT[7]=0;
  RT[11]=0;
  mat4Transpose(RT,R);
  mat4Multiply(R,Mext,Tinv);

  mat4RotationToAxis(R,w);

  t[0]=-Tinv[3];
  t[1]=-Tinv[7];
  t[2]=-Tinv[11];

  camSetExternalParams(cam,t,w);

  return 0;
}

int camSetInternalParams(camera *cam,double fx,double fy,double skew,double ox,double oy)
{
    if (cam==NULL) return -1;

    cam->K[0]=fx;
    cam->K[1]=skew;
    cam->K[2]=ox;
    cam->K[3]=0.0;
    cam->K[4]=fy;
    cam->K[5]=oy;
    cam->K[6]=0.0;
    cam->K[7]=0.0;
    cam->K[8]=1.0;

	if( mat3Inverse(cam->K,cam->Kinv) ) { printf("Impossible d'inverser.\n");return(-1); }

    return 0;
}

int camSetInternalParamsFromViewAngle(camera *cam,double hangle)
{
    double angle,temp;
    double fx;

    if (cam==NULL) return -1;
    if (cam->img==NULL) return -1;

    angle=hangle/2.0;
    temp=tan(angle*M_PI/180.0);

    if (fabs(temp)<1e-8) return -1;

    fx=cam->img->xs/(2.0*temp);
    camSetInternalParams(cam,-fx,-fx,0.0,cam->img->xs/2.0,cam->img->ys/2.0);

    return 0;
}

static int camUndistort(camera *cam,vector3 pcam)
{
    double radius2;

    if (cam==NULL) return -1;

    radius2=pcam[0]*pcam[0]+pcam[1]*pcam[1];
    pcam[0]*=(1+radius2*cam->k1+radius2*radius2*cam->k2);
    pcam[1]*=(1+radius2*cam->k1+radius2*radius2*cam->k2);

    return 0;
}

int camImageToCamera (camera *cam,vector3 pim,vector3 pcam)
{
    if (cam==NULL) return -1;

    //camOriginAtCenter(cam,ptemp);
	mat3MultiplyVector(cam->Kinv,pim,pcam);
    camUndistort(cam,pcam);

	return 0;
}

int camCameraToImage (camera *cam,vector3 pcam,vector3 pim)
{
    if (cam==NULL) return -1;

	mat3MultiplyVector(cam->K,pcam,pim);

	return 0;
}

int camCameraToWorld (camera *cam,double x,double y,double z,double depth,vector4 pworld)
{
    vector4 pcam;
    if (cam==NULL) return -1;
    if (fabs(depth)<1e-8) return -1;

	pcam[0]=x;
	pcam[1]=y;
	pcam[2]=z;
    pcam[3]=1/depth;
	vect4Homogenize3D(pcam);
	mat4MultiplyVector(cam->Minv,pcam,pworld);

	return 0;
}

int camWorldToCamera (camera *cam,vector4 pworld,vector4 pcam)
{
    if (cam==NULL) return -1;

	mat4MultiplyVector(cam->M,pworld,pcam);
	vect4Homogenize3D(pcam);
	vect3Homogenize2D(pcam);

	return 0;
}

int camInFront(camera *cam,vector4 pworld)
{
    vector4 pcam;

    if (cam==NULL) return -1;

	mat4MultiplyVector(cam->M,pworld,pcam);
	vect4Homogenize3D(pcam);

    if (pcam[2]>0) return 1;
    else return 0;
}

int camImageToWorld (camera *cam,double x,double y,double depth,vector4 pworld)
{
    vector4 pcam;
    vector3 pim;
    if (cam==NULL) return -1;
    if (fabs(depth)<1e-8) return -1;

	pim[0]=x;
	pim[1]=y;
	pim[2]=1.0;

    //camOriginAtCenter(cam,pim);
	mat3MultiplyVector(cam->Kinv,pim,pcam);
    camUndistort(cam,pcam);
    pcam[3]=1/depth;
	vect4Homogenize3D(pcam);
	mat4MultiplyVector(cam->Minv,pcam,pworld);

	return 0;
}

int camWorldToImage (camera *cam,vector4 pworld,vector3 pim)
{
    vector4 pcam;
    if (cam==NULL) return -1;

	mat4MultiplyVector(cam->M,pworld,pcam);
	vect4Homogenize2D(pcam);
	mat3MultiplyVector(cam->K,pcam,pim);
    //camOriginAtUpperLeft(cam,pim);

    return 0;
}

int camImageUndistort(camera *cam,vector3 pixel)
{
    vector3 pcam;

    if (cam==NULL) return -1;

    camImageToCamera(cam,pixel,pcam);
    camCameraToImage(cam,pcam,pixel);

    return 0;
}

int camOriginAtCenter(camera *cam,vector3 pixel)
{
  if (cam==NULL || cam->img==NULL) return -1;

  pixel[0]-=cam->img->xs/2.0;
  pixel[1]-=cam->img->ys/2.0;

  return 0;
}

int camOriginAtUpperLeft(camera *cam,vector3 pixel)
{
  if (cam==NULL || cam->img==NULL) return -1;

  pixel[0]+=cam->img->xs/2.0;
  pixel[1]+=cam->img->ys/2.0;

  return 0;
}

//points have to be non-coplanar
//pts2d are in pixels, but assumes origin is center of image
int camCalibrateFrom7Points(camera *cam,matrix *pts2d,matrix *pts3d)
{
    int i,j;
    matrix *A,*b,*res;
    matrix *pts2d_cntr;
    matrix4 T,R,Mext;
    vector3 t;
    matrix3 wrows;
    double magn,sign_check;
    //double alpha;
    double gamma;
    int N;

    if (pts2d==NULL || pts3d==NULL) return -1;
    if (pts2d->cs!=pts3d->cs) return -1;
    if (pts2d->cs<7) return -1;
    if (pts2d->rs!=2) return -1;
    if (pts3d->rs!=3) return -1;
    if (cam==NULL) return -1;

    A=NULL;
    b=NULL;
    res=NULL;
    pts2d_cntr=NULL;

    N=pts2d->cs;

    matCopy(&pts2d_cntr,pts2d);
    for (i=0;i<N;i++)
    {
      camOriginAtCenter(cam,&(pts2d_cntr->values[2*i]));
    }

    matAllocate(&A,N,8);

//matPrint(pts2d);
//matPrint(pts3d);

    //model is:
    //pimg=K.T.R.pworld
    //Kinv.pimg=T.R.pworld
    //see Trucco & Verri (p.128)
    //assume (ox,oy)=(0,0)

    for (i=0;i<N;i++)
    {
        matSet(A,i,0,matGet(pts2d_cntr,i,0)*matGet(pts3d,i,0));
        matSet(A,i,1,matGet(pts2d_cntr,i,0)*matGet(pts3d,i,1));
        matSet(A,i,2,matGet(pts2d_cntr,i,0)*matGet(pts3d,i,2));
        matSet(A,i,3,matGet(pts2d_cntr,i,0));
        matSet(A,i,4,-matGet(pts2d_cntr,i,1)*matGet(pts3d,i,0));
        matSet(A,i,5,-matGet(pts2d_cntr,i,1)*matGet(pts3d,i,1));
        matSet(A,i,6,-matGet(pts2d_cntr,i,1)*matGet(pts3d,i,2));
        matSet(A,i,7,-matGet(pts2d_cntr,i,1));
    }

//matPrint(A);

    matSolveA(&res,A);
  
    magn=res->values[0]*res->values[0];
    magn+=res->values[1]*res->values[1];
    magn+=res->values[2]*res->values[2];
    if (magn<0.0) return -1;
    gamma=sqrt(magn);
    if (gamma<1e-8) return -1;

    magn=res->values[4]*res->values[4];
    magn+=res->values[5]*res->values[5];
    magn+=res->values[6]*res->values[6];
    if (magn<0.0) return -1;
    //alpha=sqrt(magn)/gamma;

    //check sign of scale factor
    sign_check=matGet(pts2d_cntr,0,0)*(res->values[4]*matGet(pts3d,0,0)+res->values[5]*matGet(pts3d,0,1)+res->values[6]*matGet(pts3d,0,2)+res->values[7]);
//fprintf(stderr,"Sign check: %f\n",sign_check);
    if (sign_check>0)
    {
//fprintf(stderr,"Sign change!\n");
      gamma=-gamma;
    }
//alpha should be 1
//fprintf(stderr,"Alpha: %f\n",alpha);
//fprintf(stderr,"Gamma: %f\n",gamma);
    for (j=0;j<8;j++) res->values[j]/=gamma;

    wrows[0]=res->values[4];
    wrows[1]=res->values[5];
    wrows[2]=res->values[6];
    wrows[3]=res->values[0];
    wrows[4]=res->values[1];
    wrows[5]=res->values[2];
    vect3Cross(&(wrows[0]),&(wrows[3]),&(wrows[6]));
    mat3ConvertToMat4(wrows,R);
    mat4NormalizeRotation(R);

    t[0]=res->values[7];
    t[1]=res->values[3];  
  
    //we still need to estimate f,Tz
    matAllocate(&A,N,2);
    matAllocate(&b,N,1);

    for (i=0;i<N;i++)
    {
        matSet(A,i,0,matGet(pts2d_cntr,i,0));
        matSet(A,i,1,R[0]*matGet(pts3d,i,0)+R[1]*matGet(pts3d,i,1)+R[2]*matGet(pts3d,i,2)+t[0]);
        matSet(b,i,0,-matGet(pts2d_cntr,i,0)*(R[8]*matGet(pts3d,i,0)+R[9]*matGet(pts3d,i,1)+R[10]*matGet(pts3d,i,2)));
    }

    matSolveAb(&res,A,b);
    t[2]=res->values[0];
   
    if (cam->img!=NULL) camSetInternalParams(cam,-res->values[1],-res->values[1],0.0,cam->img->xs/2.0,cam->img->ys/2.0);
    else camSetInternalParams(cam,-res->values[1],-res->values[1],0.0,0.0,0.0);

    mat4Translation(t,T);
    mat4Multiply(T,R,Mext);
    camSetExternalParamsFromMatrix(cam,Mext);

//mat4Print(cam->T);
//mat4Print(cam->R);
//mat4Print(Mext);

    matFree(&A);
    matFree(&b);
    matFree(&res);
    matFree(&pts2d_cntr);

    return 0;
}

int camCalibrateFrom6Points(camera *cam,matrix *pts2d,matrix *pts3d)
{
    int i;
    matrix *A,*res;
    matrix *pts2d_cntr;
    vector3 q1,q2,q3;
    double q1Tq1,q2Tq2;
    double gamma,magn;
    vector3 t;
    matrix4 T,R,Mext;
    double ox,oy,fx,fy;
    int sign;
    int N;

    if (pts2d==NULL || pts3d==NULL) return -1;
    if (pts2d->cs!=pts3d->cs) return -1;
    if (pts2d->cs<6) return -1;
    if (pts2d->rs!=2) return -1;
    if (pts3d->rs!=3) return -1;
    if (cam==NULL) return -1;

    A=NULL;
    res=NULL;
    pts2d_cntr=NULL;

    N=pts2d->cs;

    matCopy(&pts2d_cntr,pts2d);
    for (i=0;i<N;i++)
    {
      camOriginAtCenter(cam,&(pts2d_cntr->values[2*i]));
    }

    matAllocate(&A,2*N,12);

//matPrint(pts2d);
//matPrint(pts3d);

    for (i=0;i<2*N;i++)
    {
        matSet(A,2*i,0,matGet(pts3d,i,0));
        matSet(A,2*i,1,matGet(pts3d,i,1));
        matSet(A,2*i,2,matGet(pts3d,i,2));
        matSet(A,2*i,3,1.0);
        matSet(A,2*i,8,-matGet(pts2d_cntr,i,0)*matGet(pts3d,i,0));
        matSet(A,2*i,9,-matGet(pts2d_cntr,i,0)*matGet(pts3d,i,1));
        matSet(A,2*i,10,-matGet(pts2d_cntr,i,0)*matGet(pts3d,i,2));
        matSet(A,2*i,11,-matGet(pts2d_cntr,i,0));

        matSet(A,2*i+1,4,matGet(pts3d,i,0));
        matSet(A,2*i+1,5,matGet(pts3d,i,1));
        matSet(A,2*i+1,6,matGet(pts3d,i,2));
        matSet(A,2*i+1,7,1.0);
        matSet(A,2*i+1,8,-matGet(pts2d_cntr,i,1)*matGet(pts3d,i,0));
        matSet(A,2*i+1,9,-matGet(pts2d_cntr,i,1)*matGet(pts3d,i,1));
        matSet(A,2*i+1,10,-matGet(pts2d_cntr,i,1)*matGet(pts3d,i,2));
        matSet(A,2*i+1,11,-matGet(pts2d_cntr,i,1));
    }

//matPrint(A);

    matSolveA(&res,A);

    magn=res->values[8]*res->values[8];
    magn+=res->values[9]*res->values[9];
    magn+=res->values[10]*res->values[10];
    if (magn<0.0) return -1;
    gamma=sqrt(magn);
    if (gamma<1e-8) return -1;

    for (i=0;i<12;i++)
    {
      res->values[i]/=gamma;
    }

    q1[0]=res->values[0];
    q1[1]=res->values[1];
    q1[2]=res->values[2];
    q2[0]=res->values[4];
    q2[1]=res->values[5];
    q2[2]=res->values[6];
    q3[0]=res->values[8];
    q3[1]=res->values[9];
    q3[2]=res->values[10];

    mat4Identity(R);

    sign=1;
    t[2]=res->values[11];
    if (t[2]<0)
    {
      t[2]=-t[2];
      sign=-1;
    }
   
    for (i=0;i<3;i++)
    {
      R[8+i]=sign*res->values[8+i];
    }

    ox=vectxDot(q1,q3,3);
    oy=vectxDot(q2,q3,3);
    ox=0;
    oy=0;

    q1Tq1=vectxDot(q1,q1,3);
    q2Tq2=vectxDot(q2,q2,3);
    fx=q1Tq1-ox*ox;
    if (fx<0) return -1;
    fx=sqrt(fx);
    fy=q2Tq2-oy*oy;
    if (fy<0) return -1;
    fy=sqrt(fy);

    fy=fx;

    for (i=0;i<3;i++)
    {
      R[i]=sign*(ox*res->values[8+i]-res->values[i])/fx;
    }
    for (i=0;i<3;i++)
    {
      R[4+i]=sign*(oy*res->values[8+i]-res->values[4+i])/fy;
    }
    mat4NormalizeRotation(R);

    t[0]=sign*(ox*t[2]-res->values[3])/fx;
    t[1]=sign*(oy*t[2]-res->values[7])/fy;
    
    if (cam->img!=NULL) camSetInternalParams(cam,-fx,-fy,0.0,cam->img->xs/2.0,cam->img->ys/2.0);
    else camSetInternalParams(cam,-fx,-fy,0.0,0.0,0.0);

    mat4Translation(t,T);
    mat4Multiply(T,R,Mext);
//mat4Print(Mext);
    camSetExternalParamsFromMatrix(cam,Mext);
  
    matFree(&A);
    matFree(&res);
    matFree(&pts2d_cntr);

    return 0;
}

static int intersect_lines(vector3 p1,vector3 p2,vector3 q1,vector3 q2,vector4 intersection)
{
  vector3 v1,v2,v3;
  vector3 c1,c2;
  vector3 ac2,ac2c1,mac2c1;
  vector3 av1;
  double a,denom;

  vectxSubtract(p2,p1,v1,3);
  vectxSubtract(q2,q1,v2,3);
  vectxSubtract(q1,p1,v3,3);
  vect3Cross(v3,v2,c1);
  vect3Cross(v1,v2,c2);
  denom=vectxNorm(c2,3);
  if (fabs(denom)<1e-8) return -1;
  a=vectxNorm(c1,3)/denom;
  vectxScale(c2,a,ac2,3);
  vectxSubtract(ac2,c1,ac2c1,3);
  vectxScale(c2,-a,ac2,3);
  vectxSubtract(ac2,c1,mac2c1,3);
  if (vectxNorm(ac2c1,3)>vectxNorm(mac2c1,3)) a=-a;
  vectxScale(v1,a,av1,3);
  vectxAdd(p1,av1,intersection,3);
  intersection[3]=1.0;

  return 0;
}


static int camTriangulate(camera *cam1,camera *cam2,matrix **pts_3d,unsigned char space)
{
    int i,j,k,N;
    int ret1,ret2;
    vector4 p1,p2,pcam1,pcam2;
    vector4 pworld1,pworld2;

    if (cam1==NULL || cam2==NULL) return -1;
    if (pts_3d==NULL) return -1;
    if (cam1->mfeatures==NULL || cam2->mfeatures==NULL) return -1;
    if (cam1->mfeatures->cs!=cam2->mfeatures->cs) return -1;
    if (space==IMAGE_SPACE)
    {
      if (cam1->mfeatures->rs<2) return -1;
      if (cam2->mfeatures->rs<2) return -1;
    }
    else
    {
      if (cam1->mfeatures->rs<3) return -1;
      if (cam2->mfeatures->rs<3) return -1;
    }

    N=cam1->mfeatures->cs;
    matAllocate(pts_3d,N,4);

    for (j=0;j<N;j++)
    {
      if (space==IMAGE_SPACE)
      {
        camImageToWorld(cam1,matGet(cam1->mfeatures,j,0),matGet(cam1->mfeatures,j,1),1.0,p1);
        camImageToWorld(cam2,matGet(cam2->mfeatures,j,0),matGet(cam2->mfeatures,j,1),1.0,p2);
      }
      else
      {
        camCameraToWorld(cam1,matGet(cam1->mfeatures,j,0),matGet(cam1->mfeatures,j,1),matGet(cam1->mfeatures,j,2),1.0,p1);
        camCameraToWorld(cam2,matGet(cam2->mfeatures,j,0),matGet(cam2->mfeatures,j,1),matGet(cam2->mfeatures,j,2),1.0,p2);
      }
      ret1=intersect_lines(cam1->position,p1,cam2->position,p2,pworld1);
      ret2=intersect_lines(cam2->position,p2,cam1->position,p1,pworld2);
      mat4MultiplyVector(cam1->M,pworld1,pcam1);
      mat4MultiplyVector(cam2->M,pworld2,pcam2);
      if (ret1!=0 || ret2!=0 || pcam1[2]<0 || pcam2[2]<0)
      //if (ret1!=0 || pcam1[2]<0)
      {
//printf("triangulation error: %d\n",j);
        //put point at infinity w.r.t. cam1 and cam2 and take the average
        if (space==IMAGE_SPACE)
        {
          camImageToWorld(cam1,matGet(cam1->mfeatures,j,0),matGet(cam1->mfeatures,j,1),1000000.0,pworld1);
          camImageToWorld(cam2,matGet(cam2->mfeatures,j,0),matGet(cam2->mfeatures,j,1),1000000.0,pworld2);                
        }
        else
        {
          camCameraToWorld(cam1,matGet(cam1->mfeatures,j,0),matGet(cam1->mfeatures,j,1),matGet(cam1->mfeatures,j,2),1000000.0,pworld1);
          camCameraToWorld(cam2,matGet(cam2->mfeatures,j,0),matGet(cam2->mfeatures,j,1),matGet(cam2->mfeatures,j,2),1000000.0,pworld2);                
        }
      }
      for (k=0;k<4;k++)
      {
        //matSet((*pts_3d),j,k,pworld1[k]);
        matSet((*pts_3d),j,k,(pworld1[k]+pworld2[k])/2.0);
      }
    }

    return 0;
}

int camTriangulatePixels(camera *cam1,camera *cam2,matrix **pts_3d)
{
  return camTriangulate(cam1,cam2,pts_3d,IMAGE_SPACE);
}

int camTriangulatePoints(camera *cam1,camera *cam2,matrix **pts_3d)
{
  return camTriangulate(cam1,cam2,pts_3d,CAMERA_SPACE);
}

int camTriangulatePixelTracks(camera *cam1,camera *cam2,matrix **wpts,matrix **werrors,matrix **perrors)
{
    int i,j,k,N,index;
    int count,nbcams;
    matrix *ptmp;
    camera *currcam1;
    camera *currcam2;
    vector4 pcam;
    vector3 pimg;
    vector3 pt;

    if (cam1==NULL || cam2==NULL) return -1;
    if (wpts==NULL || werrors==NULL) return -1;
    if (cam1->features==NULL) return -1;
    if (cam2->features==NULL) return -1;
    if (cam1->features->cs!=cam2->features->cs) return -1;
    if (cam1->features->rs!=cam2->features->rs) return -1;

    N=cam1->features->cs;
    matAllocate(wpts,N,4);
    matAllocate(werrors,N,4);
    matAllocate(perrors,N,3);

    ptmp=NULL;

    currcam1=cam1;
    currcam2=cam2;
    for (i=0;i<cam1->features->rs;i+=2)
    {
      matSelectColumns(&(currcam1->mfeatures),cam1->features,i,2);
      matSelectColumns(&(currcam2->mfeatures),cam2->features,i,2);
      camTriangulatePixels(currcam1,currcam2,&ptmp);
      for (j=0;j<currcam1->mfeatures->cs;j++)
      {
        if (matGet(currcam1->mfeatures,j,0)>=0 && matGet(currcam2->mfeatures,j,0)>=0)
        {
          for (k=0;k<4;k++)
          {
            index=j*(*wpts)->rs+k;
            (*wpts)->values[index]+=ptmp->values[index];
            if (k<3) (*werrors)->values[index]+=ptmp->values[index]*ptmp->values[index];
            else (*werrors)->values[index]+=ptmp->values[index];
          }
        }
      }
      currcam1=currcam1->next;
      currcam2=currcam2->next;
      if (currcam1==NULL || currcam2==NULL) break;
    }

    for (i=0;i<(*wpts)->cs;i++)
    {
      vect4Homogenize3D(&((*wpts)->values[i*(*wpts)->rs]));
      N=(*werrors)->values[i*(*werrors)->rs+(*werrors)->rs-1];
      if (N<1)
      {
        for (k=0;k<3;k++)
        {
          (*werrors)->values[i*(*werrors)->rs+k]=0;
        }
      }
      else
      {
        for (k=0;k<3;k++)
        {
          index=i*(*werrors)->rs+k;
          (*werrors)->values[index]=((*werrors)->values[index]-N*(*wpts)->values[index]*(*wpts)->values[index])/(N-1);
          if ((*werrors)->values[index]<0) (*werrors)->values[index]=0;
          else (*werrors)->values[index]=sqrt((*werrors)->values[index]);
        }
      }
      (*werrors)->values[i*(*werrors)->rs+k]=1.0;
    }

    currcam1=cam1;
    currcam2=cam2;
    for (i=0;i<cam1->features->rs;i+=2)
    {
      for (j=0;j<currcam1->mfeatures->cs;j++)
      {
        if (matGet(currcam1->mfeatures,j,0)>=0 && matGet(currcam2->mfeatures,j,0)>=0)
        {
          camWorldToImage(currcam1,&((*wpts)->values[j*(*wpts)->rs]),pimg);
          pt[0]=matGet(currcam1->mfeatures,j,0);
          pt[1]=matGet(currcam1->mfeatures,j,1);
          pt[2]=1.0;
          camImageToCamera(currcam1,pt,pcam);
          camCameraToImage(currcam1,pcam,pt);
          for (k=0;k<2;k++)
          {
            index=j*(*perrors)->rs+k;
            (*perrors)->values[index]+=fabs(pimg[k]-pt[k]);
          }
          camWorldToImage(currcam2,&((*wpts)->values[j*(*wpts)->rs]),pimg);
          pt[0]=matGet(currcam2->mfeatures,j,0);
          pt[1]=matGet(currcam2->mfeatures,j,1);
          pt[2]=1.0;
          camImageToCamera(currcam2,pt,pcam);
          camCameraToImage(currcam2,pcam,pt);
          for (k=0;k<2;k++)
          {
            index=j*(*perrors)->rs+k;
            (*perrors)->values[index]+=fabs(pimg[k]-pt[k]);
          }
          (*perrors)->values[j*(*perrors)->rs+k]+=2.0;
        }
      }
      currcam1=currcam1->next;
      currcam2=currcam2->next;
      if (currcam1==NULL || currcam2==NULL) break;
    }

    for (i=0;i<(*perrors)->cs;i++)
    {
      vect3Homogenize2D(&((*perrors)->values[i*(*perrors)->rs]));
    }

    matFree(&ptmp);

    return 0;
}

int camSelectPose(camera *cam1,camera *cam2,matrix4 *P)
{
  int i,j,maxi;
  int count[4];
  double error[4];
  double dx,dy;
  vector4 cpoint1,cpoint2;
  vector4 ipoint1,ipoint2;
  matrix4 I,Mext1cpy,Mext2cpy;
  matrix *wpoints;

  if (cam1==NULL || cam2==NULL) return -1;
  if (cam1==cam2) return -1;

  wpoints=NULL;

  mat4Identity(I);
  mat4Copy(cam1->M,Mext1cpy); //backup external params, they will be copied back at the end of this function
  mat4Copy(cam2->M,Mext2cpy);

  for (i=0;i<4;i++) 
  {
//mat4Print(P[i]);
    count[i]=0;
    camSetExternalParamsFromMatrix(cam1,I);
    camSetExternalParamsFromMatrix(cam2,P[i]);
    camTriangulate(cam1,cam2,&wpoints,IMAGE_SPACE);
    if (wpoints!=NULL)
    {
      for (j=0;j<wpoints->cs;j++)
      {
        mat4MultiplyVector(cam1->M,&(wpoints->values[j*wpoints->rs]),cpoint1);
        vect4Homogenize3D(cpoint1);
        mat4MultiplyVector(cam2->M,&(wpoints->values[j*wpoints->rs]),cpoint2);
        vect4Homogenize3D(cpoint2);
        if (cpoint1[2]>0 && cpoint2[2]>0)
        {
          count[i]++;
        }
        camWorldToImage(cam1,&(wpoints->values[j*wpoints->rs]),ipoint1);
        camWorldToImage(cam2,&(wpoints->values[j*wpoints->rs]),ipoint2);
        dx=ipoint1[0]-ipoint2[0];
        dy=ipoint1[1]-ipoint2[1];
        error[i]+=sqrt(dx*dx+dy*dy);
      }
    }
  }

  maxi=0;
  if (count[1]>count[maxi] || (count[1]==count[maxi] && error[1]<error[maxi])) maxi=1;
  if (count[2]>count[maxi] || (count[2]==count[maxi] && error[2]<error[maxi])) maxi=2;
  if (count[3]>count[maxi] || (count[3]==count[maxi] && error[3]<error[maxi])) maxi=3;
//printf("%d %d %d %d\n",count[0],count[1],count[2],count[3]);
//printf("%f %f %f %f\n",error[0],error[1],error[2],error[3]);

  camSetExternalParamsFromMatrix(cam1,Mext1cpy);
  camSetExternalParamsFromMatrix(cam2,Mext2cpy);

  matFree(&wpoints);

  return maxi;
}

///// DEBUG //////

#ifdef SKIP

int main(int argc,char *argv[])
{
double r1[16],r2[16],n[3];
double s1[16],is1[16];
double cam[16],icam[16];
double camIn[16],camEx[16];
double p[10],qa[4],pa[4],pb[4];
double L;
	printf("allo\n");

	n[0]=1.0;
	n[1]=0.0;
	n[2]=0.0;
	rotationMatrix(M_PI/4.0,n,r1);

	n[0]=1.0;
	n[1]=1.0;
	n[2]=1.0;
	rotationMatrix(-M_PI/4.0,n,r2);

	n[0]=2.0;
	n[1]=3.0;
	n[2]=4.0;
	scaleMatrix(n,r2);

	multMM(r1,r2,s1);

	dump4x4(r1);
	dump4x4(r2);
	dump4x4(s1);

	inverseMaffine(s1,is1);

	dump4x4(is1);

	inverseMaffine(is1,s1);
	dump4x4(s1);
	internalMatrix(256.0,128.0,64.0,64.0,s1);
	dump4x4(s1);


	// interne
	p[0]=512.0;
	p[1]=512.0;
	p[2]=512.0;
	p[3]=512.0;

	// translation
	p[4]=1.0;
	p[5]=2.0;
	p[6]=3.0;

	// rotation
	p[7]=1.0;
	p[8]=1.0;
	p[9]=1.0; normalize(p+7,3); // make p[7,8,9] into an axis
	L=10*1.5*M_PI/180.0; // rotation magnitude
	p[7]*=L;p[8]*=L;p[9]*=L;

	cameraMatrix(p,cam,camIn,camEx);
	dump4x4(cam);
	inverseMaffine(cam,icam);
	dump4x4(icam);

	qa[0]=10.0;
	qa[1]=15.0;
	qa[2]=20.0;
	qa[3]=1.0;
	multMV(cam,qa,pa);
	dump4x1(qa);printf(" -> ");dump4x1(pa);

	homogene2d(pa);
	printf(" -H-> ");dump4x1(pa);
	printf("\n");

	multMV(icam,pa,pb);
	printf("in 3d -> ");dump4x1(pb);
	homogene3d(pb);
	printf(" -H-> ");dump4x1(pb);
	printf("\n");

	pa[3]/=2;
	multMV(icam,pa,pb);
	printf("in 3d -> ");dump4x1(pb);
	homogene3d(pb);
	printf(" -H-> ");dump4x1(pb);
	printf("\n");

	pa[0]=1.0;
	pa[1]=0.0;
	pa[2]=0.0;
	pa[3]=0.0;
	multMV(camEx,pa,pb);
	printf("NORMALE: ");dump4x1(pa);printf(" --> ");dump4x1(pb);printf("\n");

}

#endif


