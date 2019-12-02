

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <imgu/imgu.h>


#ifdef HAVE_BMC
#include <bmc/udpcast.h>

#include <imgu/imgu_stream.h>

int stream_send_img(imgustream *is, imgu *IA, int flag) {
  return stream_send_img_part(is,IA,flag,0,1);
}

int stream_send_img_part(imgustream *is, imgu *IA, int flag,
             int part, int nbparts)
{
  head *H;
  int i;

  int max_part_pos;
  int part_size;
  int nb_blocks;
  unsigned char *p,*q;
  is->I=IA;


  H=(head *)is->buffer;


  H->tag=0x6a6b;
  H->flag=flag;
  H->n=is->n;
  H->xs=IA->xs;
  H->ys=IA->ys;
  H->cs=IA->cs;
  H->len = is->mtu - sizeof(head); // en bytes


  q=(unsigned char *)(is->buffer)+sizeof(head);

  if(flag == STREAM_8_BITS_HIGH_COMPRESSED || flag == STREAM_16_BITS_COMPRESSED) {
    if (part == 0) imguCompress(IA);
    H->maxpos = IA->compressed_data_size;
    p=(unsigned char *)(IA->data + IA->xs*IA->ys*IA->cs); // buffer to send

  } else {
    H->maxpos = is->I->xs*is->I->ys*is->I->cs;
    p=(unsigned char *)(IA->data);  // buffer to send
  }

  nb_blocks = H->maxpos / H->len; // total de blocks dans le data
  nb_blocks++; // ceiling!
  part_size = nb_blocks / nbparts; // en nombre de blocks de H->len
  part_size++; // ceiling again!;

/**
  printf("len = %d; nb_blocks = %d; part_size = %d; total fits? %d\n",
    H->len, nb_blocks, part_size, part_size*nbparts*H->len);
**/

  H->max_pkts = nb_blocks;

  H->pos = part * part_size * H->len;
  max_part_pos = (part+1) * part_size * H->len;
  if(max_part_pos > H->maxpos) max_part_pos = H->maxpos;

  if( flag==STREAM_16_BITS ) {
    H->pos*=sizeof(unsigned short); // in bytes
    H->maxpos*=sizeof(unsigned short); // in bytes
    max_part_pos*=sizeof(unsigned short); // in bytes
  }

  //printf("MTU is %d sf=%d bs=%d maxpos=%d\n",is->mtu,sizeof(head),H->len,H->maxpos);

  //printf("STREAM: part=%d; pos=%d; maxpoartpos=%d; maxpos=%d; len=%d\n",
  //     part, H->pos, max_part_pos, H->maxpos, H->len);

  while( H->pos < H->maxpos && H->pos < max_part_pos ) {
    if( H->pos+H->len > max_part_pos ) H->len = max_part_pos-H->pos;
    if( H->pos+H->len > H->maxpos ) H->len = H->maxpos-H->pos;




    // printf("sending tag='%c%c' flag=%d n=%d xs=%d ys=%d cs=%d pos=%d len=%d maxpos=%d\n",
    // ((unsigned char *)(&H->tag))[0],
    // ((unsigned char *)(&H->tag))[1],
    // H->flag,H->n,H->xs,H->ys,H->cs,H->pos,H->len,H->maxpos);


    if( flag == STREAM_16_BITS ) {
      memcpy(q,p+H->pos,H->len);

    } else if( flag == STREAM_8_BITS_HIGH ) {
      for(i=0;i<H->len;i++) q[i]=p[(H->pos+i)*2+1];

    } else if(flag == STREAM_8_BITS_HIGH_COMPRESSED || flag == STREAM_16_BITS_COMPRESSED) {
      memcpy(q,p+H->pos,H->len);
    }

    udp_send_data(&is->uc,(unsigned char *)(is->buffer),sizeof(head)+H->len);
    usleep(10);

    // done sending... next block!
    H->pos+=H->len;
  }
  //printf("done sending\n");
  is->n++;
  is->I=NULL;
  return(0);
}

int stream_receive_img(imgustream *is,imgu **pIA) {
  return stream_receive_img_part(is,pIA,-1);
}

