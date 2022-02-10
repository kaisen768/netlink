#include "netlink.h"

int main(int argc, const char *argv[])
{
    uv_loop_t *loop;
    nl_module_t nl;

    loop = uv_default_loop();

    nl_module_init(&nl, loop);

    nl.start(&nl);

    uv_run(loop, UV_RUN_DEFAULT);

    return 0;
}
