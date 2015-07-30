#ifndef TAK_SIZE_H
#define TAK_SIZE_H

#include <ctf/ctf.h>
#include <stdlib.h>

int size_array(ctf_type type, void* arg);
int size_int(ctf_type type, void* arg);
int size_noop(ctf_type type, void* arg);
int size_pointer(ctf_type type, void* arg);
int size_struct(ctf_type type, void* arg);
int size_typedef(ctf_type type, void* arg);

/**
 * Obtain the size of a type.
 * 
 * The result of this function should be identical to a sizeof() operator.
 *
 * @param type type
 * @return size of the type in bytes 
 */
size_t size_type(ctf_type type);

#endif

