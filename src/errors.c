#include "tak.h"

char*
tak_error_string(int retval)
{
	static char* error_strings[] = {
		"OK",
		"Local CTF data set error",
		"Target CTF data set error",
		"No such symbol",
		"Types do not match",
		"No such type",
		"Unknown type info",
		"Unknown data source",
		"KVM error"
	};

	if (retval < 0 || retval > TAK_E_MAX)
		return NULL;
	else
		return error_strings[retval];
}

