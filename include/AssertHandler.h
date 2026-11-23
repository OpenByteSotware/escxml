#ifndef ESCX_ASSERT_HANDLER_H
#define ESCX_ASSERT_HANDLER_H
#include "ESCXConfig.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef void (*ESCX_AssertCallback)(const char *file, unsigned long line, const char *condition);
void ESCX_SetAssertCallback(ESCX_AssertCallback callback);
void ESCX_AssertFailure(const char *file, unsigned long line, const char *condition);

#if ESCX_ENABLE_ASSERTS
#define ESCX_ASSERT_TRUE(condition) \
    do { if (!(condition)) { ESCX_AssertFailure(__FILE__, (unsigned long)__LINE__, #condition); } } while (0)
#else
#define ESCX_ASSERT_TRUE(condition) do { (void)sizeof(condition); } while (0)
#endif

#ifdef __cplusplus
}
#endif
#endif
