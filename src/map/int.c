#include "map/map.h"

static intmax_t
sign_extend(void* data, size_t bitsize, size_t bytesize)
{
	intmax_t result;
	int sign;

	result = 0;
	memcpy(&result, data, bytesize);

	if (sizeof(intmax_t)*8 == bitsize)
		return result;

	result &= ((intmax_t)1 << bitsize) - (intmax_t)1;
	sign = !!(result & ((intmax_t)1 << bitsize));
	if (sign)
		result |= ((intmax_t)-1 << bitsize);
	else
		result &= (((intmax_t)1 << bitsize) - (intmax_t)1);

	return result;
}

static uintmax_t
zero_extend(void* data, size_t bitsize, size_t bytesize)
{
	uintmax_t result;

	result = 0;
	memcpy(&result, data, bytesize);

	if (sizeof(uintmax_t)*8 == bitsize)
		return result;

	result &= (((uintmax_t)1 << bitsize) - (uintmax_t)1);

	return result;
}

int
map_int(ctf_type type, void* _arg)
{
	ctf_int_offset offset;
	ctf_int_size size;
	ctf_int_content content;
	ctf_int _int;
	unsigned int nbytes;
	intmax_t signed_int;
	uintmax_t unsigned_int;
	uint8_t is_signed;
	uint8_t sign;
	struct map_arg* arg;
	intmax_t signed_data;
	size_t bytesize;

	printf("mapping an integer!\n");

	arg = _arg;
	ctf_int_init(type, &_int);
	ctf_int_get_offset(_int, &offset);
	ctf_int_get_size(_int, &size);
	ctf_int_get_content(_int, &content);
	ctf_int_get_is_signed(_int, &is_signed);

	nbytes = (offset + size)/8;
	if (nbytes > sizeof(uintmax_t) || nbytes > sizeof(intmax_t))
		return 1;

	if (size % 8 == 0)
		bytesize = size/8;
	else
		bytesize = ((size-(size%8))/8)+1;

	if (is_signed) {
		kvm_read(arg->t->target_kvm, arg->addr, &signed_data, sizeof(intmax_t));
		signed_int = sign_extend(&signed_data, size, bytesize);
		printf("  Value: %lld\n", signed_int);

		if (arg->need_alloc)
			*(arg->output) = malloc(sizeof(intmax_t));
		*((intmax_t*)arg->output) = signed_int;
		printf("  Value: %lld\n", *((intmax_t*)arg->output));
	}

	/* if (is_signed) { */
	/* 	printf("Signed integer\n"); */
	/* 	signed_int = 0; */
	/* 	printf("  Value: %lld\n", signed_int); */

	/* 	signed_int &= ((intmax_t)1 << size) - (intmax_t)1; */
	/* 	printf("  Value: %lld\n", signed_int); */

	/* 	sign = signed_int & ((intmax_t)1 << size); */
	/* 	if (sign) */
	/* 		signed_int |= ((intmax_t)-1 << size); */
	/* 	else */
	/* 		signed_int &= (((intmax_t)1 << size) - (intmax_t)1); */

	/* 	printf("  Value: %lld\n", signed_int); */

	/* 	if (arg->need_alloc) */
	/* 		*(arg->output) = malloc(sizeof(intmax_t)); */
	/* 	**((intmax_t**)arg->output) = signed_int; */
	/* } else { */

	/* 	if (arg->t->data_source == TAK_DATA_SOURCE_KVM) { */
	/* 		kvm_read(arg->t->target_kvm, arg->addr, &unsigned_int, sizeof(uintmax_t)); */
	/* 	} else */ 
	/* 		return TAK_E_DATA_SOURCE_UNKNOWN; */	

	/* 	unsigned_int &= (((uintmax_t)1 << size) - (uintmax_t)1); */

	/* 	if (arg->need_alloc) */
	/* 		*(arg->output) = malloc(sizeof(uintmax_t)); */
	/* 	*((uintmax_t*)arg->output) = unsigned_int; */
	/* } */

	return CTF_OK;
}

