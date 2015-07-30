#include "size/size.h"

int
size_typedef(ctf_type type, void* arg)
{
	ctf_typedef _typedef;
	ctf_type ref_type;
	size_t size;

	printf("measuring a typedef\n");
	ctf_typedef_init(type, &_typedef);
	ctf_typedef_get_type(_typedef, &ref_type);

	size = size_type(ref_type);
	*((size_t*)arg) = size;

	return CTF_OK;
}

