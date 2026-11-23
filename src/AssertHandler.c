#include "AssertHandler.h"
#include <assert.h>

static ESCX_AssertCallback scx_assert_callback = (ESCX_AssertCallback)0;

void ESCX_SetAssertCallback(ESCX_AssertCallback callback)
{
    scx_assert_callback = callback;
}

void ESCX_AssertFailure(const char *file, unsigned long line, const char *condition)
{
    if (scx_assert_callback != (ESCX_AssertCallback)0)
    {
        scx_assert_callback(file, line, condition);
    }
    else
    {
        (void)file;
        (void)line;
        (void)condition;
        assert(0);
    }
}
