#include "support/ESCXMachineTest.h"

namespace escx {
namespace unittest {

TEST_F(ESCXMachineTest, ConfiguresAndClearsErrorState)
{
    EXPECT_EQ(ESCX_STATUS_PROCESSED, ESCX_StateMachineConfigureErrorEvents(&machine, 2, 3));
    EXPECT_EQ(ESCX_ERROR_NONE, ESCX_StateMachineGetLastError(&machine));
    EXPECT_EQ(ESCX_STATUS_PROCESSED, ESCX_StateMachineGetLastErrorStatus(&machine));
    ESCX_StateMachineClearError(&machine);
    EXPECT_EQ(ESCX_ERROR_NONE, ESCX_StateMachineGetLastError(&machine));
}

TEST_F(ESCXMachineTest, RejectsOutOfRangeErrorEvent)
{
    EXPECT_EQ(ESCX_STATUS_INVALID_EVENT,
              ESCX_StateMachineConfigureErrorEvents(&machine, 4, ESCX_NO_EVENT));
}

} // namespace unittest
} // namespace escx
