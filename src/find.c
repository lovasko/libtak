#include <string.h>

#include "find.h"

struct type_arg
{
	char* name;
	ctf_type type;
};

static int 
compare_int_name(ctf_type type, void* arg)
{
	ctf_int _int;
	char* name;

	ctf_int_init(type, &_int);
	ctf_int_get_name(_int, &name);

	if (strcmp(name, ((struct type_arg*)arg)->name) == 0) 
		((struct type_arg*)arg)->type = type;

	return CTF_OK;
}

static int 
compare_float_name(ctf_type type, void* arg)
{
	ctf_float _float;
	char* name;

	ctf_float_init(type, &_float);
	ctf_float_get_name(_float, &name);

	if (strcmp(name, ((struct type_arg*)arg)->name) == 0) 
		((struct type_arg*)arg)->type = type;

	return CTF_OK;
}

static int 
compare_struct_union_name(ctf_type type, void* arg)
{
	ctf_struct_union struct_union;
	char* name;
	char* arg_name;

	ctf_struct_union_init(type, &struct_union);
	ctf_struct_union_get_name(struct_union, &name);

	arg_name = ((struct type_arg*)arg)->name;

	if (strncmp(arg_name, "struct ", 7) == 0)
		if (strcmp(name, arg_name+7) == 0)
			((struct type_arg*)arg)->type = type;

	if (strncmp(arg_name, "union ", 6) == 0)
		if (strcmp(name, arg_name+6) == 0)
			((struct type_arg*)arg)->type = type;

	return CTF_OK;
}

static int 
compare_enum_name(ctf_type type, void* arg)
{
	ctf_enum _enum;
	char* name;
	char* arg_name;

	ctf_enum_init(type, &_enum);
	ctf_enum_get_name(_enum, &name);

	arg_name = ((struct type_arg*)arg)->name;

	if (strncmp(arg_name, "enum ", 5) == 0)
		if (strcmp(name, arg_name+5) == 0)
			((struct type_arg*)arg)->type = type;

	return CTF_OK;
}

static int 
compare_typedef_name(ctf_type type, void* arg)
{
	ctf_typedef _typedef;
	char* name;

	ctf_typedef_init(type, &_typedef);
	ctf_typedef_get_name(_typedef, &name);

	if (strcmp(name, ((struct type_arg*)arg)->name) == 0) 
		((struct type_arg*)arg)->type = type;

	return CTF_OK;
}

static int 
compare_dummy(ctf_type type, void* arg)
{
	(void)type;
	(void)arg;

	return CTF_OK;
}

static void
compare_type_name(void* type, void* arg)
{
	ctf_polycall compare_fns[] = {
		compare_dummy,
		compare_int_name,
		compare_float_name,
		compare_dummy,
		compare_dummy,
		compare_dummy,
		compare_struct_union_name,
		compare_struct_union_name,
		compare_enum_name,
		compare_dummy,
		compare_typedef_name,
		compare_dummy,
		compare_dummy,
		compare_dummy
	};

	ctf_type_polycall(type, arg, compare_fns);
}

ctf_type
find_type(ctf_file file, char* name)
{
	struct type_arg arg;

	arg.name = (char*)name;
	arg.type = NULL;

	ctf_file_foreach_type(file, &arg, compare_type_name);
	return arg.type;	
}

