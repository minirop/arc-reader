#ifndef ARC_ETHORNELL_H
#define ARC_ETHORNELL_H

#include <stdint.h>

struct Arc;

/**
 * open the given file and return NULL in case of an error
 */
struct Arc * arc_open(const char * filename);

/**
 * close the file and free its memory
 */
void arc_close(struct Arc * arc);

/**
 * return the number of files contained in the arc file
 */
uint32_t arc_files_count(struct Arc * arc);

/**
 * get the raw data of the file at position "idx"
 * WARNING: make a deep copy of the data, you have to
 *          free the memory yourself with free()
 */
uint8_t * arc_get_file_data(struct Arc * arc, uint32_t idx);

/**
 * get the (maybe compressed) size of the file at position "idx"
 */
uint32_t arc_get_file_size(struct Arc * arc, uint32_t idx);

/**
 * get the name of the file at position "idx"
 */
char * arc_get_file_name(struct Arc * arc, uint32_t idx);

#endif

