// build_id.c — persistent firmware breadcrumbs (visible via `strings` on flash dumps)
//
// Goal: years later, you can `dump_image` flash and run `strings` to learn what this firmware is.
// This does not require UART/printf/semihosting; it is just constant data in flash.

#ifndef BUILD_STAGE
#define BUILD_STAGE "stage3"
#endif

#ifndef BUILD_TARGET
#define BUILD_TARGET "NUCLEO-L432KC"
#endif

#ifndef GIT_HASH
#define GIT_HASH "nogit"
#endif

// Keep this symbol and keep its section (linker KEEP() below).
__attribute__((used, section(".build_id")))
const char g_build_id[] =
"FWID\n"
"stage=" BUILD_STAGE "\n"
"target=" BUILD_TARGET "\n"
"git=" GIT_HASH "\n"
"built=" __DATE__ " " __TIME__ "\n"
"\n"
"      .-.\n"
"     (   ).\n"
"    (___(__)\n"
"   ' ' ' '    cider + OpenOCD\n"
"   ' ' ' '    ::blink:: archaeology\n";
