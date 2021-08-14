#include "../api.h"
#include <assert.h>
#include <stdio.h>
#include "quickjs/cutils.h"
#include "quickjs/quickjs.h"
#include "quickjs/quickjs-libc.h"
#define COUNTOF(a)  (sizeof(a) / sizeof(*(a)))
void dlopen() {}
void dlsym() {}
void dlclose() {}
static JSValue js_backend_response(JSContext*, JSValueConst, int, JSValueConst*);

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

EMBED_BINARY(myjs, "../my.js");

JSValue global_obj;
JSContext *g_ctx;
struct {
	JSValue varnish;
	JSValue backend_func;
} vapi;

int main(int argc, char** argv)
{
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
	return JS_UNDEFINED;
}

extern void __attribute__((used))
my_backend(const char *arg)
{
	JSValueConst argv[1];
	argv[0] = JS_NewString(g_ctx, arg);

	JSValue ret = JS_Call(
		g_ctx,
		vapi.backend_func,
		JS_UNDEFINED,
		countof(argv), argv
	);
	if (JS_IsArray(g_ctx, ret)) {

	}
}
