#include "size/size.h"

int
size_pointer(ctf_type type, void* arg)
{
	(void) type;

	*((size_t*)arg) = sizeof(intptr_t);
	return CTF_OK;
}

