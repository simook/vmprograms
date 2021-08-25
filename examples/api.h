#pragma once
#include <stddef.h>
#include <stdint.h>
#include <string.h>

#ifdef __cplusplus
extern "C" {
#endif

#ifndef KVM_API_ALREADY_DEFINED
asm(".global backend_response\n" \
".type backend_response, function\n" \
"backend_response:\n" \
"	mov $0xFFFF, %eax\n" \
"	out %eax, $0\n");

asm(".global storage_call\n" \
".type storage_call, function\n" \
"storage_call:\n" \
"	mov $0x10707, %eax\n" \
"	out %eax, $0\n" \
"   ret\n");

asm(".global storage_callv\n" \
".type storage_callv, function\n" \
"storage_callv:\n" \
"	mov $0x10708, %eax\n" \
"	out %eax, $0\n" \
"   ret\n");

asm(".global storage_return\n" \
".type storage_return, function\n" \
"storage_return:\n" \
"	mov $0xFFFF, %eax\n" \
"	out %eax, $0\n" \
"   ret\n");

asm(".global vmcommit\n" \
".type vmcommit, function\n" \
"vmcommit:\n" \
"	mov $0x1070A, %eax\n" \
"	out %eax, $0\n" \
"   ret\n");
#endif

/* Use this to create a backend response from a KVM backend */
extern void __attribute__((noreturn))
backend_response(int16_t status, const void *t, uint64_t, const void *c, uint64_t);

static inline
void backend_response_str(int16_t status, const char *ctype, const char *content)
{
	backend_response(status, ctype, strlen(ctype), content, strlen(content));
}

/* Vector-based serialized call into storage VM */
struct virtbuffer {
	void  *data;
	size_t len;
};
typedef void (*storage_func) (size_t n, struct virtbuffer[n], size_t res);

extern long
storage_call(storage_func, const void* src, size_t, void* dst, size_t);

extern long
storage_callv(storage_func, size_t n, const struct virtbuffer[n], void* dst, size_t);

/* Used to return from storage functions */
extern void
storage_return(const void* data, size_t len);

static inline void
storage_return_nothing(void) { storage_return(NULL, 0); }

/* Record the current state into a new VM, and make that
   VM handle future requests. WARNING: RCU, Racey */
extern long vmcommit(void);

/* This cannot be used when KVM is used as a backend */
#ifndef KVM_API_ALREADY_DEFINED
#define DYNAMIC_CALL(name, hash, ...) \
	asm(".global " #name "\n" \
	#name ":\n" \
	"	mov $" #hash ", %eax\n" \
	"	out %eax, $1\n" \
	"   ret\n"); \
	extern long name(__VA_ARGS__);
#else
#define DYNAMIC_CALL(name, hash, ...) \
	extern long name(__VA_ARGS__);
#endif
DYNAMIC_CALL(goto_dns, 0x746238D2)

struct curl_opts {
	long   status;
	size_t content_length;
	void  *content;
	long   ctlen;
	char   ctype[256];
};
DYNAMIC_CALL(curl_fetch, 0xB86011FB, const char*, size_t, struct curl_opts*)

/* Embed binary data into executable */
#define EMBED_BINARY(name, filename) \
	asm(".section .rodata\n" \
	"	.global " #name "\n" \
	#name ":\n" \
	"	.incbin " #filename "\n" \
	#name "_end:\n" \
	"	.int  0\n" \
	"	.global " #name "_size\n" \
	"	.type   " #name "_size, @object\n" \
	"	.align 4\n" \
	#name "_size:\n" \
	"	.int  " #name "_end - " #name "\n" \
	".section .text"); \
	extern char name[]; \
	extern unsigned name ##_size;

#define TRUST_ME(ptr)    ((void*)(uintptr_t)(ptr))

#ifdef __cplusplus
}
#endif
