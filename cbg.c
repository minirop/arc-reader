#include "cbg.h"
#include "decrypt.h"
#include "write.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>

struct NodeCBG
{
	uint32_t vv[6];
};

static uint32_t readVariable(uint8_t ** ptr);
static uint32_t color_add(uint32_t x, uint32_t y);
static uint32_t color_avg(uint32_t x, uint32_t y);
static uint32_t extract(uint8_t ** src, uint32_t bpp);
static int method2(uint32_t table1[256], struct NodeCBG table2[511]);

int cbg_is_valid(uint8_t * data, uint32_t size)
{
	if(size < 48) return 0;
	
	return (memcmp((char*)data, "CompressedBG___", 15) == 0);
}

uint8_t * cbg_decrypt(uint8_t * crypted, uint16_t * pwidth, uint16_t * pheight)
{
	uint16_t width, height;
	uint32_t bpp, data1_len, data0_val, data0_len;
	uint8_t sum_check, xor_check;
	uint32_t table[256];
	struct NodeCBG table2[511];
	uint32_t n;
	uint8_t sum_data = 0;
	uint8_t xor_data = 0;
	uint8_t * ptr;
	int method2_res;
	uint8_t *data1, *data3, *psrc, *pdst;
	uint32_t mask = 0x80;
	int type = 0;
	uint32_t * data;
	int aa;
	uint8_t * src;
	uint32_t * dst;
	uint32_t c = 0;
	uint16_t x, y;
	uint8_t *pixels, *pixels_ptr;
	uint32_t px;
	uint8_t * data0;
	
	crypted += 16;
	width     = read16(&crypted);
	height    = read16(&crypted);
	bpp       = read32(&crypted);
	            read32(&crypted);
	            read32(&crypted);
	data1_len = read32(&crypted);
	data0_val = read32(&crypted);
	data0_len = read32(&crypted);
	sum_check = read8(&crypted);
	xor_check = read8(&crypted);
	/*uint8_t unknown =*/ read16(&crypted);
	*pwidth  = width;
	*pheight = height;
	
	data0 = malloc(sizeof(*data0) * data0_len);
	memcpy(data0, crypted, data0_len);
	crypted += data0_len;
	
	for(n = 0; n < data0_len; n++)
	{
		data0[n] -= hash_update(&data0_val) & 0xFF;
		sum_data += data0[n];
		xor_data ^= data0[n];
	}
	
	if(sum_data != sum_check || xor_data != xor_check)
	{
		free(data0);
		return NULL;
	}
	
	ptr = data0;
	for(n = 0; n < 256; n++)
	{
		table[n] = readVariable(&ptr);
	}
	free(data0);
	
	method2_res = method2(table, table2);
	data1 = malloc(sizeof(*data1) * data1_len);
	
	for(n = 0; n < data1_len; n++)
	{
		uint32_t cvalue = method2_res;

		if(table2[method2_res].vv[2] == 1)
		{
			do
			{
				int bit = !!(*crypted & mask);
				mask >>= 1;

				cvalue = table2[cvalue].vv[4 + bit];

				if (!mask)
				{
					crypted++;
					mask = 0x80;
				}
			} while(table2[cvalue].vv[2] == 1);
		}

		data1[n] = cvalue;
	}
	
	data3 = malloc(sizeof(*data3) * width * height * 4);
	psrc = data1;
	pdst = data3;
	while(psrc < data1 + data1_len)
	{
		uint32_t len = readVariable(&psrc);
		uint32_t i;
		if(type)
		{
			for(i = 0;i < len;i++)
			{
				pdst[i] = 0;
			}
		}
		else
		{
			for(i = 0;i < len;i++)
			{
				pdst[i] = psrc[i];
			}
			psrc += len;
		}
		pdst += len;
		type = !type;
	}
	free(data1);
	
	data = malloc(sizeof(*data) * width * height);
	for(aa = 0;aa < width * height;aa++)
	{
		data[aa] = 0x00;
	}
	
	src = data3;
	dst = data;
	
	for(x = 0;x < width;x++)
	{
		c = color_add(c, extract(&src, bpp));
		*dst = c;
		dst++;
	}
	for(y = 1;y < height;y++)
	{
		c = color_add((*(dst - width)), extract(&src, bpp));
		*dst = c;
		dst++;
		for(x = 1;x < width;x++)
		{
			uint32_t moy = color_avg(c, (*(dst - width)));
			c = color_add(moy, extract(&src, bpp));
			*dst = c;
			dst++;
		}
	}
	free(data3);
	
	pixels = malloc(sizeof(*pixels) * width*height*4);
	pixels_ptr = pixels;
	for(px = 0;px < width * height; px++)
	{
		uint8_t r, g, b, a;
		if(bpp == 32)
		{
			a = (data[px] >> 24) & 0xFF;
			r = (data[px] >> 16) & 0xFF;
			g = (data[px] >> 8) & 0xFF;
			b = data[px] & 0xFF;
		}
		else
		{
			b = (data[px] >> 16) & 0xFF;
			g = (data[px] >> 8) & 0xFF;
			r = data[px] & 0xFF;
			a = 0xFF;
		}
		
		*pixels_ptr = r; pixels_ptr++;
		*pixels_ptr = g; pixels_ptr++;
		*pixels_ptr = b; pixels_ptr++;
		*pixels_ptr = a; pixels_ptr++;
	}
	
	free(data);
	
	return pixels;
}

