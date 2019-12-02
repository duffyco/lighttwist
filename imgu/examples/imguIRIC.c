/*
 * IRIC test
 * 
 */


#include <imgu.h>


#define NAME	"/home/roys/Images/IRIC/for_seb/Droso_V2_IRIC_20090204_1_F3_P%d_%02d_V1_%s_0512.png"
//
// P:1..5
// PP:01..07
// B:CY3,CY5,FTC,TXR
//
char *base[4]={"CY3","CY5","FTC","TXR"};

int main( int argc, char **argv )
{
int k;
imgu *I[10];
int p,pp,b,n;
char buf[100];
imgu *IN,*IR;
int xs,ys;
double L;
int x,y;

	p=1;
	pp=1;
	n=0;
	for(b=0;b<4;b++) {
		sprintf(buf,NAME,p,pp,base[b]);
		printf("opening %s\n",buf);
		I[n]=NULL;
		k=imguLoad(I+n,buf,LOAD_16_BITS);
		if( k ) { printf("yo.\n");exit(-1); }
		printf("pixel 796,1084 = %d\n",PIXEL(I[n],796,1084,0));
		n++;
	}

	xs=I[0]->xs;
	ys=I[0]->ys;

	IN=NULL;
	imguAllocate(&IN,xs,ys,1);
	IR=NULL;
	imguAllocate(&IR,xs,ys,3);


	//
	// compute the norm...
	//
	// compute the direction vector
	// normalize (x0,x1,x2,x3) then put (x0,x1,x2) ina rgb image, and then the norm as alpha.
	//
	double max=0.0;
	double vf[4];
	int v;
	double f;
	for(y=0;y<ys;y++)
	for(x=0;x<xs;x++) {
		for(b=0;b<4;b++) vf[b]=(double)PIXEL(I[b],x,y,0);
		L=0.0;
		for(b=0;b<4;b++) L+=vf[b]*vf[b];
		L=sqrt(L); // max value should be 65535
		if( L>max ) max=L;
	}
	for(y=0;y<ys;y++)
	for(x=0;x<xs;x++) {
		for(b=0;b<4;b++) vf[b]=(double)PIXEL(I[b],x,y,0);
		L=0.0;
		for(b=0;b<4;b++) L+=vf[b]*vf[b];
		L=sqrt(L); // max value should be 65535
		//printf("-> L=%12.6f\n",L);
		// MAXVAL est la longueur 1, ce qui est ce qu'on veut. Donc, on penalise en fonction de |1-L|
		v=IMGU_MAXVAL-L; // 0 = best! 1=worst. or -1
		if( v<0 ) v=-v;  // 0=best, 1=worst
		//PIXEL(IN,x,y,0)=(int)(L/max*IMGU_MAXVAL+0.5);
		PIXEL(IN,x,y,0)=(int)(IMGU_MAXVAL-v+0.5);

		for(b=0;b<4;b++) vf[b]/=L;
/**
		PIXEL(IR,x,y,0)=(int)((1-vf[0])*IMGU_MAXVAL+0.5);
		PIXEL(IR,x,y,1)=(int)((1-vf[1])*IMGU_MAXVAL+0.5);
		PIXEL(IR,x,y,2)=(int)((1-vf[2])*IMGU_MAXVAL+0.5);
		//PIXEL(IR,x,y,3)=(int)(L/max*IMGU_MAXVAL+0.5);
		PIXEL(IR,x,y,3)=(int)(IMGU_MAXVAL-v+0.5);
**/
		f=1.0-(double)v/IMGU_MAXVAL;
		PIXEL(IR,x,y,0)=(int)((1-vf[0])*IMGU_MAXVAL*f+0.5);
		PIXEL(IR,x,y,1)=(int)((1-vf[1])*IMGU_MAXVAL*f+0.5);
		PIXEL(IR,x,y,2)=(int)((1-vf[2])*IMGU_MAXVAL*f+0.5);
	}


	imguSave(IN,"out_norm.png",1,SAVE_AS_IS);
	imguSave(IR,"out_vec.png",1,SAVE_AS_IS);
	
    return 0;
}

