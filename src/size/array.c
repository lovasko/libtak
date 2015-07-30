#include "size/size.h"

int
size_array(ctf_type type, void* arg)
{
	ctf_array array;
	ctf_array_length array_length;
	ctf_type array_type;
	size_t array_type_size;

	ctf_array_init(type, &array);
	ctf_array_get_length(array, &array_length);
	ctf_array_get_content_type(array, &array_type);
	array_type_size = size_type(array_type);

	*((size_t*)arg) = array_length * array_type_size;
	return CTF_OK;
}