static int col = 0;
// receive a single image
int stream_receive_img_part(imgustream *is,imgu **pIA, int maxblocks)
{
  head *H;
  unsigned char *p,*q;
  int nblocks = 0;

  is->I=NULL;
  is->pI=pIA;

  col = (col + 50 ) % 230;

  H=(head *)is->buffer;
  q=(unsigned char *)(is->buffer)+sizeof(head);


  for(;;) {
    //printf("waiting for message\n");
    if( udp_receive_data(&is->uc,(unsigned char *)(is->buffer),is->mtu) <0 ) {
      printf("ERR\n");continue;
    }
    nblocks++;

    //printf("imgu_stream: expecting img %i, got %i?\n", is->curr_n, H->n);

	//
	// on oublie les vielles images.
	// ca devrait etre en parametre...
	//
    if (H->n > is->curr_n) is->curr_n = H->n;
    else if (H->n < is->curr_n) {
        //printf("OLD IMAGE!! IGNORE IT!\n");
      continue; // OLD IMAGE!! IGNORE IT!

    }
    is->nb_pkts++;

/*
      printf("received tag='%c%c' flag=%d n=%d xs=%d ys=%d cs=%d pos=%d len=%d maxpos=%d\n",
      ((unsigned char *)(&H->tag))[0],
      ((unsigned char *)(&H->tag))[1],
      H->flag,H->n,H->xs,H->ys,H->cs,H->pos,H->len,H->maxpos);
*/

    p=NULL;   
    if( is->I==NULL ) {
      if( H->flag==STREAM_16_BITS ) {
    imguAllocate(is->pI,H->xs,H->ys,H->cs);
    is->I=*(is->pI);
    p=(unsigned char *)(is->I->data);
      } else if( H->flag==STREAM_8_BITS_HIGH ) {
    imguAllocateExtra(is->pI, H->xs, H->ys, H->cs, H->xs*H->ys*H->cs ); // bytes en dehors de l'image
    is->I=*(is->pI);
    p=(unsigned char *)((*(is->pI))->data+H->xs*H->ys*H->cs);
      } else if(H->flag == STREAM_8_BITS_HIGH_COMPRESSED || H->flag == STREAM_16_BITS_COMPRESSED) {
    imguAllocateExtra(is->pI, H->xs, H->ys, H->cs, H->maxpos ); // bytes en dehors de l'image
    is->I=*(is->pI);
    p=(unsigned char *)((*(is->pI))->data+H->xs*H->ys*H->cs);
    is->I->compressed_data_size = H->maxpos;
      }
      else
      {
        return -1;
      }
    }

    if(maxblocks==-1)
      maxblocks = H->maxpos;

    // le mode ne change rien a la copie des informations
    //memset((void *)(p+H->pos),255,H->len);
    memcpy((void *)(p+H->pos),q,H->len);
    //      for(i=0;i<H->len;i++) { printf("%02x ",q[i]);if( i%32==0 ) printf("\n"); }
    //      printf("\n");
    //memset((void *)(p+nblocks*H->len),col,H->len);
    //memcpy((void *)(p+nblocks*H->len),q,H->len);
    //printf("nblocks=%d; maxblox=%d; pos=%d; len=%d; maxpos%d\n",
    //       nblocks, maxblocks, H->pos, H->len, H->maxpos);

    if( H->pos+H->len==H->maxpos || nblocks >= maxblocks ) {
      if( H->flag==STREAM_8_BITS_HIGH ) {
    //printf("unpacking...\n");
    imguUnpack8bit(is->I,p,LOAD_16_BITS);
      }
      break;
    }
  }

  if( H->pos+H->len==H->maxpos ) {
    is->img_done = 1;
    is->n++; // useless?  curr_n is more gooder.. maybe redundant...
    if (H->flag == STREAM_8_BITS_HIGH_COMPRESSED || H->flag == STREAM_16_BITS_COMPRESSED) {
      if (is->nb_pkts == H->max_pkts) imguUncompress(is->I);
      else {
        printf("PACKETS MISSING (%d/%d)... will not attempt to uncompress\n", is->nb_pkts, H->max_pkts);
        is->img_done = 0; // not uncompressed.. therefore not done.  imgu_inet will not increment rotating buffer index
      }
    }
    //imguSavePNG(is->I, "uncompress.png", 1, SAVE_16_BITS);
    //printf("pixel 100,100 = %d\n",PIXEL(is->I,100,100,0));
    is->nb_pkts = 0;
  }
  else {
    is->img_done = 0;
  }

  //printf("imgu_stream: DONE = %d\n", is->img_done);

  return(0);
}

int init_send_stream(imgustream *is,char *host,int port,int type)
{
  int k;
  k=udp_init_sender(&is->uc,host,port,type);
  is->mtu=1450; // should be set inside uc (utiliser maximum MTU-28 bytes)
  is->n=0; // # de la prochaine image ou courante
  is->buffer=(unsigned int *)malloc(is->mtu);
  is->img_done = 0;
  is->curr_n = 0;
  return(k);
}

int uninit_send_stream(imgustream *is)
{
  udp_uninit_sender(&is->uc);
  free(is->buffer);
  return(0);
}

int init_receive_stream(imgustream *is,char *host,int port)
{
  int k;
  k=udp_init_receiver(&is->uc,port,host);
  if( k ) { printf("unable to access the network\n");return(-1); }

  is->mtu=1450; // should be set inside uc (utiliser maximum MTU-28 bytes)
  is->n=0; // # de la prochaine image ou courante
  is->buffer=(unsigned int *)malloc(is->mtu);
  is->curr_n = 0;
  is->img_done = 0;
  return(0);
}

int uninit_receive_stream(imgustream *is)
{
  udp_uninit_receiver(&is->uc);
  free(is->buffer);
  return(0);
}

#endif // BMC


