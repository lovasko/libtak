#include "size/size.h"

int
size_noop(ctf_type type, void* arg)
{
	(void)type;
	(void)arg;

	return CTF_OK;
}

