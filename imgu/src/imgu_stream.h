#ifndef IMGU_STREAM_H
#define IMGU_STREAM_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <imgu/imgu.h>
#include <bmc/udpcast.h>

#ifdef __cplusplus
extern "C" {
#endif



typedef struct {
  udpcast uc;   // [both] network access
  int mtu;  // [both] maximum total message size (and sizeof buffer)
  int n;        // [both] image number of next image to send or currently sent
            //    we always send this number with each block
  int pos;  // [send] position inside the image of the next message to send
  imgu *I;  // [send] image being sent (NULL = not sending anything)
  imgu **pI;    // [recv] pointer to image structure for receiving.
  unsigned int *buffer; // [both] [size=mtu]
  int img_done; // (has H->pos reached maxpos)? 1 : 0 .. mainly for recieve
  int curr_n; // the image number that we SHOULD be currently recieving
  int nb_pkts; // number of packets recieved
} imgustream;

#define STREAM_16_BITS 0
#define STREAM_8_BITS_HIGH 1
#define STREAM_16_BITS_COMPRESSED 2
#define STREAM_8_BITS_HIGH_COMPRESSED 3

typedef struct {
  unsigned short tag; // always 0x6a6b or 0x6b6a selon swapped ou non
  unsigned short flag; // 0=full image, 1=high 8 bits only
  int n;            // numero sequentiel d'image
  unsigned short xs,ys,cs; // taille de l'image
  int pos;
  int maxpos;       // taille totale a reserver pour le download
  unsigned short len;       // length of this block data
  int max_pkts;
} head;


// flag: STREAM_16_BITS ou _8_BITS_HIGH
int stream_send_img(imgustream *is,imgu *IA,int flag);
int stream_send_img_part(imgustream *is,imgu *IA, int flag,
             int part, int nbparts);

// receive a single image
int stream_receive_img(imgustream *is,imgu **pIA);
int stream_receive_img_part(imgustream *is,imgu **pIA,
int maxblocks);


int init_send_stream(imgustream *is,char *host,int port,int type);

int uninit_send_stream(imgustream *is);

int init_receive_stream(imgustream *is,char *host,int port);

int uninit_receive_stream(imgustream *is);

#ifdef __cplusplus
}
#endif

#endif

