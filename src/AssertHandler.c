#include "AssertHandler.h"
#include <assert.h>

static ESCX_AssertCallback ESCX_assert_callback = (ESCX_AssertCallback)0;

void ESCX_SetAssertCallback(ESCX_AssertCallback callback)
{
    ESCX_assert_callback = callback;
}

void ESCX_AssertFailure(const char *file, unsigned long line, const char *condition)
{
    if (ESCX_assert_callback != (ESCX_AssertCallback)0)
    {
        ESCX_assert_callback(file, line, condition);
    }
    else
    {
        (void)file;
        (void)line;
        (void)condition;
        assert(0);
    }
}
