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

#include <stdlib.h>
#include <stdio.h>
#include <imgu.h>

void test1()
{
imgu *I;

    I=NULL;
	printf("-- test 1 : init --\n");
	imguAllocateText(&I);
	imguDump(I);
}

void test2(char *name)
{
int k;
imgu *IA,*I;
    IA=NULL;
    I=NULL;
	printf("-- test 2 : load %s --\n",name);
	if (imguLoadMulti(&IA,name)) { printf("err %d\n",k);exit(0); }
	for(I=IA;I!=NULL;I=I->next) imguDump(I);
	imguFreeMulti(&IA);
}


void test3(char *inname,char *outname,int compress)
{
int k;
imgu *IA;
    IA=NULL;
	printf("-- test 3 : load %s , save %s --\n",inname,outname);
	if (imguLoadMulti(&IA,inname)) { printf("err load %d\n",k);exit(0); }

	imguAddText(IA,"Comment","cinq");
	imguAddText(IA->next,"Comment","six");

	imguDumpMulti(IA);
	k=imguSaveMulti(IA,outname,compress);
	if( k ) { printf("err save %d\n",k);exit(0); }
	imguFreeMulti(&IA);
}


void test4()
{
int k;
imgu *IA;
int i;
FILE *F;
char buf[100];
char obuf[100];
    IA=NULL;
	F=fopen("png_list","r");
	if( F==NULL ) { exit(0); }

	while( fscanf(F," %s",buf)==1 ) {
		//if( strncmp(buf,"png/basi0g08.png",10)!=0 ) continue;
		printf("test file %s\n",buf);
		if (imguLoad(&IA,buf)) { printf("err load %d\n",k);continue; }
		imguDump(IA);

		sprintf(obuf,"o%s",buf);
		k=imguSave(IA,obuf,1);
		if( k ) { printf("err save %d\n",k);continue; }
		imguFree(&IA);
	}

	//imguDump(IA);

//	for(i=0;i<IA->xs*IA->ys*IA->cs;i++) IA->data[i]/=32;

//	k=imguSave(IA,"blub_1.png",1);
//	if( k ) { printf("err save %d\n",k);exit(0); }
	//imguFree(IA);

	fclose(F);
}

void test5(char *inname,char *outname,int compress)
{
int k;
imgu *IA;
    IA=NULL;
	printf("-- test 5 : load %s , save %s --\n",inname,outname);
	if (imguLoadMulti(&IA,inname)) { printf("err load %d\n",k);exit(0); }

	imguDumpMulti(IA);
	k=imguSaveMulti(IA,outname,compress);
	if( k ) { printf("err save %d\n",k);exit(0); }
	imguFreeMulti(&IA);
}

// load multi, save single
void test6(char *inname,char *outname,int compress)
{
imgu *I,*J;
char buf[100];
int i,k;
    I=NULL;
    J=NULL;
	printf("-- test 6 : load multi (in ram), save many single compressed --\n");
	if (imguLoadMulti(&I,inname)) { printf("err loading\n");exit(0); }
	for(i=0,J=I;J!=NULL;J=J->next,i++) {
		sprintf(buf,outname,i);
		k=imguSave(J,buf,compress);
		if( k ) { printf("err saving %s\n",buf);break; }
	}
	imguFreeMulti(&I);
}


// load multi, save multi compressed
void test7(char *inname,char *outname,int compress)
{
imgu *I,*J;
int i,k;
    I=NULL;
    J=NULL;
	printf("-- test 7 : load multi (in ram), save multi compressed --\n");
	if (imguLoadMulti(&I,inname)) { printf("err loading\n");exit(0); }
	k=imguSaveMulti(I,outname,compress);
	if( k ) { printf("err saving %s\n",outname); }
	imguFreeMulti(&I);
}


