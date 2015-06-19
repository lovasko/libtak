#ifndef TAK_FIND_H
#define TAK_FIND_H

#include <ctf/ctf.h>

/**
 * Locate a type from the CTF data set based on the human readable name.

 * Note that it is not possible to find a pointer, array or a type qualifiers.
 * All structs, unions and enums shall be prefixed by their respective type
 * word, e.g. 'struct proc' or 'enum color'.
 *
 * @param file CTF data set
 * @param name 
 * @return a ctf_type on a match, NULL otherwise
 */
ctf_type
find_type(ctf_file file, char* name);

#endif 

