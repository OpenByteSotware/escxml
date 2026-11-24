#include "support/ESCXMachineTest.h"

namespace escx {
namespace unittest {

TEST_F(ESCXMachineTest, DelayedSendCanBeCancelled)
{
    ESCX_SendRequest q = { 1,ESCX_SEND_TARGET_INTERNAL,0,7,10,0 };
    EXPECT_EQ(ESCX_STATUS_PROCESSED, ESCX_StateMachineSendRequest(&machine, &q));
    EXPECT_EQ(ESCX_STATUS_PROCESSED, ESCX_StateMachineCancelSend(&machine, 7));
    EXPECT_EQ(ESCX_STATUS_SEND_NOT_FOUND, ESCX_StateMachineCancelSend(&machine, 7));
}

} // namespace unittest
} // namespace escx
