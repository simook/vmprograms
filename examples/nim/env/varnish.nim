
proc backend_response*(ctype: cstring, clen: int, data: cstring, length: int) {.importc, header: "api.h"}

type
    request_cb = proc(arg: cstring) {.cdecl.}
    post_cb = proc(arg: string, data: string): (string, string) {.cdecl.}

proc set_request_handler*(handler: request_cb) {.importc.}
proc set_post_handler*(handler: post_cb) {.importc.}

proc post_backend_trampoline*(callback: post_cb, arg: cstring, data: cstring, length: int) {.exportc.} =
    var nimdata = newString(length)
    copyMem(addr(nimdata[0]), data, length)
    let (a, b) = callback($arg, nimdata)
    backend_response(a, len(a), b, len(b))

proc response*(x: string, y: string) {.exportc.} =
    backend_response(x, len(x), y, len(y))
