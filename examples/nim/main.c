#include <stdint.h>
extern void NimMain();
extern void __attribute__((noreturn))
backend_response(const void *t, uint64_t, const void *c, uint64_t);

int main()
{
	NimMain();
}

__attribute__((used))
void my_backend()
{
	/* Nim backend main */
	extern void backend();
	backend();
	/* Fallback response in case Nim doesn't generate one */
	const char ctype[] = "text/plain";
	const char content[] = "No response";
	backend_response(ctype, sizeof(ctype)-1, content, sizeof(content)-1);
}
