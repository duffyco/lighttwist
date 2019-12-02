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

#include <fitsio2.h>
#include <imgu.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include<glob.h>


main (int argc, char *argv[])
{
  glob_t globbuf;
  int i;
  imgu *I;

  fitsfile *fptr;		/* FITS file pointer, defined in fitsio.h */
  int status = 0;		/* CFITSIO status value MUST be initialized to zero! */
  int hdupos, nkeys, ii;
  char value[100];
  char comment[100];
  char *pch;
  int naxis1, naxis2;
  char *pc, temp[255], oname[255],*pc1;
  char key[8];
  char card[255];
  if (argc < 2)
    {
      printf ("needs the files to convert\n");
      return (-1);
    }
  glob (argv[1], GLOB_ERR, NULL, &globbuf);
  for (i = 0; i < globbuf.gl_pathc; i++)
    {
      printf ("Open:%s\n", globbuf.gl_pathv[i]);
      strcpy (temp, globbuf.gl_pathv[i]);
      pc = strtok (temp, ".");
      sprintf (oname, "%s.png", temp);
      fits_open_file (&fptr, globbuf.gl_pathv[i], READONLY, &status);
      if (status)
	{
	  printf ("yo\n");
	  fits_report_error (stderr, status);
	  return (-1);
	}
 fits_read_keyword (fptr, "NAXIS1", value, comment, &status);
	naxis1=atoi(value);
        fits_read_keyword (fptr, "NAXIS2", value, comment, &status);
	naxis2=atoi(value);
             fits_close_file (fptr, &status);

      if (status)
	fits_report_error (stderr, status);



 I=ImguCreate(naxis1,naxis2,1);



fits_open_file (&fptr, globbuf.gl_pathv[0], READONLY, &status);

      fits_get_hdu_num (fptr, &hdupos);


      for (; !status; hdupos++)
	{
	  fits_get_hdrspace (fptr, &nkeys, NULL, &status);


	  for (ii = 1; ii <= nkeys; ii++)
	    {


	      if (fits_read_record (fptr, ii, card, &status))
		break;
	      //printf ("%s\n", card);
              pc = strtok(card,"=");
              pc1 = strtok(NULL,"=");
              //printf("key='%s' txt='%s'\n",pc,pc1);  
	      ImguAddText(I,pc,pc1);

	    }


	  fits_movrel_hdu (fptr, 1, NULL, &status);
	}

      if (status == END_OF_FILE)
	status = 0;


int fpixel = 1;
int  nulval = 0;			// no check for invalid pixels
int  nbuffer = naxis1 * naxis2;
  int anynull;
  fits_read_img (fptr, TUSHORT, fpixel, nbuffer, &nulval, I->data, &anynull,
		 &status);

int i;
unsigned short v;
int min,max;
min=99999;max=0;
for(i=0;i<I->xs*I->ys*I->cs;i++) {
	v=I->data[i];
	//v=(v>>8) | (v<<8);
	v=v*32767/6528;
	I->data[i]=v;
	if( v<min ) min=v;
	if( v>max ) max=v;
	I->data[i]=(v>>8) | (v<<8);
}
printf("min=%d max=%d\n",min,max);
//for(i=0;i<I->xs*I->ys*I->cs;i++) {
//	I->data[i]=((int)I->data[i]*65535*2+1)/(2*max);
//}


/*fits_read_keyword (fptr, "NAXIS1", value, comment, &status);
ImguAddText(I,"NAXIS1",value);
fits_read_keyword (fptr, "NAXIS2", value, comment, &status);
ImguAddText(I,"NAXIS2",value);
/////
  fits_read_keyword (fptr, "CRPIX1", value, comment, &status);
  ImguAddText(I,"CRPIX1",value);
  fits_read_keyword (fptr, "CRPIX2", value, comment, &status);
ImguAddText(I,"CRPIX2",value);
  fits_read_keyword (fptr, "CRVAL1", value, comment, &status);
ImguAddText(I,"CRVAL1",value);
  fits_read_keyword (fptr, "CRVAL2", value, comment, &status);
ImguAddText(I,"CRVAL2",value);
  fits_read_keyword (fptr, "CDELT1", value, comment, &status);
ImguAddText(I,"CDELT1",value);
  fits_read_keyword (fptr, "CDELT2", value, comment, &status);
ImguAddText(I,"CDELT2",value);
  fits_read_keyword (fptr, "PC1_1", value, comment, &status);
ImguAddText(I,"PC1_1",value);
  fits_read_keyword (fptr, "PC1_2", value, comment, &status);
ImguAddText(I,"PC1_2",value);
  fits_read_keyword (fptr, "PC2_1", value, comment, &status);
ImguAddText(I,"PC2_1",value);
  fits_read_keyword (fptr, "PC2_2", value, comment, &status);
ImguAddText(I,"PC2_2",value);
  fits_read_keyword (fptr, "CTYPE1", value, comment, &status);
ImguAddText(I,"CTYPE1",value);
  fits_read_keyword (fptr, "CTYPE2", value, comment, &status);
ImguAddText(I,"CTYPE2",value);
  fits_read_keyword (fptr, "CUNIT1", value, comment, &status);
ImguAddText(I,"CUNIT1",value);
  fits_read_keyword (fptr, "CUNIT2", value, comment, &status);
ImguAddText(I,"CUNIT2",value);
  fits_read_keyword (fptr, "LONPOLE", value, comment, &status);
ImguAddText(I,"LONPOLE",value);
  //fits_read_keyword(fptr,"LATPOLE",value,comment,&status);
  //if( !status ) F->latpole=atof(value);
  //else F->latpole=0.0;
ImguAddText(I,"LOTPOLE","0.0");

  fits_read_keyword (fptr, "DSUN_OBS", value, comment, &status);
ImguAddText(I,"DSUN_OBS",value);
  fits_read_keyword (fptr, "HGLN_OBS", value, comment, &status);
ImguAddText(I,"HGLN_OBS",value);
  fits_read_keyword (fptr, "HGLT_OBS", value, comment, &status);
ImguAddText(I,"HGLT_OBS",value);
//////////////////












*/

      ImguSave (I, oname, 1);
      ImguFree (I);
    }





  globfree (&globbuf);

}