// load stream, save single, reuse
void test8(char *inname,char *outname,int compress)
{
FILE *F;
imgu *I;
char buf[100];
int i,k;
	I=NULL;
	printf("-- test 8 : load stream, save many single compressed --\n");
	F=fopen(inname,"r");
	if( F==NULL ) { printf("Unable to open %s\n",inname);exit(0); }
	for(i=0;;i++) {
		imguLoadFromFile(&I,F); // try to reuse I (when non NULL)
		if( I==NULL ) break;

		sprintf(buf,outname,i);
		k=imguSave(I,buf,compress);
		if( k ) { printf("err saving %s\n",buf);break; }
	}
    imguFree(&I);
	fclose(F);
}


// load stream, save single, subtract previous
/*void test9(char *inname,char *outname,int compress)
{
FILE *F;
imgu *I,*Iprev,*J;
imgu *IB; // buffer
char buf[100];
int i,j,k,v;

    I=NULL;
    Iprev=NULL;
    J=NULL;
	/// Iprev, I -> save I-Iprev
	/// Iprev, I -> reuse Iprev as J, set Iprev as I, set I as J
	Iprev=I=NULL;
	IB=NULL;
	printf("-- test 9 : load stream, save many single compressed diff --\n");
	F=fopen(inname,"r");
	if( F==NULL ) { printf("Unable to open %s\n",inname);exit(0); }
	for(i=0;;i++) {
		J=imguLoadFromFile(F,Iprev); // try to reuse I (when non NULL)
		if( IB==NULL && J!=NULL ) {
			IB=imguCreate(J->xs,J->ys,J->cs);
		}
		if( J==NULL ) break;
		if( J!=Iprev && Iprev ) imguFree(Iprev); // we got a new image. no reuse.
		Iprev=I;
		I=J;

		// save I-Iprev
		if( I && Iprev ) {
			for(j=I->xs*I->ys*I->cs;j>=0;j--) {
				v=(int)(I->data[j])-(int)(Iprev->data[j]);
				if( v>0 ) v=2*v-1;
				else v=(-v)*2;
				if( v>65535 ) v=65535;
				IB->data[j]=v;
			}
		}

		sprintf(buf,outname,i);
		k=imguSave(IB,buf,compress);
		if( k ) { printf("err saving %s\n",buf);break; }
	}
	fclose(F);
	imguFree(IB);
}*/

// test jpeg
void test10(char *inname,char *outname,char *outnameJPEG,int compress)
{
int k;
imgu *I;
    I=NULL;
	printf("-- test 10 : JPEG : load %s , save %s --\n",inname,outname);
	if (imguLoadJPEG(&I,inname)) { printf("err load %d\n",k);exit(0); }

	imguDump(I);
	k=imguSave(I,outname,1);
	if( k ) { printf("err save %d\n",k);exit(0); }
	k=imguSaveJPEG(I,outnameJPEG,compress);
	if( k ) { printf("err save %d\n",k);exit(0); }
	imguFree(&I);
}


int main(int argc,char *argv[])
{
	//test1();
	//test2("simple.png");
	//test3("simple.png","simple_save.png",1);
	//test3("double.png","double_save.png",1);
	//test3("double_save.png","double_savesave.png",1);
	//test5("rotule.png","rotule_save.png",1);

	//test6("/home/roys/tmp/yo.png","blub%05d.png",1);
	//test7("/home/roys/tmp/yo.png","blub%05d.png",1);
	//test8("/home/roys/tmp/yo.png","blub%05d.png",1);
	//test8("/home/roys/tmp/pipe","blub%05d.png",1);
	//test9("/home/roys/tmp/pipe","blub%05d.png",1);

	//test3("toto_rgba_8.png","out_c0.png",0);
	//test3("toto_rgba_8.png","out_c1.png",1);
	//test3("toto_rgba_8.png","out_c2.png",2);
	
	//test4();

	//test10("allo_rgb.jpg","out_rgb.png","out_rgb.jpg",80);
	//test10("allo_gray.jpg","out_gray.png","out_gray.jpg",80);
	//test10("anim.jpg","out.png","out.jpg",80);
}


