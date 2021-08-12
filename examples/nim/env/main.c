#include <stddef.h>
#include <stdint.h>
extern void NimMain();
extern void __attribute__((noreturn))
backend_response(const void *t, uint64_t, const void *c, uint64_t);

typedef void (*request_func_t)(const char *);
typedef void (*post_func_t)();
extern void post_backend_trampoline(post_func_t, const char*, const char*, size_t);

int main()
{
	NimMain();
}

static request_func_t request_handler = NULL;
extern void set_request_handler(request_func_t handler) {
	request_handler = handler;
}

extern void __attribute__((used))
my_backend(const char *arg)
{
	/* Nim backend main */
	if (request_handler) {
		request_handler(arg);
	}
	/* Fallback response in case Nim doesn't generate one */
	const char ctype[] = "text/plain";
	const char content[] = "No response";
	backend_response(ctype, sizeof(ctype)-1, content, sizeof(content)-1);
}

static post_func_t post_handler = NULL;
extern void set_post_handler(post_func_t handler) {
	post_handler = handler;
}

extern void __attribute__((used))
my_post_backend(const char *arg, void *data, size_t len)
{
	/* Nim backend main */
	if (post_handler) {
		post_backend_trampoline(post_handler, arg, data, len);
	}
	/* Fallback response in case Nim doesn't generate one */
	const char ctype[] = "text/plain";
	const char content[] = "No response";
	backend_response(ctype, sizeof(ctype)-1, content, sizeof(content)-1);
}
