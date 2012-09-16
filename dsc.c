#include "dsc.h"
#include "decrypt.h"
#include "write.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

struct NodeDSC
{
	uint32_t has_childs;
	uint32_t leaf_value;
	uint32_t childs[2];
};

static int buffer_sorting(const void * a, const void * b);
static int dsc_is_image(uint8_t * data);

int dsc_is_valid(uint8_t * data, uint32_t size)
{
	if(size < 32) return 0;
	
	return (memcmp((char*)data, "DSC FORMAT 1.00", 15) == 0);
}

uint8_t * dsc_decrypt(uint8_t * crypted, uint32_t crypted_size, uint32_t * decrypted_size)
{
	int i, n;
	uint32_t hash, size;
	uint8_t * begin_of_raw_data = crypted;
	int buffer_len = 0;
	uint32_t buffer[512] = {0};
	uint32_t vector0[1024] = {0};
	uint32_t nn = 0, toggle = 0x200, dec0 = 1, value_set = 1;
	uint32_t * v13 = vector0;
	int buffer_cur;
	uint8_t * data;
	uint32_t bits = 0, nbits = 0;
	uint32_t src_ptr = 0, dst_ptr = 0;
	uint32_t src_end, dst_end;
	struct NodeDSC * nodes;
	
	crypted += 16;
	hash = read32(&crypted);
	size = read32(&crypted);
	/*uint32_t v2 = */ read32(&crypted);
	/*uint32_t padding = */ read32(&crypted);
	*decrypted_size = size;
	
	nodes = malloc(sizeof(*nodes) * 1024);
	for(i = 0;i < 1024;i++)
	{
		nodes[i].has_childs = 0;
		nodes[i].leaf_value = 0;
	}
	
	for(n = 0;n < 512;n++)
	{
		uint8_t v = crypted[n] - (uint8_t)hash_update(&hash);
		if(v)
		{
			buffer[buffer_len] = (v << 16) + n;
			buffer_len++;
		}
	}
	
	qsort(buffer, buffer_len, sizeof(buffer[0]), buffer_sorting);
	
	for(buffer_cur = 0;buffer_cur < buffer_len;nn++)
	{
		uint32_t * vector0_ptr = &vector0[toggle];
		uint32_t * vector0_ptr_init = vector0_ptr;
		uint32_t group_count = 0;
		uint32_t v18;
		
		for( ;nn == myHIWORD(buffer[buffer_cur]); buffer_cur++, v13++, group_count++)
		{
			nodes[*v13].has_childs = 0;
			nodes[*v13].leaf_value = buffer[buffer_cur] & 0x1FF;
		}
		
		v18 = 2 * (dec0 - group_count);
		if(group_count < dec0)
		{
			uint32_t dd;
			dec0 = (dec0 - group_count);
			for(dd = 0; dd < dec0; dd++)
			{
				int m;
				nodes[*v13].has_childs = 1;
				for(m = 0; m < 2; m++)
				{
					*vector0_ptr++ = nodes[*v13].childs[m] = value_set;
					value_set++;
				}
				v13++;
			}
		}
		dec0 = v18;
		v13 = vector0_ptr_init;
		toggle ^= 0x200;
	}
	
	crypted += 512;
	data = malloc(sizeof(*data) * size);
	
	src_end = crypted_size - (crypted - begin_of_raw_data);
	dst_end = size;
	
	while(src_ptr < src_end && dst_ptr < dst_end)
	{
		uint32_t nentry = 0;
		uint16_t info;
		
		for(; nodes[nentry].has_childs; nbits--, bits = (bits << 1) & 0xFF)
		{
			if(!nbits)
			{
				nbits = 8;
				bits = (uint8_t)crypted[src_ptr++];
			}
			nentry = nodes[nentry].childs[(bits >> 7) & 1];
		}
		
		info = myLOWORD(nodes[nentry].leaf_value);
		
		if(myHIBYTE(info) == 1)
		{
			uint32_t cvalue = bits >> (8 - nbits);
			uint32_t nbits2 = nbits;
			int32_t offset;
			uint32_t ring_ptr, count;
			if(nbits < 12)
			{
				uint32_t bytes = ((11 - nbits) >> 3) + 1;
				nbits2 = nbits;
				while (bytes--)
				{
					cvalue = (uint8_t)crypted[src_ptr++] + (cvalue << 8);
					nbits2 += 8;
				}
			}
			nbits = nbits2 - 12;
			bits = myLOBYTE(cvalue << (8 - (nbits2 - 12)));
			
			offset = ((uint32_t)cvalue >> (nbits2 - 12)) + 2;
			ring_ptr = dst_ptr - offset;
			count = myLOBYTE(info) + 2;
			
			while (count--)
			{
				uint8_t tmp = data[ring_ptr++];
				data[dst_ptr++] = tmp;
			}
		}
		else
		{
			data[dst_ptr++] = myLOBYTE(info);
		}
	}
	
	free(nodes);
	
	return data;
}

int dsc_is_image(uint8_t * data)
{
	/*
	header is 16b:
	- width 2b
	- height 2b
	- bpp 1b
	- 11 zeros 1b
	*/
	uint8_t * ptr = data;
	uint16_t width, height;
	uint8_t bpp;
	int i;
	
	width = read16(&ptr);
	if(width == 0 || width > 8096)
		goto not_an_image;
	
	height = read16(&ptr);
	if(height == 0 || height > 8096)
		goto not_an_image;
	
	bpp = read8(&ptr);
	if(bpp != 8 && bpp != 24 && bpp != 32)
		goto not_an_image;
	
	for(i = 0;i < 11;i++)
	{
		uint8_t blank = read8(&ptr);
		if(blank != 0)
			goto not_an_image;
	}
	
	return 1;
	
not_an_image:
	return 0;
}

int dsc_save(uint8_t * data, uint32_t size, const char * filename)
{
	int ret = 1;
	/* avoid segfault in case data is less than 16 bits */
	if(size > 15 && dsc_is_image(data))
	{
		int y;
		char file_name[1024];
		uint16_t width = read16(&data);
		uint16_t height = read16(&data);
		uint8_t bpp = read8(&data);
		uint8_t * pixels = malloc(sizeof(*pixels) * width * height * 4);
		uint8_t * pixels_ptr = pixels;
		data += 11;
		
		for(y = 0;y < height;y++)
		{
			int x;
			for(x = 0;x < width;x++)
			{
				uint8_t a = 255;
				uint8_t b = read8(&data);
				uint8_t g = read8(&data);
				uint8_t r = read8(&data);
				if(bpp == 32)
					a = read8(&data);
				
				*pixels_ptr = r; pixels_ptr++;
				*pixels_ptr = g; pixels_ptr++;
				*pixels_ptr = b; pixels_ptr++;
				*pixels_ptr = a; pixels_ptr++;
			}
		}
		
		sprintf(file_name, "%s.png", filename);
		ret = write_RGBA_to_png(width, height, pixels, file_name);
		
		free(pixels);
	}
	else
	{
		FILE * f = fopen(filename, "wb");
		fwrite(data, size, 1, f);
		fclose(f);
	}
	
	return ret;
}

int buffer_sorting(const void * a, const void * b)
{
	const uint32_t * qa = a;
	const uint32_t * qb = b;
	return *qa - *qb;
}

