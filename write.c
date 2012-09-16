#include "write.h"
#include <stdlib.h>
#include <png.h>

#define PNGSETJMP 	if(setjmp(png_jmpbuf(png_ptr))) \
	{ \
		png_destroy_write_struct(&png_ptr, &info_ptr); \
		return 0; \
	}

int write_RGBA_to_png(uint16_t width, uint16_t height, uint8_t * array, const char * filename)
{
	int x, y;
	png_structp png_ptr;
	png_infop info_ptr;
	png_bytep * row_pointers;
	FILE * fp = fopen(filename, "wb");
	
	if(fp == NULL)
		return 0;
	
	png_ptr = png_create_write_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
	info_ptr = png_create_info_struct(png_ptr);
	
	PNGSETJMP
	
	png_init_io(png_ptr, fp);
	
	PNGSETJMP
	
	png_set_IHDR(png_ptr, info_ptr, width, height,
				 8, PNG_COLOR_TYPE_RGB_ALPHA, PNG_INTERLACE_NONE,
				 PNG_COMPRESSION_TYPE_BASE, PNG_FILTER_TYPE_BASE);
	
	png_write_info(png_ptr, info_ptr);
	
	/* write bytes */
	PNGSETJMP
	
	/* convert uint8_t[] to pngbyte[][] */
	row_pointers = malloc(sizeof(*row_pointers) * height);
	for(y = 0;y < height;y++)
	{
		row_pointers[y] = malloc(png_get_rowbytes(png_ptr, info_ptr));
		for(x = 0;x < width * 4;x += 4)
		{
			row_pointers[y][x] = array[y * width * 4 + x];
			row_pointers[y][x+1] = array[y * width * 4 + x+1];
			row_pointers[y][x+2] = array[y * width * 4 + x+2];
			row_pointers[y][x+3] = array[y * width * 4 + x+3];
		}
	}
	
	png_write_image(png_ptr, row_pointers);
	
	/* end write */
	PNGSETJMP
	
	png_write_end(png_ptr, NULL);
	png_destroy_write_struct(&png_ptr, &info_ptr);
	
	for(y = 0;y < height;y++)
	{
		free(row_pointers[y]);
	}
	free(row_pointers);
	
	fclose(fp);
	
	return 1;
}

