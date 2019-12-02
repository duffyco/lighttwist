/*
 * This file is part of Lighttwist.
 *
 * @Copyright 2004-2008 Université de Montréal, Laboratoire Vision3D
 *   Sébastien Roy (roys@iro.umontreal.ca)
 *   Vincent Chapdelaine-Couture (chapdelv@iro.umontreal.ca)
 *   Louis Bouchard (lwi.bouchard@gmail.com)
 *   Jean-Philippe Tardif
 *   Patrick Holloway
 *   Nicolas Martin
 *   Vlad Lazar
 *   Jamil Draréni
 *   Marc-Antoine Drouin
 * @Copyright 2005-2007 Société des arts technologiques
 *
 * Lighttwist is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Lighttwist is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Lighttwist.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "tileinfo.h"

#include <glob.h>

// test a tile spanning cpos to cpos+clen+1  compared to a range bmin..bmax
int test_inside(int cpos,int clen,int bmin,int bmax)
{
  if (cpos<=bmax && cpos+clen-1>=bmin) return 1;
  else return 0;
}

//
// movie est de la forme:  bassin si bassin_Tile00000x00000x00000x00000.mp4
//
int fill_tiles_tab(const char* movie, int origW, int origH, vector4 lut_bb, tile_info* tiles, int maxNbTile, int lut_bb_pixels[4] )
{
    int i,nb_tiles;
    FILE *pfp;
    char line[128];
    char cmd[128];

    int tile_inside;
    int len_path;

    //region covered by lut in original movie
    lut_bb_pixels[XMIN]=(int)(lut_bb[XMIN] * (origW-1) + 0.5);
    lut_bb_pixels[XMAX]=(int)(lut_bb[XMAX] * (origW-1) + 0.5);
    lut_bb_pixels[YMIN]=(int)((1.0-lut_bb[YMAX]) * (origH-1) + 0.5);
    lut_bb_pixels[YMAX]=(int)((1.0-lut_bb[YMIN]) * (origH-1) + 0.5);

    printf("lut_x_min=%d ; lut_x_max=%d\n",lut_bb_pixels[XMIN],lut_bb_pixels[XMAX]);
    printf("lut_y_min=%d ; lut_y_max=%d\n",lut_bb_pixels[YMIN],lut_bb_pixels[YMAX]);

    len_path = strlen(movie);

    glob_t g;
    sprintf(cmd,"%s_Tile*.avi",movie);
    glob(cmd, 0, NULL, &g);


    int j;
    for(i=0,j=0;j<g.gl_pathc && i<maxNbTile;j++) {
      printf("*** matched %s\n",g.gl_pathv[j]);
      //remove file extension from filename
      strcpy(tiles[i].filename,g.gl_pathv[j]);
      sscanf(tiles[i].filename+len_path,"_Tile%dx%dx%dx%d",&tiles[i].x,&tiles[i].y,&tiles[i].width,&tiles[i].height);

      tile_inside=0;

      //printf("TEST: %d %d %d %d\n",tiles[i].x,tiles[i].y,tiles[i].width,tiles[i].height);

      if (test_inside(tiles[i].x,tiles[i].width,lut_bb_pixels[XMIN],lut_bb_pixels[XMAX]) &&
          test_inside(tiles[i].y,tiles[i].height,lut_bb_pixels[YMIN],lut_bb_pixels[YMAX]))
      { //tile is overlapping lut
        tile_inside=1;
      }
      else if (lut_bb_pixels[XMAX]>origW-1 &&
          test_inside(tiles[i].x,tiles[i].width,lut_bb_pixels[XMIN]-origW,lut_bb_pixels[XMAX]-origW))
      { //tile is overlapping lut with tile overlaping horizontal boundary
        tile_inside=1;
        tiles[i].x+=origW;
      }
      else if (lut_bb_pixels[YMAX]>origH-1 &&
          test_inside(tiles[i].y,tiles[i].height,lut_bb_pixels[YMIN]-origH,lut_bb_pixels[YMAX]-origH))
      { //tile is overlapping lut with tile overlaping vertical boundary
        tile_inside=1;
        tiles[i].y+=origH;
      }

      if (tile_inside)
      {
        //initialize texture coordinates
        tiles[i].bb_x=0;
        tiles[i].bb_y=0;
        tiles[i].bb_width=tiles[i].width;
        tiles[i].bb_height=tiles[i].height;

        i++;
      }
    }

    nb_tiles=i;
    for(i=0;i<nb_tiles;i++)
    {
        //update texture coordinates
        if (tiles[i].x-lut_bb_pixels[XMIN]<0) tiles[i].bb_x=lut_bb_pixels[XMIN]-tiles[i].x;
        if (tiles[i].y-lut_bb_pixels[YMIN]<0) tiles[i].bb_y=lut_bb_pixels[YMIN]-tiles[i].y;
        if (tiles[i].x+tiles[i].width-1>lut_bb_pixels[XMAX]) tiles[i].bb_width=lut_bb_pixels[XMAX]-tiles[i].x+1;
        if (tiles[i].y+tiles[i].height-1>lut_bb_pixels[YMAX]) tiles[i].bb_height=lut_bb_pixels[YMAX]-tiles[i].y+1;
        tiles[i].bb_width-=tiles[i].bb_x;
        tiles[i].bb_height-=tiles[i].bb_y;

        printf("TILE %d: %d %d %d %d | TEXTURE: %d %d %d %d (%s)\n",
               i,tiles[i].x,tiles[i].y,tiles[i].width,tiles[i].height,
               tiles[i].bb_x,tiles[i].bb_y,tiles[i].bb_width,tiles[i].bb_height, tiles[i].filename);
    }

    return nb_tiles;
}


//
// toto.mp4 -> toto_Tile00000x00000x00000x00000.mp4
//
int getTilesCoord(const char *movie,vector4 lut_bb,tile_info* tiles, int maxNbTile, int lut_bb_pixels[4])
{
    int origW,origH;
    double length;
    char movie_without_suffix[500];
    char stats_filename[500];
    const char *pt_char;
    int pt_char_no;
    //Entry *config;

    if (tiles==NULL) return -1;

    origW=0;
    origH=0;
    //config=NULL;

    //get stats
    pt_char=strrchr(movie,'.');
    pt_char_no = pt_char - movie;
    strncpy (movie_without_suffix, movie, pt_char_no);
    movie_without_suffix[pt_char_no]='\0';
    sprintf(stats_filename,"%s.stats",movie_without_suffix);
    printf("stats_filename = %s\n",stats_filename);

	// le .stats contient sur chaque ligne:
	// nom, 0, width, height, fps, length
    FILE *F=fopen(stats_filename, "r");
    if( F==NULL ) { printf("[getTilesCoord] unable to read %s\n",stats_filename);return 0;}
    int k;
    char name[500];
    fscanf(F," %d=%s",&k,name);
    fscanf(F," %d=%d",&k,&k); // ???
    fscanf(F," %d=%d",&k,&origW);
    fscanf(F," %d=%d",&k,&origH);
    fscanf(F," %d=%lf",&k,&length);

    printf("[getTile] name='%s' w=%d h=%d len=%f\n",name,origW,origH,length);

    //entryFree(&config);

    return fill_tiles_tab(movie_without_suffix, origW, origH, lut_bb,tiles, maxNbTile,lut_bb_pixels);
}


