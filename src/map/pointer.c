#include "map/map.h"
#include "size.h"

int
map_pointer(ctf_type type, void* _arg)
{
	ctf_pointer local_pointer;
	ctf_pointer target_pointer;
	ctf_type local_ref_type;
	ctf_type target_ref_type;
	size_t local_ref_type_size;
	struct map_arg *in_arg;
	struct map_arg out_arg;
	void* storage;
	intmax_t target_pointer_value;

	(void)type;
	in_arg = _arg;

	kvm_read(in_arg->t->target_kvm,
	         in_arg->addr,
	         &target_pointer_value,
	         sizeof(intmax_t));

	printf("target_pointer_value %#llx\n", target_pointer_value);
	if (target_pointer_value == 0) {
		*in_arg->output = NULL;
		return CTF_OK;
	}

	ctf_pointer_init(in_arg->local_type, &local_pointer);
	ctf_pointer_get_type(local_pointer, &local_ref_type);

	ctf_pointer_init(in_arg->target_type, &target_pointer);
	ctf_pointer_get_type(target_pointer, &target_ref_type);

	local_ref_type_size = size_type(local_ref_type);

	storage = malloc(local_ref_type_size);
	printf("Allocating %u bytes at address %p\n", 
		local_ref_type_size, storage);

	*in_arg->output = storage;
	printf("%p now contains %p\n", in_arg->output, *in_arg->output);

	out_arg.local_type = local_ref_type;
	out_arg.target_type = target_ref_type;
	out_arg.addr = target_pointer_value;
	out_arg.t = in_arg->t;
	out_arg.output = &storage;
	out_arg.need_alloc = 0;

	map_type(&out_arg);

	return CTF_OK;
}