int cbg_save(uint8_t * data, uint32_t width, uint32_t height, const char * filename)
{
	char file_name[1024];
	sprintf(file_name, "%s.png", filename);
	return write_RGBA_to_png(width, height, data, file_name);
}

uint32_t readVariable(uint8_t ** ptr)
{
	uint8_t c;
	uint32_t v = 0;
	int32_t shift = 0;
	do
	{
		c = **ptr;
		(*ptr)++;
		v |= (c & 0x7F) << shift;
		shift += 7;
	} while (c & 0x80);
	return v;
}

uint32_t color_avg(uint32_t x, uint32_t y)
{
	uint32_t a = (((x & 0xFF000000) / 2) + ((y & 0xFF000000) / 2)) & 0xFF000000;
	uint32_t r = (((x & 0x00FF0000) + (y & 0x00FF0000)) / 2) & 0x00FF0000;
	uint32_t g = (((x & 0x0000FF00) + (y & 0x0000FF00)) / 2) & 0x0000FF00;
	uint32_t b = (((x & 0x000000FF) + (y & 0x000000FF)) / 2) & 0x000000FF;
	return (a | r | g | b);
}

uint32_t color_add(uint32_t x, uint32_t y)
{
	uint32_t a = ((x & 0xFF000000) + (y & 0xFF000000)) & 0xFF000000;
	uint32_t r = ((x & 0x00FF0000) + (y & 0x00FF0000)) & 0x00FF0000;
	uint32_t g = ((x & 0x0000FF00) + (y & 0x0000FF00)) & 0x0000FF00;
	uint32_t b = ((x & 0x000000FF) + (y & 0x000000FF)) & 0x000000FF;
	
	return (a | r | g | b);
}

uint32_t extract(uint8_t ** src, uint32_t bpp)
{
	if(bpp == 32)
	{
		return read32(src);
	}
	else
	{
		uint8_t r = read8(src), g = r, b = r;
		if(bpp == 24)
		{
			g = read8(src);
			b = read8(src);
		}
		return (0xff000000 | r << 16 | g << 8 | b);
	}
}

int method2(uint32_t table1[256], struct NodeCBG table2[511])
{
	uint32_t sum_of_values = 0;
	struct NodeCBG node;
	uint32_t n;
	uint32_t cnodes = 256;
	uint32_t vinfo[2];
	
	for(n = 0;n < 256;n++)
	{
		table2[n].vv[0] = table1[n] > 0;
		table2[n].vv[1] = table1[n];
		table2[n].vv[2] = 0;
		table2[n].vv[3] =-1;
		table2[n].vv[4] = n;
		table2[n].vv[5] = n;
		sum_of_values += table1[n];
	}
	
	node.vv[0] = 0;
	node.vv[1] = 0;
	node.vv[2] = 1;
	node.vv[3] =-1;
	node.vv[4] =-1;
	node.vv[5] =-1;
	
	for(n = 0; n < 255; n++)
		table2[256 + n] = node;

	while(1)
	{
		uint32_t m;
		for(m = 0; m < 2; m++)
		{
			uint32_t min_value = 0xFFFFFFFF;
			vinfo[m] = UINT_MAX;
			
			for(n = 0; n < cnodes; n++)
			{
				struct NodeCBG * cnode = &table2[n];

				if(cnode->vv[0] && (cnode->vv[1] < min_value))
				{
					vinfo[m] = n;
					min_value = cnode->vv[1];
				}
			}

			if(vinfo[m] != UINT_MAX)
			{
				table2[vinfo[m]].vv[0] = 0;
				table2[vinfo[m]].vv[3] = cnodes;
			}
		}

		node.vv[0] = 1;
		node.vv[1] = ((vinfo[1] != 0xFFFFFFFF) ? table2[vinfo[1]].vv[1] : 0) + table2[vinfo[0]].vv[1];
		node.vv[2] = 1;
		node.vv[3] =-1;
		node.vv[4] = vinfo[0];
		node.vv[5] = vinfo[1];

		table2[cnodes++] = node;

		if(node.vv[1] == sum_of_values)
			break;
	}

	return cnodes - 1;
}

