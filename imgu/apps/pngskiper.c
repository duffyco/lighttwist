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


// PNG is network byte order (most significant first) (little-endian)
// donc HL avec n=H*256+L
// retourne 1 si on est low-hi, et 0 si on est hi-low
static int is_big_endian()
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



char sig[8]={0x89,0x50,0x4E,0x47,0x0D,0x0A,0x1A,0x0A};


int check_sig_is_png(FILE *F)
{
char buf[8];
int i,k;
	k=fread(buf,1,8,F);
	if( k!= 8 ) return(0);
	for(i=0;i<8;i++) if( buf[i]!=sig[i] ) return(0);
	return(1);
}


void swap4(unsigned int *a)
{
unsigned char *b=(unsigned char *)a;
unsigned char z;
	// 0,1,2,3 -> 3,2,1,0
	z=b[0];b[0]=b[3];b[3]=z;
	z=b[1];b[1]=b[2];b[2]=z;
}


int main(int argc,char *argv[])
{
FILE *F;
char InName[100];
int i,k;
int bigendian;
int len;
char head[5];
int pos;

	bigendian=is_big_endian();

	InName[0]=0;

	for(i=1;i<argc;i++) {
		if( strcmp("-i",argv[i])==0 && i+1<argc ) {
			strcpy(InName,argv[i+1]);
			i++;continue;
		}
	}

	if( InName[0] ) {
		F=fopen(InName,"r");
		if( F==NULL ) { fprintf(stderr,"yo!\n");exit(0); }
	}else F=stdin;

	printf("is big endian : %d\n",bigendian);
	head[4]=0;

   for(;;) {
	printf("Nouveau PNG!\n");

	k=check_sig_is_png(F);
	printf("k=%d\n",k);

	if( k==0 ) break;


	// read chunks
	for(i=0;i<1000000;i++) {
		fread(&len,1,4,F);
		if( bigendian ) swap4(&len);
		//printf("Len is %d\n",len);

		fread(head,1,4,F);
		//printf("Head is %s\n",head);

		//skip the content + CRC
		fseek(F,len+4,SEEK_CUR);

		if( strcmp("IEND",head)==0 ) break;
	}

	pos=ftell(F);
	printf("position=%d\n",pos);
  }


	if( InName[0] ) fclose(F);
}

