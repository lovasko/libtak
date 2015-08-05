#include <string.h>

#include "map/map.h"
#include "size/size.h"

static void
print_key(void* key, void* payload)
{
	intptr_t pointer;

	(void)payload;

	memcpy(&pointer, key, sizeof(intptr_t));
	printf("%#x\n", pointer);
}

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
	intptr_t storage_int;
	intptr_t target_pointer_value;
	char ptr_str[sizeof(intptr_t)+1];
	intptr_t* saved_output;
	unsigned int i;
	int trie_retval;

	(void)type;
	in_arg = _arg;

	kvm_read(in_arg->t->target_kvm,
	         in_arg->addr,
	         &target_pointer_value,
	         sizeof(intptr_t));

	printf("target_pointer_value %p\n", target_pointer_value);
	if (target_pointer_value == 0) {
		in_arg->output = NULL;
		return CTF_OK;
	}

	memset(ptr_str, '\0', sizeof(intptr_t)+1);
	printf("a\n");

	for (i = 0; i < sizeof(intptr_t); i++) {
		printf("i: %d\n", i);
		ptr_str[i] = ((char*)&target_pointer_value)[i];
	}

	printf("a\n");
	printf("before get: %#x %#x %#x %#x\n", ptr_str[0], ptr_str[1], ptr_str[2], ptr_str[3]);
	trie_retval = m_trie_get(in_arg->pointer_db,
	                         ptr_str,
													 sizeof(intptr_t),
	                         (void**)&saved_output);

	if (trie_retval == M_TRIE_OK) {
		printf("Pointer DB hitting success!\n");
		*(intptr_t*)(*(in_arg->output)) = *saved_output;
		return CTF_OK;
	} else
		printf("Not in pointer database. %s\n", m_trie_error_string(trie_retval));

	ctf_pointer_init(in_arg->local_type, &local_pointer);
	ctf_pointer_get_type(local_pointer, &local_ref_type);

	ctf_pointer_init(in_arg->target_type, &target_pointer);
	ctf_pointer_get_type(target_pointer, &target_ref_type);

	local_ref_type_size = size_type(local_ref_type);

	storage = malloc(local_ref_type_size);
	printf("Allocating %u bytes at address %p\n", 
		local_ref_type_size, storage);

	*(intptr_t*)(*(in_arg->output)) = (intptr_t)storage;
	
	storage_int = (intptr_t)storage;

	printf("storing %d\n", storage_int);
	printf("before set: %#x %#x %#x %#x\n", ptr_str[0], ptr_str[1], ptr_str[2], ptr_str[3]);
	trie_retval = m_trie_set(in_arg->pointer_db,
                           ptr_str,
                           sizeof(intptr_t),
                           M_TRIE_COPY_DEEP,
                           M_TRIE_OVERWRITE_PREVENT,
                           &storage_int,
                           sizeof(intptr_t));

	if (trie_retval != M_TRIE_OK)
		printf("TRIE NOT OK: %s\n", m_trie_error_string(trie_retval));

	printf("printing keys! %lld\n", in_arg->pointer_db->keys.length);
	m_list_map(&in_arg->pointer_db->keys, print_key, NULL);

	out_arg.local_type = local_ref_type;
	out_arg.target_type = target_ref_type;
	out_arg.addr = target_pointer_value;
	out_arg.t = in_arg->t;
	out_arg.output = &storage;
	out_arg.pointer_db = in_arg->pointer_db;

	map_type(&out_arg);

	return CTF_OK;
}

