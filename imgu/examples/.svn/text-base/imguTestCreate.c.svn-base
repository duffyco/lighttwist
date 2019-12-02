/*
 * Test
 * - image creation
 * - image save
 * - image load
 *
 */

//
// Flags for saving:
//
// SAVE_16_BITS : save as a 16 bit image
//
// SAVE_8_BITS : save as a 8 bit image (lower 8 bits if imgu16)
// SAVE_8_BITS_LOW
//
// SAVE_8_BITS_HIGH : save as a 8 bit image (hi 8 bits if imgu16)
//
//       /// Save a single byte (8 bit image) as the HI of a 16bbp image
// SAVE_16_BITS_HIGH : save as a 16 bit image (imgu8: put the 8 bits in hi part)
// SAVE_16_BITS_LOW  : save as a 16 bit image (imgu8: put the 8 bits in hi part)
//
// SAVE_AS_IS : save as 16 bit image if imgu16, 8 bit image if imgu8
//

//
// create an image with value IMGU_MAXVAL-1  (254 in 8 bit) and (65534 or FFFE in 16 bits)
//
// save with all possible save flags
//
// check the bit size of each image
//
// reload all images with LOAD_AS_IS
//
// display the result values
// 
//



#include <string.h>

#include <imgu.h>

#ifdef IMGU8
 #define MODE	8
#else
 #define MODE	16
#endif

int loadFlags[] = {
	LOAD_16_BITS,
	LOAD_AS_IS,
	LOAD_8_BITS_HIGH,
	LOAD_8_BITS_LOW,
	-1
};

char *loadFlagsNames[] = {
	"LOAD_16_BITS",
	"LOAD_AS_IS",
	"LOAD_8_BITS_HIGH",
	"LOAD_8_BITS_LOW",
	NULL
};

int saveFlags[] = {
	SAVE_16_BITS,
	SAVE_8_BITS,
	SAVE_8_BITS_LOW,
	SAVE_8_BITS_HIGH,
	SAVE_16_BITS_HIGH,
	SAVE_16_BITS_LOW,
	SAVE_AS_IS,
	-1
};

char *saveFlagsNames[] = {
	"SAVE_16_BITS",
	"SAVE_8_BITS",
	"SAVE_8_BITS_LOW",
	"SAVE_8_BITS_HIGH",
	"SAVE_16_BITS_HIGH",
	"SAVE_16_BITS_LOW",
	"SAVE_AS_IS",
	NULL
};


char *fileNames[100]; // test filenames (8 and 16)
int imageBits[100]; // 8 or 16, of the original image
int imageFlag[100]; // SAVE_
char *imageFlagName[100];



int resultBitsSave[100]; // 8 or 16, observed in the save data
int resultValue[400];


static void makefiles(void)
{
int i,j,s;
char buf[100];
	j=0;
	for(s=1;s<=2;s++) {
	for(i=0;saveFlagsNames[i];i++) {
		sprintf(buf,"tmp/test_create_%d_save_%s.png",s*8,saveFlagsNames[i]);
		fileNames[j]=strdup(buf);
		imageBits[j]=s*8;
		imageFlag[j]=saveFlags[i];
		imageFlagName[j]=saveFlagsNames[i];
		j++;
	}
	}
	fileNames[j]=0;

	for(i=0;fileNames[i];i++) printf("Filename is %s\n",fileNames[i]);
}

static void saveonly(void)
{
	imgu *I=NULL;
	imguAllocate(&I,100,100,1);

	int x,y;
	for(y=0;y<I->ys;y++)
	for(x=0;x<I->xs;x++) PIXEL(I,x,y,0)=IMGU_MAXVAL-1; // 254 if IMGU8, 65534 in IMGU16

	int i;
	for(i=0;fileNames[i];i++) {
		if( imageBits[i]==MODE ) imguSave(I,fileNames[i],1,imageFlag[i]);
	}
}


