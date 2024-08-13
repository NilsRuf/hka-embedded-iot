// This file conains our application's entry point.

#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>

// We register a log module here so we can use LOG_INF, LOG_ERR, etc. in this
// module.
LOG_MODULE_REGISTER(app);

int main(void)
{
    LOG_INF("entering main");
    // Seems we've got nothing to do here for now...
    return 0;
}