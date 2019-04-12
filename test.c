/*
 *  test.c
 *
 *  copyright (c) 2019 Xiongfei Shi
 *
 *  author: Xiongfei Shi <jenson.shixf(a)gmail.com>
 *  license: Apache2.0
 *
 *  https://github.com/shixiongfei/xtea
 */

#include <stdio.h>
#include <string.h>

#include "xtea.h"

int main(int argc, char *argv[]) {
  const char text[] = { "Hello World!" };
  unsigned char key[] = {
    0x00, 0x01, 0x02, 0x03, 
    0x04, 0x05, 0x06, 0x07, 
    0x08, 0x09, 0x0A, 0x0B, 
    0x0C, 0x0D, 0x0E, 0x0F
  };
  char encode[64] = { 0 };
  char decode[64] = { 0 };
  unsigned char iv[8];
  int enclen, declen, i;
  xtea_t xtea;

  xtea_setkey(&xtea, key);

  enclen = xtea_enclen((int)strlen(text));
  printf("Encode evaluate size: %d\n", enclen);

  memset(iv, 0, sizeof(iv));
  enclen = xtea_encode(&xtea, encode, text, (int)strlen(text), iv);
  printf("Encode real size: %d\n", enclen);

  declen = xtea_declen(enclen);
  printf("Decode evaluate size: %d\n", declen);

  memset(iv, 0, sizeof(iv));
  declen = xtea_decode(&xtea, decode, encode, enclen, iv);
  printf("Decode real size: %d\n", declen);

  printf("Encode: ");
  for (i = 0; i < enclen; ++i) {
    printf("%02x ", (unsigned char)encode[i]);
  }
  printf("\n");
  printf("Decode: %s\n", decode);

  return 0;
}
