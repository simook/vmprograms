#include "api.h"
#include <assert.h>
#include <stdio.h>
#include <quickjs/cutils.h>
#include <quickjs/quickjs.h>
#include <quickjs/quickjs-libc.h>
#define COUNTOF(a)  (sizeof(a) / sizeof(*(a)))
void dlopen() {}; void dlsym() {}; void dlclose() {} /* Don't ask */
static JSValue js_backend_response(JSContext*, JSValueConst, int, JSValueConst*);
static JSValue js_storage_call(JSContext*, JSValueConst, int, JSValueConst*);
static JSValue js_is_storage_call(JSContext*, JSValueConst, int, JSValueConst*);
static JSValue js_vmcommit_call(JSContext*, JSValueConst, int, JSValueConst*);
static JSValue js_sendfile_call(JSContext*, JSValueConst, int, JSValueConst*);
extern JSValue js_http_fetch_call(JSContext*, JSValueConst, int, JSValueConst*);
extern void static_site(const char*);

static int eval_buf(JSContext *ctx, const void *buf, int buf_len,
					const char *filename, int eval_flags)
{
	JSValue val;
	int ret;

	if ((eval_flags & JS_EVAL_TYPE_MASK) == JS_EVAL_TYPE_MODULE) {
		/* for the modules, we compile then run to be able to set
		   import.meta */
		val = JS_Eval(ctx, buf, buf_len, filename,
					  eval_flags | JS_EVAL_FLAG_COMPILE_ONLY);
		if (!JS_IsException(val)) {
			js_module_set_import_meta(ctx, val, TRUE, TRUE);
			val = JS_EvalFunction(ctx, val);
		}
	} else {
		val = JS_Eval(ctx, buf, buf_len, filename, eval_flags);
	}
	if (JS_IsException(val)) {
		js_std_dump_error(ctx);
		ret = -1;
	} else {
		ret = 0;
	}
	JS_FreeValue(ctx, val);
	return ret;
}

EMBED_BINARY(myjs, "src/my.js");

static JSValue global_obj;
static JSContext *g_ctx;
static struct {
	JSValue varnish;
	JSValue backend_func;
	JSValue post_backend_func;
} vapi;

static int is_storage = 0;
int main(int argc, char** argv)
{
	is_storage = (*argv[2] == '1');

	/* QuickJS runtime */
	JSRuntime *rt = JS_NewRuntime();
	g_ctx = JS_NewContext(rt);

	js_std_init_handlers(rt);
	/* system modules */
	js_init_module_std(g_ctx, "std");
	js_init_module_os(g_ctx, "os");

	js_std_add_helpers(g_ctx, argc, argv);

	global_obj = JS_GetGlobalObject(g_ctx);

	/*** Begin VARNISH API ***/
	vapi.varnish = JS_NewObject(g_ctx);
	JS_SetPropertyStr(g_ctx, global_obj, "varnish", vapi.varnish);

	JS_SetPropertyStr(g_ctx, vapi.varnish,
		"response",
		JS_NewCFunction(g_ctx, js_backend_response, "response", 3));
	JS_SetPropertyStr(g_ctx, vapi.varnish,
		"sendfile",
		JS_NewCFunction(g_ctx, js_sendfile_call, "sendfile", 1));
	JS_SetPropertyStr(g_ctx, vapi.varnish,
		"vmcommit",
		JS_NewCFunction(g_ctx, js_vmcommit_call, "vmcommit", 0));
	JS_SetPropertyStr(g_ctx, vapi.varnish,
		"storage",
		JS_NewCFunction(g_ctx, js_storage_call, "storage", 2));
	JS_SetPropertyStr(g_ctx, vapi.varnish,
		"is_storage",
		JS_NewCFunction(g_ctx, js_is_storage_call, "is_storage", 0));
	JS_SetPropertyStr(g_ctx, vapi.varnish,
		"fetch",
		JS_NewCFunction(g_ctx, js_http_fetch_call, "fetch", 1));
	/*** End Of VARNISH API ***/

	const char *str = "import * as std from 'std';\n"
		"import * as os from 'os';\n"
		"globalThis.std = std;\n"
		"globalThis.os = os;\n";
	eval_buf(g_ctx, str, strlen(str), "<input>", JS_EVAL_TYPE_MODULE);

	eval_buf(g_ctx, myjs, myjs_size, "my.js", JS_EVAL_TYPE_GLOBAL);

	vapi.backend_func =
		JS_GetPropertyStr(g_ctx, global_obj, "my_backend");
	assert(JS_IsFunction(g_ctx, vapi.backend_func));
	vapi.post_backend_func =
		JS_GetPropertyStr(g_ctx, global_obj, "my_post_backend");
	assert(JS_IsFunction(g_ctx, vapi.post_backend_func));
}

JSValue js_backend_response(JSContext *ctx,
	JSValueConst this_val, int argc, JSValueConst *argv)
{
	(void)this_val;
	if (argc == 3) {
		int code;
		if (JS_ToInt32(ctx, &code, argv[0]))
			return JS_EXCEPTION;
		size_t tlen;
		const char* type =
			JS_ToCStringLen(ctx, &tlen, argv[1]);
		size_t clen;
		const char* cont =
			JS_ToCStringLen(ctx, &clen, argv[2]);
		if (!type || !cont)
			return JS_EXCEPTION;
		/* Give response to waiting clients */
		backend_response(code, type, tlen, cont, clen);
	}
	return JS_EXCEPTION;
}

