#include <sys/types.h>
#include <sys/sysctl.h>

#include <ctf/ctf.h>
#include <fcntl.h>
#include <kvm.h>
#include <string.h>
#include <paths.h>

#include "tak.h"

static char*
get_current_executable_path(void)
{
	int mib[4];
	char buf[1024];
	size_t cb;
	
	cb = sizeof(buf);

	mib[0] = CTL_KERN;
	mib[1] = KERN_PROC;
	mib[2] = KERN_PROC_PATHNAME;
	mib[3] = -1;

	sysctl(mib, 4, buf, &cb, NULL, 0);
	return strdup(buf);
}

int
tak_open(tak_t* _tak, int type_info, int data_source)
{
	tak_t tak;
	int ctf_retval;

	tak = malloc(sizeof(struct tak));	
	tak->type_info = type_info;
	tak->data_source = data_source;

	if (type_info == TAK_TYPE_INFO_CTF) {
		ctf_retval = ctf_file_read(get_current_executable_path(), &tak->local_ctf);
		if (ctf_is_error(ctf_retval)) {
			free(tak);
			return TAK_E_LOCAL_CTF;
		}
	} else {
		free(tak);
		return TAK_E_TYPE_INFO_UNKNOWN;
	}

	if (data_source == TAK_DATA_SOURCE_KVM) {
		tak->target_kvm = kvm_open(NULL, NULL, NULL, O_RDONLY, NULL);
		if (tak->target_kvm == NULL)
			return TAK_E_KVM;

		/* TODO do some type_info checks for CTF */
		ctf_retval = ctf_file_read(getbootfile(), &tak->target_ctf);
		if (ctf_is_error(ctf_retval)) {
			kvm_close(tak->target_kvm);
			free(tak);
			return TAK_E_TARGET_CTF;
		}
	} else {
		free(tak);
		return TAK_E_DATA_SOURCE_UNKNOWN;
	}

	*_tak = tak;
	return TAK_OK;
}

