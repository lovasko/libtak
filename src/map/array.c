#include "map/map.h"
#include "size/size.h"

int
map_array(ctf_type type, void* _arg)
{
	struct map_arg* in_arg;
	struct map_arg out_arg;
	ctf_array_length idx;
	char* output_addr;

	ctf_array local_array;
	ctf_type local_array_type;
	ctf_array_length local_array_length;
	size_t local_array_type_size;

	ctf_array target_array;
	ctf_type target_array_type;
	ctf_array_length target_array_length;
	size_t target_array_type_size;

	(void)type;
	in_arg = _arg;

	printf("Mapping an array!\n");

	ctf_array_init(in_arg->local_type, &local_array);
	ctf_array_init(in_arg->target_type, &target_array);

	ctf_array_get_length(local_array, &local_array_length);
	ctf_array_get_length(target_array, &target_array_length);

	if (local_array_length != target_array_length) {
		printf("Different array lengths!\n");
		return 1;
	}

	ctf_array_get_content_type(local_array, &local_array_type);
	ctf_array_get_content_type(target_array, &target_array_type);

	local_array_type_size = size_type(local_array_type);
	target_array_type_size = size_type(target_array_type);
		
	out_arg.local_type = local_array_type;
	out_arg.target_type = target_array_type;
	out_arg.addr = in_arg->addr;
	out_arg.output = in_arg->output;
	out_arg.t = in_arg->t;
	out_arg.pointer_db = in_arg->pointer_db;

	for (idx = 0; idx < local_array_length; idx++) {
		printf("Array idx %d\n", idx);
		map_type(&out_arg); 
		out_arg.addr += target_array_type_size;

		output_addr = (char*)(*out_arg.output);
		output_addr += local_array_type_size;
		out_arg.output = (void**)&output_addr;
	}

	return CTF_OK;
	/*
		get the local array length
		get the target array length
		compare lengths
			return error if not the same

		local array memory is already allocated
		for element in array.elements:
			map_type
			if (map_type) failed
				return failure too
			increment address
			increment output 
	*/
}

