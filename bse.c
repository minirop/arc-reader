#include <string.h>
#include <stdlib.h>
#include <stdint.h>
#include "bse.h"

static int bse_rand(int * seed)
{
	int tmp = ((*seed * 257 >> 8) + *seed * 97) + 23 ^ -1496474763;
	*seed = tmp >> 16 & 65535 | tmp << 16;
	return *seed & 32767;
}

void bse_decrypt(char * data, uint32_t * filesize) {
	int r, counter;
  char *copy;
	int flags[64], seed, i, s;
	char *target;
	int check_sum,check_xor;
	char *header = data, *body = data + 16;

	if (strcmp(header + 0, "BSE 1.0"))
    return;
	if (*(short *)(header + 8) != 256)
    return;

  copy = malloc(sizeof(copy) * (*filesize - 16));
  memcpy(copy, data, *filesize);

	memset(flags, 0, 64 * 4);
	seed = *(long *)(header + 12);
	counter = 0;
	do {
		r = bse_rand(&seed);
		i = r & 64 - 1;
		while (flags[i]){
			i = i + 1 & 64 - 1;
		}
		r = bse_rand(&seed);
		s = r & 7;
		target = body + i;
		r = bse_rand(&seed);
		switch(r & 1){
		case 0:
			r = bse_rand(&seed);
			r = (*target & 255) - r & 255;
			*target = r >> s | r << 8 - s;
			break;
		case 1:
			r = bse_rand(&seed);
			r = (*target & 255) - r & 255;
			*target = r << s | r >> 8 - s;
			break;
		}
		flags[i] = 1;
		counter++;
	} while (counter < 64);
	check_sum = 0;
	check_xor = 0;
	target = body + 0;
	counter = 0;
	do {
		check_sum = check_sum + (target[counter] & 255);
		check_xor = check_xor ^ (target[counter] & 255);
    counter++;
	} while (counter < 64);

  if((header[10] & 255) == (check_sum & 255) &&
      (header[11] & 255) == (check_xor & 255)) {
    free(copy);
    memmove(data, data + 16, *filesize - 16);
    *filesize -= 16;
  } else {
    free(data);
    data = copy;
  }
}
