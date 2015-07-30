#include "size/size.h"

size_t
size_type(ctf_type type)
{
	size_t result;
	ctf_kind kind;
	ctf_polycall size_fns[] = {
		size_noop,
		size_int,
		size_noop,
		size_pointer,
		size_array,
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

	ctf_type_get_kind(type, &kind);
	printf("Measuring type of a kind %s\n", ctf_kind_to_string(kind));
	getchar();

	ctf_type_polycall(type, &result, size_fns);
	printf("RESULT: %u\n", result);
	return result;
}