static void checksave(void)
{
int i,j,k;
char cmd[100];
imgu *J=NULL;
	for(i=0;fileNames[i];i++) {
		// est-ce que le fichier existe?
		sprintf(cmd,"ls %s",fileNames[i]);
		k=system(cmd);
		printf("LS %s returned %d\n",fileNames[i],k);
		if( k!=0 ) {
			resultBitsSave[i]=0;
			continue;
		}
		// regarde si c'est bien 16 ou 8 bits avec pngcheck
		sprintf(cmd,"pngcheck %s | grep 16-bit",fileNames[i]);
		k=system(cmd);
		printf("k=%d\n",k);
		if( k==0 ) resultBitsSave[i]=16; else resultBitsSave[i]=8;


		// load l'image et regarde la valeur dedans...
		for(j=0;loadFlagsNames[j];j++) {
			k=imguLoad(&J,fileNames[i],loadFlags[j]);
			resultValue[i*10+j]=PIXEL(J,0,0,0);
		}
	}
}

void results()
{
int i,j;
	printf("-- IMGU SAVE (16b value is 0x%04x, 8b value is 0x%02x --\n",255-1,65535-1);
	for(i=0;fileNames[i];i++) {
		printf("%20s : %2db in -> %2db out,",imageFlagName[i], imageBits[i],resultBitsSave[i]);
		for(j=0;loadFlagsNames[j];j++) {
#ifdef IMGU8
			printf(" %s=0x%02x",loadFlagsNames[j],resultValue[i*10+j]);
#else
			printf(" %s=0x%04x",loadFlagsNames[j],resultValue[i*10+j]);
#endif
		}
		printf("\n");
	}
}



void doit() {
	imgu *I=NULL;
	imgu *J=NULL;
	imguAllocate(&I,256,256,3);

	int x,y;
	for(y=0;y<I->ys;y++)
	for(x=0;x<I->xs;x++) {
		PIXEL(I,x,y,0)=x;
		PIXEL(I,x,y,1)=y;
		PIXEL(I,x,y,2)=(x+y)/2;
	}

	imguSave(I,"tmp/test_create_save8_8b.png",1,SAVE_8_BITS);
	imguSave(I,"tmp/test_create_save8_16b.png",1,SAVE_16_BITS); // =HIGH
	imguSave(I,"tmp/test_create_save8_16bh.png",1,SAVE_16_BITS_HIGH);
	imguSave(I,"tmp/test_create_save8_16bl.png",1,SAVE_16_BITS_LOW);

	imguLoad(&J,"tmp/test_create_save8_8b.png",LOAD_AS_IS);
	imguSave(J,"yo1.png",1,SAVE_8_BITS);

	imguLoad(&J,"tmp/test_create_save8_16b.png",LOAD_AS_IS); // =HIGH
	imguSave(J,"yo2.png",1,SAVE_8_BITS);

	imguLoad(&J,"tmp/test_create_save8_16bh.png",LOAD_8_BITS_HIGH);
	imguSave(J,"yo3.png",1,SAVE_8_BITS);

	imguLoad(&J,"tmp/test_create_save8_16bl.png",LOAD_8_BITS_LOW);
	imguSave(J,"yo4.png",1,SAVE_8_BITS);

	imguLoad(&J,"tmp/test_create_save8_16bh.png",LOAD_8_BITS_LOW);
	imguSave(J,"yo5.png",1,SAVE_8_BITS);

	imguLoad(&J,"tmp/test_create_save8_16bl.png",LOAD_8_BITS_HIGH);
	imguSave(J,"yo6.png",1,SAVE_8_BITS);

}


#ifdef SKIP

#include <imgu.h>

void doit() {
	imgu *I=NULL;
	imguAllocate(&I,256,256,3);

	int x,y,c;
	for(y=0;y<I->ys;y++)
	for(x=0;x<I->xs;x++) {
		PIXEL(I,x,y,0)=x*257;
		PIXEL(I,x,y,1)=y*257;
		PIXEL(I,x,y,2)=(x+y)/2*257;
	}

	imguSave(I,"test_create_save_16b.png",1,SAVE_16_BITS);
	imguSave(I,"test_create_save_8b.png",1,SAVE_8_BITS_HIGH);
}

#endif




int main( int argc, char **argv )
{
	printf("-- imgu Test Create (%d bits) --\n",sizeof(pix_t)*8);
	printf("SAVE_16_BITS=%d SAVE_16_BITS_HIGH=%d LOW=%d\n",
		SAVE_16_BITS,
		SAVE_16_BITS_HIGH,
		SAVE_16_BITS_LOW);
	makefiles();
	saveonly();
	checksave();
	results();

    return 0;
}

