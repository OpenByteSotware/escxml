#ifndef ESCX_ASSERT_HANDLER_H
#define ESCX_ASSERT_HANDLER_H

#ifdef __cplusplus
extern "C" {
#endif

typedef void (*ESCX_AssertCallback)(const char *file, unsigned long line, const char *condition);
void ESCX_SetAssertCallback(ESCX_AssertCallback callback);
void ESCX_AssertFailure(const char *file, unsigned long line, const char *condition);

#define ESCX_ASSERT_TRUE(condition) \
    do { if (!(condition)) { ESCX_AssertFailure(__FILE__, (unsigned long)__LINE__, #condition); } } while (0)

#ifdef __cplusplus
}
#endif
#endif
