#include "map/map.h"
#include "size.h"

static int
compare_member_name(void* member, void* key)
{
	char* name;

	ctf_member_get_name(member, &name);
	return strcmp(name, (char*)key) == 0;
}

int
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

	printf("  Mapping a struct\n");

	arg = _arg;
	local_su_size = size_type(arg->local_type);
	ctf_struct_union_init(arg->local_type, &local_su);
	ctf_struct_union_init(arg->target_type, &target_su);

	/* arg->output = malloc(local_su_size); */
	/* printf("Allocated %u bytes at address %p for 'struct %s'\n", local_su_size, arg->output, local_su->name); */

	m_list_first(&local_su->members, &member_runner);
	while (member_runner != NULL) {
		m_elem_data(member_runner, (void**)&local_member);
		ctf_member_get_name(local_member, &local_member_name);
		printf("Examining member '%s' ... ", local_member_name);

		find_status = m_list_find(&target_su->members,
		                          compare_member_name,
		                          local_member_name,
		                          (void**)&target_member);

		if (find_status == M_LIST_OK) {
			printf("pairing exists\n");
			member_arg.target_type = target_member->type;
			member_arg.local_type = local_member->type;
			member_arg.t = arg->t;
			member_arg.addr = arg->addr + (target_member->offset/8);

			member_addr = (char*)(*arg->output);
			printf("member_addr = %p\n", member_addr);
			member_addr += local_member->offset/8;
			printf("member_addr = %p\n", member_addr);
			member_arg.output = (void**)&member_addr;
			map_type(&member_arg);
		} else
			printf("No '%s' member in target!\n", local_member_name);

		m_elem_next(member_runner, &member_runner);
	}

	return CTF_OK;
}

