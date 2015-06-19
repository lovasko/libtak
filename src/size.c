#include "size.h"

static int
size_noop(ctf_type type, void* arg)
{
	(void)type;
	(void)arg;

	return CTF_OK;
}

static void
size_member(void* member, void* arg)
{
	ctf_type type;
	size_t size;

	printf("measuring struct member\n");
	ctf_member_get_type(member, &type);
	size = size_type(type);
	*((size_t*)arg) += size;
}

static int
size_struct(ctf_type type, void* arg)
{
	ctf_struct_union _struct;
	
	printf("measuring struct\n");
	ctf_struct_union_init(type, &_struct);
	*((size_t*)arg) = 0;
	ctf_struct_union_foreach_member(_struct, arg, size_member);

	return CTF_OK;
}

static int
size_int(ctf_type type, void* arg)
{
	ctf_int _int;
	ctf_int_size size;

	ctf_int_init(type, &_int);
	ctf_int_get_size(_int, &size);

	*((size_t*)arg) = (size+7)/8;
	printf("measuring integer: %u %d\n", *((size_t*)arg), (size+7)/8);
	return CTF_OK;
}

static int
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

size_t
size_type(ctf_type type)
{
	size_t result;

	ctf_polycall size_fns[] = {
		size_noop,
		size_int,
		size_noop,
		size_noop,
		size_noop,
		size_noop,
		size_struct,
		size_noop,
		size_noop,
		size_noop,
		size_typedef,
		size_noop,
		size_noop,
		size_noop
	};

	ctf_kind kind;
	ctf_type_get_kind(type, &kind);
	printf("got to measure type of a kind %s\n", ctf_kind_to_string(kind));

	ctf_type_polycall(type, &result, size_fns);
	return result;
}

