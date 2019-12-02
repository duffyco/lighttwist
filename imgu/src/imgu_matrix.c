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
#include <string.h>

int imguLoadMatrix(matrix **mat,imgu *I,const char *key)
{
    char *buf,*ctemp;
    int rs,cs;
    int i,j,k;
    
	if( mat==NULL ) return -1;
	if( I==NULL ) return -1;
	if( key==NULL ) return -1;

    buf=imguGetText(I,key);
    if (buf==NULL) return -1;
    k=sscanf(buf,"%d %d\n",&cs,&rs);
    if (k!=2) return -1;
    matAllocate(mat,cs,rs);
    ctemp=strchr(buf,'\n');
    if (ctemp==NULL) return -1;
    buf=ctemp+1;
    for (i=0;i<(*mat)->cs;i++)
    {
        for (j=0;j<(*mat)->rs;j++)
        {
            k=sscanf(buf,"%lf ",&((*mat)->values[i*(*mat)->rs+j]));
            ctemp=strchr(buf,' ');
            if (ctemp==NULL) return -1;
            buf=ctemp+1;
        }
        buf++;
    }

	return 0;
}


int imguSaveMatrix(matrix *mat,imgu *I,const char *key)
{
    int i,j;
    char buf[MAT_BUFFER_SIZE];

    sprintf(buf,"%d %d\n",mat->cs,mat->rs);
    imguReplaceAddText(I,key,buf);
    for (i=0;i<mat->cs;i++)
    {
        for (j=0;j<mat->rs;j++)
        {
            sprintf(buf,"%.16f ",mat->values[i*mat->rs+j]);
            imguAppendText(I,key,buf);
        }
        sprintf(buf,"\n");
        imguAppendText(I,key,buf);
    }

    return 0;
}

