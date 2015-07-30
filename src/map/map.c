#include <kvm.h>
#include <ctf/ctf.h>
#include <string.h>
#include <m_list.h>

#include "tak.h"
#include "find.h"
#include "size/size.h"
#include "map/map.h"

static ctf_type
solve_typedef_chain(ctf_type type)
{
	ctf_type follow;
	ctf_typedef _typedef;
	ctf_kind kind;

	follow = type;
	while (1)
	{
		ctf_type_get_kind(follow, &kind);
		if (kind == CTF_KIND_TYPEDEF) {
			ctf_typedef_init(follow, &_typedef);
			ctf_typedef_get_type(_typedef, &follow);
			if (follow == NULL)
				break;
		}
		else
			break;
	}

	return follow;
}

static int
compare_data_object_name(void* data_object, void* key)
{
	char* name;

	ctf_data_object_get_name(data_object, &name);
	return strcmp(name, (char*)key) == 0;
}

int 
map_type(struct map_arg* arg)
{
	ctf_kind local_type_kind;
	ctf_kind target_type_kind;
	ctf_polycall map_fns[] = {
		map_noop,
		map_int,
		map_noop,
		map_pointer,
		map_array,
		map_noop,
		map_struct,
		map_noop,
		map_noop,
		map_noop,
		map_noop,
		map_noop,
		map_noop,
		map_noop
	};

	ctf_type_get_kind(arg->local_type, &local_type_kind);
	if (local_type_kind == CTF_KIND_TYPEDEF)
		arg->local_type = solve_typedef_chain(arg->local_type);

	ctf_type_get_kind(arg->target_type, &target_type_kind);
	if (target_type_kind == CTF_KIND_TYPEDEF)
		arg->target_type = solve_typedef_chain(arg->target_type);

	ctf_type_get_kind(arg->local_type, &local_type_kind);
	ctf_type_get_kind(arg->target_type, &target_type_kind);
	if (local_type_kind != target_type_kind) {
		printf("Tye mismatch: %s vs %s\n", ctf_kind_to_string(local_type_kind),
		  ctf_kind_to_string(target_type_kind));
		return TAK_E_NO_TYPE_MATCH;
	}

	printf("Mapping:\n  local kind: %d\n  target kind: %d\n",
		arg->target_type->kind,
		arg->local_type->kind);

	ctf_type_polycall(arg->local_type, arg, map_fns);
	return CTF_OK;
}

int
tak_map_sym(struct tak* t,
            const char* local_type_name,
            const char* symbol_name,
            void** output)
{
	ctf_data_object symbol;
	ctf_type local_type;
	ctf_type symbol_type;
	int find_status;
	struct map_arg arg;

	/* cache the object names? m_trie tak->symbol_cache */
	uint64_t do_count;
	m_list_length(&t->target_ctf->data_objects, &do_count);

	find_status = m_list_find(&t->target_ctf->data_objects,
	                          compare_data_object_name,
	                          (char*)symbol_name,
	                          (void**)&symbol);

	if (find_status == M_LIST_FALSE)
		return TAK_E_SYMBOL;
	
	if (find_status != M_LIST_OK)
		return TAK_E_TARGET_CTF;

	ctf_data_object_get_type(symbol, &symbol_type);

	/* cache the type names? m_trie tak->type_cache*/
	local_type = find_type(t->local_ctf, local_type_name);
	if (local_type == NULL)
		return TAK_E_NO_TYPE;

	arg.addr = symbol->value;
	arg.t = t;
	arg.output = output;
	arg.local_type = local_type;
	arg.target_type = symbol_type;

	printf("Starting mapping\n");
	map_type(&arg);
	return TAK_OK;
}

