#include <kvm.h>
#include <ctf/ctf.h>
#include <string.h>
#include <m_list.h>

#include "tak.h"
#include "find.h"
#include "size.h"

struct map_arg
{
	unsigned long addr;
	tak_t tak;
	void** output;
	ctf_type local_type;
	ctf_type target_type;
	int need_alloc;
};

static int map_type(struct map_arg* arg);

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

static int
map_noop(ctf_type type, void* arg)
{
	(void) type;
	(void) arg;

	return CTF_OK;
}

static int
map_int(ctf_type type, void* _arg)
{
	ctf_int_offset offset;
	ctf_int_size size;
	ctf_int_content content;
	ctf_int _int;
	unsigned int nbytes;
	intmax_t signed_int;
	uintmax_t unsigned_int;
	uint8_t is_signed;
	uint8_t sign;
	struct map_arg* arg;

	printf("mapping an integer!\n");

	arg = _arg;
	ctf_int_init(type, &_int);
	ctf_int_get_offset(_int, &offset);
	ctf_int_get_size(_int, &size);
	ctf_int_get_content(_int, &content);
	ctf_int_get_is_signed(_int, &is_signed);

	nbytes = (offset + size)/8;
	if (nbytes > sizeof(uintmax_t) || nbytes > sizeof(intmax_t))
		return 1;

	if (is_signed) {
		signed_int = 0;
		if (arg->tak->data_source == TAK_DATA_SOURCE_KVM) {
			kvm_read(arg->tak->target_kvm, arg->addr, &signed_int, sizeof(intmax_t));
		} else
			return TAK_E_DATA_SOURCE_UNKNOWN;

		signed_int &= ((intmax_t)1 << size) - (intmax_t)1;

		sign = signed_int & ((intmax_t)1 << size);
		if (sign)
			signed_int |= ((intmax_t)-1 << size);
		else
			signed_int &= (((intmax_t)1 << size) - (intmax_t)1);

		if (arg->need_alloc)
			*(arg->output) = malloc(sizeof(intmax_t));
		**((intmax_t**)arg->output) = signed_int;
	} else {

		if (arg->tak->data_source == TAK_DATA_SOURCE_KVM) {
			kvm_read(arg->tak->target_kvm, arg->addr, &unsigned_int, sizeof(uintmax_t));
		} else 
			return TAK_E_DATA_SOURCE_UNKNOWN;	

		unsigned_int &= (((uintmax_t)1 << size) - (uintmax_t)1);

		if (arg->need_alloc)
			*(arg->output) = malloc(sizeof(uintmax_t));
		*((uintmax_t*)arg->output) = unsigned_int;
	}

	return CTF_OK;
}

static int
compare_member_name(void* member, void* key)
{
	char* name;

	ctf_member_get_name(member, &name);
	return strcmp(name, (char*)key) == 0;
}

static int
map_struct(ctf_type type, void* _arg)
{
	ctf_member target_member;
	ctf_struct_union target_su;

	ctf_struct_union local_su;
	size_t local_su_size;
	ctf_member local_member;
	char* local_member_name;

	struct m_elem* member_runner;
	int find_status;
	struct map_arg member_arg;
	struct map_arg* arg;
	char* member_addr;

	(void)type;

	arg = _arg;
	ctf_struct_union_init(arg->local_type, &local_su);
	ctf_struct_union_init(arg->target_type, &target_su);

	local_su_size = size_type(arg->local_type);
	printf("the local SU size is %u bytes\n", local_su_size);

	*arg->output = malloc(local_su_size);

	m_list_first(&local_su->members, &member_runner);
	while (member_runner != NULL) {
		m_elem_data(member_runner, (void**)&local_member);
		ctf_member_get_name(local_member, &local_member_name);

		find_status = m_list_find(&target_su->members,
		                          compare_member_name,
		                          local_member_name,
		                          (void**)&target_member);

		if (find_status == M_LIST_OK) {
			member_arg.need_alloc = 0;
			member_arg.target_type = target_member->type;
			member_arg.local_type = local_member->type;
			member_arg.tak = arg->tak;
			member_arg.addr = arg->addr + (target_member->offset/8);

			member_addr = (char*)(*arg->output);
			member_addr += local_member->offset/8;
			member_arg.output = (void**)&member_addr;
			map_type(&member_arg);
		}

		m_elem_next(member_runner, &member_runner);
	}

	return CTF_OK;
}

int 
map_type(struct map_arg* arg)
{
	ctf_polycall map_fns[] = {
		map_noop,
		map_int,
		map_noop,
		map_noop,
		map_noop,
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

	ctf_type_polycall(arg->target_type, &arg, map_fns);
	return CTF_OK;
}

int
tak_map_sym(tak_t tak, char* local_type_name, char* symbol_name, void** output)
{
	ctf_data_object symbol;
	ctf_type local_type;
	ctf_type symbol_type;
	ctf_kind local_kind;
	ctf_kind symbol_kind;
	int find_status;
	struct map_arg arg;

	/* cache the object names? m_trie tak->symbol_cache */
	uint64_t do_count;
	m_list_length(&tak->target_ctf->data_objects, &do_count);

	find_status = m_list_find(&tak->target_ctf->data_objects,
	                          compare_data_object_name,
	                          symbol_name,
	                          (void**)&symbol);

	if (find_status == M_LIST_FALSE)
		return TAK_E_SYMBOL;
	
	if (find_status != M_LIST_OK)
		return TAK_E_TARGET_CTF;

	ctf_data_object_get_type(symbol, &symbol_type);

	/* cache the type names? m_trie tak->type_cache*/
	local_type = find_type(tak->local_ctf, local_type_name);
	if (local_type == NULL)
		return TAK_E_NO_TYPE;

	ctf_type_get_kind(local_type, &local_kind);
	if (local_kind == CTF_KIND_TYPEDEF)
		local_type = solve_typedef_chain(local_type);

	ctf_type_get_kind(symbol_type, &symbol_kind);
	if (symbol_kind == CTF_KIND_TYPEDEF)
		symbol_type = solve_typedef_chain(symbol_type);

	ctf_type_get_kind(local_type, &local_kind);
	ctf_type_get_kind(symbol_type, &symbol_kind);
	if (local_kind != symbol_kind)
		return TAK_E_NO_TYPE_MATCH;

	printf("local: %s\ntarget: %s\n",
	       ctf_kind_to_string(local_kind),
	       ctf_kind_to_string(symbol_kind));

	arg.addr = symbol->value;
	arg.tak = tak;
	arg.output = output;
	arg.local_type = local_type;
	arg.target_type = symbol_type;
	arg.need_alloc = 1;
	map_type(&arg);
	return TAK_OK;
}

