/*
 *  xtea.c
 *
 *  copyright (c) 2019 Xiongfei Shi
 *
 *  author: Xiongfei Shi <jenson.shixf(a)gmail.com>
 *  license: Apache2.0
 *
 *  https://github.com/shixiongfei/xtea
 */

#include <string.h>

#include "xtea.h"

void xtea_setkey(xtea_t *xtea, const unsigned char key[16]) {
  int i, n;

  for (i = 0; i < 4; ++i) {
    n = i * 4;
    xtea->key[i] = (key[n + 0] << 24) | (key[n + 1] << 16) | (key[n + 2] << 8) |
                   key[n + 3];
  }
}

void xtea_encodeecb(xtea_t *xtea, unsigned char outbuf[8],
                    const unsigned char inbuf[8]) {
  unsigned int sum = 0, delta = 0x9E3779B9;
  unsigned int *k, v0, v1, i;
  unsigned short o1, o2, o3, o4;

  k = xtea->key;

  v0 = (inbuf[0] << 24) | (inbuf[1] << 16) | (inbuf[2] << 8) | inbuf[3];
  v1 = (inbuf[4] << 24) | (inbuf[5] << 16) | (inbuf[6] << 8) | inbuf[7];

  for (i = 0; i < 32; ++i) {
    v0 += (((v1 << 4) ^ (v1 >> 5)) + v1) ^ (sum + k[sum & 3]);
    sum += delta;
    v1 += (((v0 << 4) ^ (v0 >> 5)) + v0) ^ (sum + k[(sum >> 11) & 3]);
  }

  o1 = (unsigned short)(v0 >> 16);
  o2 = (unsigned short)(v0 & 0xFFFF);
  o3 = (unsigned short)(v1 >> 16);
  o4 = (unsigned short)(v1 & 0xFFFF);

  outbuf[0] = (unsigned char)(o1 >> 8);
  outbuf[1] = (unsigned char)(o1 & 0xFF);
  outbuf[2] = (unsigned char)(o2 >> 8);
  outbuf[3] = (unsigned char)(o2 & 0xFF);
  outbuf[4] = (unsigned char)(o3 >> 8);
  outbuf[5] = (unsigned char)(o3 & 0xFF);
  outbuf[6] = (unsigned char)(o4 >> 8);
  outbuf[7] = (unsigned char)(o4 & 0xFF);
}

void xtea_decodeecb(xtea_t *xtea, unsigned char outbuf[8],
                    const unsigned char inbuf[8]) {
  unsigned int delta = 0x9E3779B9, sum = delta * 32;
  unsigned int *k, v0, v1, i;
  unsigned short o1, o2, o3, o4;

  k = xtea->key;

  v0 = (inbuf[0] << 24) | (inbuf[1] << 16) | (inbuf[2] << 8) | inbuf[3];
  v1 = (inbuf[4] << 24) | (inbuf[5] << 16) | (inbuf[6] << 8) | inbuf[7];

  for (i = 0; i < 32; i++) {
    v1 -= (((v0 << 4) ^ (v0 >> 5)) + v0) ^ (sum + k[(sum >> 11) & 3]);
    sum -= delta;
    v0 -= (((v1 << 4) ^ (v1 >> 5)) + v1) ^ (sum + k[sum & 3]);
  }

  o1 = (unsigned short)(v0 >> 16);
  o2 = (unsigned short)(v0 & 0xFFFF);
  o3 = (unsigned short)(v1 >> 16);
  o4 = (unsigned short)(v1 & 0xFFFF);

  outbuf[0] = (unsigned char)(o1 >> 8);
  outbuf[1] = (unsigned char)(o1 & 0xFF);
  outbuf[2] = (unsigned char)(o2 >> 8);
  outbuf[3] = (unsigned char)(o2 & 0xFF);
  outbuf[4] = (unsigned char)(o3 >> 8);
  outbuf[5] = (unsigned char)(o3 & 0xFF);
  outbuf[6] = (unsigned char)(o4 >> 8);
  outbuf[7] = (unsigned char)(o4 & 0xFF);
}

int xtea_encodecbc(xtea_t *xtea, unsigned char *outbuf,
                   const unsigned char *inbuf, int len,
                   unsigned char iv[8]) {
  int i;

  if (len & 7)
    return -1;

  while (len > 0) {
    for (i = 0; i < 8; i++)
      outbuf[i] = (unsigned char)(inbuf[i] ^ iv[i]);

    xtea_encodeecb(xtea, outbuf, outbuf);
    memcpy(iv, outbuf, 8);

    inbuf += 8;
    outbuf += 8;
    len -= 8;
  }

  return 0;
}

int xtea_decodecbc(xtea_t *xtea, unsigned char *outbuf,
                   const unsigned char *inbuf, int len,
                   unsigned char iv[8]) {
  int i;
  unsigned char temp[8];

  if (len & 7)
    return -1;

  while (len > 0) {
    memcpy(temp, inbuf, 8);

    xtea_decodeecb(xtea, outbuf, inbuf);

    for (i = 0; i < 8; i++)
      outbuf[i] = (unsigned char)(outbuf[i] ^ iv[i]);

    memcpy(iv, temp, 8);

    inbuf += 8;
    outbuf += 8;
    len -= 8;
  }

  return 0;
}

int xtea_enclen(int len) {
  int blocks = len / 8;
  return (blocks * 8) + 8;
}

int xtea_declen(int len) { return len; }

int xtea_encode(xtea_t *xtea, void *outbuf, const void *inbuf,
                int inlen, unsigned char iv[8]) {
  int blocks = inlen / 8;
  int block_bytes = blocks * 8;
  unsigned char buffer[8] = {0};
  int padlen;

  if (block_bytes > 0)
    if (0 != xtea_encodecbc(xtea, (unsigned char *)outbuf,
                            (const unsigned char *)inbuf, block_bytes, iv))
      return -1;

  padlen = 8 - (inlen - block_bytes);

  memcpy(buffer, (unsigned char *)inbuf + block_bytes, 8 - padlen);
  memset(buffer + (8 - padlen), padlen, padlen);

  if (0 != xtea_encodecbc(xtea, (unsigned char *)outbuf + block_bytes,
                          buffer, 8, iv))
    return -1;

  return block_bytes + 8;
}

int xtea_decode(xtea_t *xtea, void *outbuf, const void *inbuf,
                int inlen, unsigned char iv[8]) {
  int blocks = inlen / 8;
  int block_bytes = (blocks - 1) * 8;
  unsigned char buffer[8] = {0};
  int padlen;

  if (block_bytes > 0)
    if (0 != xtea_decodecbc(xtea, (unsigned char *)outbuf,
                            (const unsigned char *)inbuf, block_bytes, iv))
      return -1;

  if (0 != xtea_decodecbc(
               xtea, buffer, (const unsigned char *)inbuf + block_bytes, 8, iv))
    return -1;

  padlen = buffer[8 - 1];

  if (padlen <= 8)
    memcpy((unsigned char *)outbuf + block_bytes, buffer, 8 - padlen);

  return block_bytes + (8 - padlen);
}
