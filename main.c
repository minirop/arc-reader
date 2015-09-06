#include <stdio.h>
#include <stdlib.h>

#include "arc.h"
#include "dsc.h"
#include "cbg.h"
#include "bse.h"

#ifdef _WIN32
#include <windows.h>
#define makedir(path) CreateDirectory(path, NULL);
#else
#include <sys/types.h>
#include <sys/stat.h>
#define makedir(path) mkdir(path, S_IRWXU | S_IRGRP | S_IXGRP | S_IROTH | S_IXOTH);
#endif

int main(int argc, char** argv)
{
	uint32_t i, count;
	char file_name_with_path[1024] = {0};
	struct Arc * arc = NULL;
	
	if(argc != 2 && argc != 3)
	{
		puts("Usage: a.exe <file.arc> [path]");
		return 1;
	}
	
	arc = arc_open(argv[1]);
	if(arc == NULL)
	{
		printf("can't read file: %s\n", argv[1]);
		return 1;
	}
	
	count = arc_files_count(arc);
	
	if(argc == 3)
		makedir(argv[2]);
	
	printf("number of file: %d\n", count);
	
	for(i = 0;i < count;i++)
	{
		uint8_t * data = NULL;
		uint8_t * raw_data = arc_get_file_data(arc, i);
		uint8_t * bse_data = raw_data;
		uint32_t filesize = arc_get_file_size(arc, i);
		int good = 1;
		
		if(argc == 3)
			sprintf(file_name_with_path, "%s/%s", argv[2], arc_get_file_name(arc, i));
		else
			sprintf(file_name_with_path, "%s", arc_get_file_name(arc, i));
		
		printf("%s...", arc_get_file_name(arc, i)); fflush(stdout);
		
		if(bse_is_valid(raw_data, filesize))
		{
			printf("BSE..."); fflush(stdout);
			if(bse_decrypt(raw_data))
			{
				bse_data = raw_data + 16;
			}
		}
		
		if(dsc_is_valid(bse_data, filesize))
		{
			uint32_t fsize;
			printf("DSC..."); fflush(stdout);
			
			data = dsc_decrypt(bse_data, filesize, &fsize);
			good = dsc_save(data, fsize, file_name_with_path);
		}
		else if(cbg_is_valid(bse_data, filesize))
		{
			uint16_t w, h;
			printf("CBG..."); fflush(stdout);
			
			data = cbg_decrypt(bse_data, &w, &h);
			good = cbg_save(data, w, h, file_name_with_path);
		}
		else
		{
			FILE * f = fopen(file_name_with_path, "wb");
			printf("uncompressed..."); fflush(stdout);
			fwrite(bse_data, filesize, 1, f);
			fclose(f);
		}
		
		if(good)
			puts("ok");
		else
			puts("ERROR");
		
		if(raw_data != NULL)
			free(raw_data);
		if(data != NULL)
			free(data);
	}
	
	arc_close(arc);
	return 0;
}
