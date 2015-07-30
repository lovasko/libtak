#include "size/size.h"

int
size_int(ctf_type type, void* arg)
{
	ctf_int _int;
	ctf_int_size size;

	ctf_int_init(type, &_int);
	ctf_int_get_size(_int, &size);

	*((size_t*)arg) = (size+7)/8;
	printf("Measuring integer: %u\n", *((size_t*)arg));
	return CTF_OK;
}

