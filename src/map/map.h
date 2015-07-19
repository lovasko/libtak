#ifndef TAK_MAP_H
#define TAK_MAP_H

#include <ctf/ctf.h>

#include "tak.h"

struct map_arg
{
	ctf_type local_type;
	ctf_type target_type;
	unsigned long addr;
	struct tak* t;
	void** output;
	int need_alloc;
};

int map_noop(ctf_type type, void* _arg);
int map_int(ctf_type type, void* _arg);
int map_pointer(ctf_type type, void* _arg);
int map_struct(ctf_type type, void* _arg);
int map_type(struct map_arg* arg);

#endif

