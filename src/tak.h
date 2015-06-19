#include <ctf/ctf.h>

#include <kvm.h>

#define TAK_OK                    0
#define TAK_E_LOCAL_CTF           1
#define TAK_E_TARGET_CTF          2
#define TAK_E_SYMBOL              3
#define TAK_E_NO_TYPE_MATCH       4
#define TAK_E_NO_TYPE             5
#define TAK_E_TYPE_INFO_UNKNOWN   6
#define TAK_E_DATA_SOURCE_UNKNOWN 7
#define TAK_E_KVM                 8

#define TAK_TYPE_INFO_CTF 0
/* #define TAK_TYPE_INFO_DWARF 1 */

#define TAK_DATA_SOURCE_KVM      0
/* #define TAK_DATA_SOURCE_PTRACE   1 */
/* #define TAK_DATA_SOURCE_ELF_DUMP 2 */

struct tak
{
	int type_info;
	int data_source;
	ctf_file local_ctf;
	ctf_file target_ctf;
	kvm_t* target_kvm;
};

typedef struct tak* tak_t;

/*
 * IDEA
 * 
 * This function could potentially contain varargs to specify things like a PID
 * for the ptrace(2) data source, path to the executable, information about the
 * kvm(3) target and so on.
 */
int tak_open(tak_t* tak, int type_info, int data_source);

int tak_map_sym(tak_t tak, char* local_type_name, char* symbol_name, void** output);