JSValue js_sendfile_call(JSContext *ctx,
	JSValueConst this_val, int argc, JSValueConst *argv)
{
	(void)this_val;
	if (argc == 1) {
		size_t flen;
		const char* filename =
			JS_ToCStringLen(ctx, &flen, argv[0]);
		if (!filename)
			return JS_EXCEPTION;
		/* Send static resource (or 404) */
		static_site(filename);
	}
	return JS_EXCEPTION;
}

extern void __attribute__((used))
my_backend(const char *arg)
{
	/* Then call into JS */
	JSValueConst argv[1];
	argv[0] = JS_NewString(g_ctx, arg);

	JSValue ret = JS_Call(g_ctx,
		vapi.backend_func,
		JS_UNDEFINED,
		countof(argv), argv
	);
	if (JS_IsException(ret)) {
		js_std_dump_error(g_ctx);
		backend_response_str(500, "text/plain", "Internal server error");
	}
	backend_response_str(404, "text/html", "Not found");
}

extern void __attribute__((used))
my_post_backend(const char *arg, void *data, size_t len)
{
	JSValueConst argv[2];
	argv[0] = JS_NewString(g_ctx, arg);
	argv[1] = JS_NewStringLen(g_ctx, data, len);

	JSValue ret = JS_Call(g_ctx,
		vapi.post_backend_func,
		JS_UNDEFINED,
		countof(argv), argv
	);
	if (JS_IsException(ret)) {
		js_std_dump_error(g_ctx);
		backend_response_str(500, "text/plain", "Internal server error");
	}
	backend_response_str(404, "text/html", "Not found");
}

static void storage_trampoline(
	size_t n, struct virtbuffer buffers[n], size_t reslen)
{
	JSValue sfunc =
		JS_GetPropertyStr(g_ctx, global_obj, buffers[0].data);
	assert(JS_IsFunction(g_ctx, sfunc));

	JSValueConst argv[1];
	argv[0] = JS_NewStringLen(g_ctx, buffers[1].data, buffers[1].len);

	JSValue ret = JS_Call(g_ctx,
		sfunc,
		JS_UNDEFINED,
		countof(argv), argv
	);
	if (JS_IsException(ret)) {
		js_std_dump_error(g_ctx);
		/* Cleanup */
		JS_FreeValue(g_ctx, ret);
		storage_return_nothing();
		return;
	}

	size_t textlen;
	const char *text =
		JS_ToCStringLen(g_ctx, &textlen, ret);
	assert(textlen <= reslen);
	storage_return(text, textlen);
	/* Cleanup */
	JS_FreeValue(g_ctx, ret);
}

JSValue js_storage_call(JSContext *ctx,
	JSValueConst this_val, int argc, JSValueConst *argv)
{
	(void)this_val;
	if (argc >= 1) {
		size_t funclen;
		const char* func =
			JS_ToCStringLen(ctx, &funclen, argv[0]);
		size_t datalen = 0;
		const char* data = "";
		if (argc >= 2) {
			data = JS_ToCStringLen(ctx, &datalen, argv[1]);
		}
		if (__builtin_expect(!func || !data, 0))
			return JS_EXCEPTION;

		const struct virtbuffer buffers[2] = {
			{TRUST_ME(func), funclen+1},
			{TRUST_ME(data), datalen}
		};
		/* Make room for a decent-sized buffer */
		const size_t result_size = 65536ul;
		char *result = malloc(result_size);

		/* Make call into storage VM */
		long reslen = storage_callv(storage_trampoline,
			2, buffers, result, result_size);

		/* Probably an exception is < 0 */
		if (reslen >= 0) {
			/* Create a JS string from the result */
			JSValue resstr = JS_NewStringLen(ctx, result, reslen);
			free(result);
			return resstr;
		}
		free(result);
	}
	return JS_EXCEPTION;
}

JSValue js_is_storage_call(JSContext *ctx,
	JSValueConst this_val, int argc, JSValueConst *argv)
{
	(void)this_val;
	(void)argc;
	(void)argv;
	return JS_NewBool(ctx, is_storage);
}

JSValue js_vmcommit_call(JSContext *ctx,
	JSValueConst this_val, int argc, JSValueConst *argv)
{
	(void) this_val;
	(void) argc;
	(void) argv;
	return JS_NewBool(ctx, vmcommit() == 0);
}

__attribute__((used))
extern void on_live_update()
{
	JSValue sfunc =
		JS_GetPropertyStr(g_ctx, global_obj, "on_live_update");
	if (!JS_IsFunction(g_ctx, sfunc))
		return;

	JSValue ret = JS_Call(g_ctx,
		sfunc,
		JS_UNDEFINED,
		0, NULL
	);

	/* TODO: Accept ArrayBuffer as well */
	if (JS_IsString(ret))
	{
		size_t datalen;
		const char *data =
			JS_ToCStringLen(g_ctx, &datalen, ret);
		storage_return(data, datalen);
		/* Cleanup */
		JS_FreeValue(g_ctx, ret);
	} else {
		storage_return_nothing();
	}
}

__attribute__((used))
extern void on_resume_update(size_t len)
{
	char* data = malloc(len);
	storage_return(data, len);

	JSValue sfunc =
		JS_GetPropertyStr(g_ctx, global_obj, "on_resume_update");
	if (!JS_IsFunction(g_ctx, sfunc)) {
		free(data);
		return;
	}

	JSValueConst argv[1];
	argv[0] = JS_NewStringLen(g_ctx, data, len);

	free(data);

	JSValue ret = JS_Call(g_ctx,
		sfunc,
		JS_UNDEFINED,
		countof(argv), argv
	);

	/* Cleanup */
	JS_FreeValue(g_ctx, ret);
}
