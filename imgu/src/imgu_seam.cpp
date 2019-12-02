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

#define SEAM_COST_INF 1000000

static int node_id(int *spos,int ssize,int ysize,int x,int y,int t)
{
  int i,index;
  index=0;
  for (i=0;i<t;i++)  
  {
    index+=ysize*spos[2*i+1]; 
  }  
  index+=y*spos[2*i+1];
  index+=x-spos[2*i];
//  index+=2; //sink, source
  return index;
}

static int isNeighbor(int x,int y,int t,int j,int i,int k,int direction)
{
  if ((x==j || x==j-1) && y==i && t==k && direction==0) return 1;
  if (x==j && (y==i || y==i-1) && t==k && direction==1) return 1;
  if (x==j && y==i && (t==k || t==k-1) && direction==2) return 1;

  return 0;
}

static int all_non_empty(imgu *I,int x,int y)
{
  int c,non_empty;

  non_empty=1;
  for (c=0;c<I->cs;c++)
  {
    if (PIXEL(I,x,y,c)==0) non_empty=0;
  }
  return non_empty;
}

//find best cut transition between Ivideo1 and Ivideo2
//store mask result in Ivideo1
//xinit is used to set cut of first frame
int imguSeam(imgu *Ivideo1,imgu *Ivideo2,unsigned char usegrad,imgu *Iinit)
{
  int i,j,k,c;
  imgu *I1,*I2;
  imgu *I1prev,*I2prev;
  int count;
  int node,node_prev;
  int M,grad;
  float cost;
  int icost;
  int *spos,ssize;
  float gc_scale=1.0;

  if (Ivideo1==NULL || Ivideo1->data==NULL) return -1;
  if (Ivideo2==NULL || Ivideo2->data==NULL) return -1;
  if (imguCount(Ivideo1) != imguCount(Ivideo2)) return -1;
  if (Ivideo1->xs!=Ivideo2->xs) return -1;
  if (Ivideo1->ys!=Ivideo2->ys) return -1;
  if (Iinit!=NULL && Ivideo1->xs!=Iinit->xs && Ivideo1->ys!=Iinit->ys) return -1;

  ssize=imguCount(Ivideo1);
  spos=(int *)(malloc(sizeof(int)*ssize*2));

  I1=Ivideo1;
  I2=Ivideo2;
  count=0;
  k=0;
  while(I1 && I2)
  {
    //imguConvertToLuminance(&I1,I1);
    //imguConvertToLuminance(&I2,I2);        

    for (i=0;i<I1->ys;i++)
    {
      for (j=0;j<I1->xs;j++)
      {
        if (all_non_empty(I1,j,i)) break;
      }
      if (i==0) spos[2*k]=j;
      else if (j<spos[2*k]) spos[2*k]=j;

      for (;j<I1->xs;j++)
      {        
        if (all_non_empty(I1,j,i)==0) break;
      }
      if (i==0)
      {
        if (spos[2*k]==j) spos[2*k+1]=-1; //row is probably empty
        else spos[2*k+1]=j; 
      }
      else if (j>spos[2*k+1]) spos[2*k+1]=j;
    }
    spos[2*k+1]-=spos[2*k];
   
    //printf("%d %d\n",spos[2*k],spos[2*k+1]);
    count+=I1->ys*spos[2*k+1];

    k++;
    I1=I1->next;
    I2=I2->next;
  }

  for (i=0;i<ssize;i++)
  {
    //fprintf(stderr,"%d %d\n",spos[2*i],spos[2*i+1]);
  }

//return 0;

  //scale costs to avoid overflow
  gc_scale=(1 << 30)/(128.0f*count);
  if (gc_scale>1.0f) gc_scale=1.0f;
  gc_scale=1.0f;
  //printf("[imgu_seam] graph-cut scale: %f.\n",gc_scale);

  //typedef Graph<int,int,int> GraphType;
  typedef Graph<float,float,float> GraphType;
  GraphType *g = new GraphType(/*estimated # of nodes*/ count, /*estimated # of edges*/ 3*count); 
  if (g==NULL)
  {
    //fprintf(stderr,"[imgu] Could not allocate graph-cut.\n");
    return -1;
  }
  g -> add_node(count); 

  I1prev=NULL;
  I2prev=NULL;
  I1=Ivideo1;
  I2=Ivideo2;
  count=0;
  k=0;
  imgu *Iout=NULL;
  imguAllocate(&Iout,I1->xs,I1->ys,3);
  while(I1 && I2)
  {
    imguClear(Iout);
//fprintf(stderr,"Costs: %d\n",k);
//fflush(stderr);
    for (i=0;i<I1->ys;i++)
    {
      for (j=0;j<I1->xs;j++)
      {
        if (all_non_empty(I1,j,i))
        {
          node=node_id(spos,ssize,Ivideo1->ys,j,i,k);          
          //horizontal cost
          if (j!=0 && all_non_empty(I1,j-1,i))
          {
            node_prev=node_id(spos,ssize,Ivideo1->ys,j-1,i,k);
            grad=1;
            if (usegrad) for (c=0;c<I1->cs;c++) grad+=abs(PIXEL(I1,j,i,c)-PIXEL(I1,j-1,i,c))+abs(PIXEL(I2,j,i,c)-PIXEL(I2,j-1,i,c));            
            M=0;
            for (c=0;c<I1->cs;c++) M+=abs(PIXEL(I1,j,i,c)-PIXEL(I2,j,i,c))+abs(PIXEL(I1,j-1,i,c)-PIXEL(I2,j-1,i,c));        
            cost=M/grad;
            //cost=0.1;
            icost=(int)(cost+0.5);
            if (icost>255) icost=255;
            //PIXEL(Iout,j,i,0)=icost;
            g -> add_edge( node, node_prev, cost*gc_scale, cost*gc_scale );
            if (k==0 && Iinit!=NULL)
            {
              if (PIXEL(Iinit,j,i,0)==0) g -> add_tweights( node, SEAM_COST_INF, 0 );
            }
//if (isNeighbor(80,21,14,j,i,k,0)) printf("horizontal (%d,%d,%d): %f\n",j,i,k,cost);
          }          
          //if (j==0 || PIXEL(I1,j-1,i,0)==0) //first column, infinite link towards patch1
          else
          {
            g -> add_tweights( node, SEAM_COST_INF, 0 );
          }
          if (j==I1->xs-1 || all_non_empty(I1,j+1,i)==0) //last column, infinite link towards patch2
          {
            g -> add_tweights( node, 0, SEAM_COST_INF );
          }       
          else
          { 
            if (k==0 && Iinit!=NULL)
            {
              if (PIXEL(Iinit,j,i,0)!=0) g -> add_tweights( node, 0, SEAM_COST_INF );
            }
          }
          //vertical cost
          if (i!=0 && all_non_empty(I1,j,i-1))
          {
            node_prev=node_id(spos,ssize,Ivideo1->ys,j,i-1,k);
            grad=1;
            if (usegrad) for (c=0;c<I1->cs;c++) grad+=abs(PIXEL(I1,j,i,c)-PIXEL(I1,j,i-1,c))+abs(PIXEL(I2,j,i,c)-PIXEL(I2,j,i-1,c));           
            M=0;
            for (c=0;c<I1->cs;c++) M+=abs(PIXEL(I1,j,i,c)-PIXEL(I2,j,i,c))+abs(PIXEL(I1,j,i-1,c)-PIXEL(I2,j,i-1,c));
            cost=M/grad;
            //cost=0.1;
            icost=(int)(cost+0.5);
            if (icost>255) icost=255;
            PIXEL(Iout,j,i,1)=icost;
            g -> add_edge( node, node_prev, cost*gc_scale, cost*gc_scale );
//if (isNeighbor(80,21,14,j,i,k,1)) printf("vertical (%d,%d,%d): %f\n",j,i,k,cost);
          }
          //temporal cost
          if (I1prev!=NULL && all_non_empty(I1prev,j,i))
          {
            node_prev=node_id(spos,ssize,Ivideo1->ys,j,i,k-1);
            grad=1;
            if (usegrad) for (c=0;c<I1->cs;c++) grad+=abs(PIXEL(I1,j,i,c)-PIXEL(I1prev,j,i,c))+abs(PIXEL(I2,j,i,c)-PIXEL(I2prev,j,i,c));
            M=0;
            for (c=0;c<I1->cs;c++) M+=abs(PIXEL(I1,j,i,c)-PIXEL(I2,j,i,c))+abs(PIXEL(I1prev,j,i,c)-PIXEL(I2prev,j,i,c));
            cost=M/grad;
            //if (j==spos[1]+10 && k!=40) cost=40;
            //else cost=0.1;
            icost=(int)(cost+0.5);
            if (icost>255) icost=255;
            //PIXEL(Iout,j,i,2)=icost;
            g -> add_edge( node, node_prev, cost*gc_scale, cost*gc_scale );
//if (isNeighbor(80,21,14,j,i,k,2)) printf("temporal (%d,%d,%d): %f\n",j,i,k,cost);
          }
        }
        count++;
      }
    }
    I1prev=I1;
    I2prev=I2;
    I1=I1->next;
    I2=I2->next;
    //sprintf(filename,"cc_%08d.png",k+1);
    //imguSave(Iout,filename,1,SAVE_8_BITS_LOW);
    k++;
  }

//fprintf(stderr,"Computing max flow...");
//fflush(stderr);
  int flow = g -> maxflow();
//fprintf(stderr,"done\n");
//fflush(stderr);

  I1=Ivideo1;
  k=0;
  while(I1)
  {
    for (i = 0; i < I1->ys; i++)
    {
      for (j = 0; j < I1->xs; j++)
      {
        if (all_non_empty(I1,j,i))
        {
          node=node_id(spos,ssize,Ivideo1->ys,j,i,k);
          if (g->what_segment(node) == GraphType::SOURCE)
          {
            PIXEL(I1,j,i,0) = 0; //link to source is cut
          }
  		  else if (g->what_segment(node) == GraphType::SINK)
          {
            PIXEL(I1,j,i,0) = IMGU_MAXVAL; //link to sink is cut
          }
          else
          {
            PIXEL(I1,j,i,0) = IMGU_MAXVAL/2;
          }
        }
        else
        {
          PIXEL(I1,j,i,0) = 0;
        }
      }
    }
    I1=I1->next;
    k++;
  }

  imguFree(&Iout);  
	
  delete g;
  free(spos);

  return 0;
}


