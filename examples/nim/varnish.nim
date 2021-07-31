
proc backend_response*(ctype: cstring, clen: int, data: cstring, length: int) {.importc, header: "api.h"}

proc response*(x: string, y: string) {.exportc.} =
    backend_response(x, len(x), y, len(y))
