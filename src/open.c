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
tak_open(struct tak* t, int type_info, int data_source)
{
	int ctf_retval;

	t->type_info = type_info;
	t->data_source = data_source;

	if (type_info == TAK_TYPE_INFO_CTF) {
		ctf_retval = ctf_file_read(get_current_executable_path(), &t->local_ctf);
		if (ctf_is_error(ctf_retval))
			return TAK_E_LOCAL_CTF;
	} else
		return TAK_E_TYPE_INFO_UNKNOWN;

	if (data_source == TAK_DATA_SOURCE_KVM) {
		t->target_kvm = kvm_open(NULL, NULL, NULL, O_RDONLY, NULL);
		if (t->target_kvm == NULL)
			return TAK_E_KVM;

		/* TODO do some type_info checks for CTF */
		ctf_retval = ctf_file_read(getbootfile(), &t->target_ctf);
		if (ctf_is_error(ctf_retval)) {
			kvm_close(t->target_kvm);
			return TAK_E_TARGET_CTF;
		}
	} else
		return TAK_E_DATA_SOURCE_UNKNOWN;

	return TAK_OK;
}

