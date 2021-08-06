#include "api.h"
#include <assert.h>
#include <malloc.h>
#include <stdio.h>
static void *my_storage(void*, size_t, size_t);

int main(int argc, char **argv)
{
	printf("Hello from '%s'! Storage=%s\n", argv[1], argv[2]);
}

__attribute__((used))
extern void my_backend(const char *arg)
{
	const char data[] = "Hello World!";
	char result[256];
	const long rlen =
		storage_call(my_storage, data, sizeof(data), result, sizeof(result));

	const char ctype[] = "text/plain";
	backend_response(ctype, sizeof(ctype)-1, result, rlen-1);
}

static int counter = 0;
void* my_storage(void *data, size_t len, size_t reslen)
{
	counter ++;
	((char *)data)[11] = '0' + (counter % 10);
	/* Data contains the inputs */
	storage_return(data, len);
}

__attribute__((used))
extern void on_live_update()
{
	/* Serialize data into ptr, len */
	int *data = (int *)malloc(sizeof(counter));
	*data = counter;
	storage_return(data, sizeof(*data));
}

__attribute__((used))
extern void on_resume_update(size_t len)
{
	assert(len == sizeof(counter));
	/* Allocate room for state, return ptr, len */
	storage_return(&counter, sizeof(counter));

	/* Restore state here */
	printf("Counter state restored: %d\n", counter);
}
