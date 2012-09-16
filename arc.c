#include "arc.h"
#include <stdio.h>
#include <stdlib.h>

struct File
{
	char name[16];
	uint32_t offset;
	uint32_t size;
};

struct Arc
{
	FILE * file;
	uint32_t data;
	uint32_t count;
	struct File * files;
};

/* helper functions */
static uint32_t read32(FILE * file);

struct Arc * arc_open(const char * filename)
{
	uint32_t magic[2], spaaaaaaaaaaaaaaace, numberOfFiles;
	int i;
	struct File * files = NULL;
	struct Arc * arc = NULL;
	FILE * file = fopen(filename, "rb");
	if(file == NULL)
		goto file_not_found;
	
	/* check if a valid ARC file */
	magic[0] = read32(file);
	magic[1] = read32(file);
	if(magic[0] != 0x6b636150 || magic[1] != 0x656c6946)
		goto close_file;
	
	spaaaaaaaaaaaaaaace = read32(file);
	if(spaaaaaaaaaaaaaaace != 0x20202020)
		goto close_file;
	
	numberOfFiles = read32(file);
	files = malloc(sizeof(*files) * numberOfFiles);
	if(files == NULL)
		goto close_file;
	
	arc = malloc(sizeof(*arc));
	if(arc == NULL)
		goto clean_files_memory;
	
	for(i = 0;i < numberOfFiles;i++)
	{
		int j;
		struct File f;
		fread(&f.name, 1, 16, file);
		f.offset = read32(file);
		f.size = read32(file);
		
		/* remove non ascii bytes */
		for(j = 0;j < 16;j++)
		{
			if(f.name[j] != 0 && (f.name[j] < 32 || f.name[j] > 127))
				f.name[j] = '_';
		}
		
		/* padding */
		read32(file);
		read32(file);
		
		files[i] = f;
	}
	
	arc->file = file;
	arc->data = ftell(file);
	arc->count = numberOfFiles;
	arc->files = files;
	
	return arc;

clean_files_memory:
	free(files);
close_file:
	fclose(file);
file_not_found:
	return NULL;
}

void arc_close(struct Arc * arc)
{
	if(arc != NULL)
	{
		fclose(arc->file);
		free(arc->files);
		free(arc);
	}
}

uint32_t arc_files_count(struct Arc * arc)
{
	return arc->count;
}

uint8_t * arc_get_file_data(struct Arc * arc, uint32_t idx)
{
	uint8_t * data = NULL;
	struct File * f = NULL;
	
	if(arc == NULL || arc->count <= idx)
		return NULL;
	
	f = &arc->files[idx];
	data = malloc(sizeof(*data) * f->size);
	if(data == NULL)
		return NULL;
	
	fseek(arc->file, arc->data + f->offset, SEEK_SET);
	fread(data, f->size, 1, arc->file);
	
	return data;
}

uint32_t arc_get_file_size(struct Arc * arc, uint32_t idx)
{
	if(arc == NULL || arc->count <= idx)
		return 0;
	return arc->files[idx].size;
}

char * arc_get_file_name(struct Arc * arc, uint32_t idx)
{
	if(arc == NULL || arc->count <= idx)
		return 0;
	return arc->files[idx].name;
}

/* helpers */
uint32_t read32(FILE * file)
{
	uint32_t i = 0;
	fread(&i, 1, 4, file);
	return i;
}

