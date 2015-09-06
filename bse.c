#include "bse.h"
#include "decrypt.h"

#include <stdio.h>
#include <string.h>

static int32_t bse_rand(int32_t * seed);

int bse_is_valid(uint8_t * data, uint32_t size)
{
	if(size < 80) return 0;
	
	return (memcmp((char*)data, "BSE 1.0", 7) == 0);
}

int bse_decrypt(uint8_t * crypted)
{
	int32_t hash = 0;
	uint8_t sum_check = 0;
	uint8_t xor_check = 0;
	uint8_t sum_data = 0;
	uint8_t xor_data = 0;
	int flags[64] = {0};
	int counter = 0;
	
	crypted  += 8;
	/* 0x100  =*/ read16(&crypted);
	sum_check = read8(&crypted);
	xor_check = read8(&crypted);
	hash      = read32(&crypted);
	
	for(counter = 0;counter < 64;counter++)
	{
		int target = NULL;
		int s, k;
		int r = bse_rand(&hash);
		int i = r & 0x3F;
		
		while(flags[i])
		{
			i = (i + 1) & 0x3F;
		}
		
		r = bse_rand(&hash);
		s = r & 0x07;
		target = i;
		
		k = bse_rand(&hash);
		r = bse_rand(&hash);
		r = ((crypted[target] & 255) - r) & 255;
		
		if(k & 1)
		{
			crypted[target] = r << s | r >> (8 - s);
		}
		else
		{
			crypted[target] = r >> s | r << (8 - s);
		}
		
		flags[i] = 1;
	}
	
	for(counter = 0;counter < 64;counter++)
	{
		sum_data = sum_data + (crypted[counter] & 255);
		xor_data = xor_data ^ (crypted[counter] & 255);
	}
	
	if(sum_data == sum_check && xor_data == xor_check)
	{
		return 1;
	}
	
	return 0;
}

int32_t bse_rand(int32_t * seed)
{
	int32_t tmp = (((*seed * 257 >> 8) + *seed * 97) + 23) ^ -1496474763;
	*seed = ((tmp >> 16) & 65535) | (tmp << 16);
	return *seed & 32767;
}
