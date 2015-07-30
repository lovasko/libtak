#include "size/size.h"

static void
size_member(void* member, void* arg)
{
	ctf_type type;
	size_t size;

	printf("Measuring struct member\n");
	getchar();
	ctf_member_get_type(member, &type);
	size = size_type(type);
	*((size_t*)arg) += size;
}

int
size_struct(ctf_type type, void* arg)
{
	ctf_struct_union _struct;
	
	printf("Measuring struct\n");
	ctf_struct_union_init(type, &_struct);
	*((size_t*)arg) = 0;
	ctf_struct_union_foreach_member(_struct, arg, size_member);

	return CTF_OK;
}

