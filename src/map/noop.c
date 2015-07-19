#include "map/map.h"

int
map_noop(ctf_type type, void* _arg)
{
	(void)type;
	(void)_arg;

	return CTF_OK;
}

