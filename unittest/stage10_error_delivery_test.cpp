#include "support/ESCXMachineTest.h"
namespace escx { namespace unittest {
TEST_F(ESCXMachineTest, TimerStartFailureRecordsExecutionError)
{
    ESCX_SendRequest request={1,ESCX_SEND_TARGET_INTERNAL,0,10,5,0};
    EXPECT_EQ(ESCX_STATUS_TIMER_START_FAILED,ESCX_StateMachineSendRequest(&machine,&request));
    EXPECT_EQ(ESCX_ERROR_EXECUTION,ESCX_StateMachineGetLastError(&machine));
    EXPECT_EQ(ESCX_STATUS_TIMER_START_FAILED,ESCX_StateMachineGetLastErrorStatus(&machine));
}
TEST_F(ESCXMachineTest, MissingParentRecordsCommunicationError)
{
    ESCX_SendRequest request={1,ESCX_SEND_TARGET_PARENT,0,0,0,0};
    EXPECT_EQ(ESCX_STATUS_NO_PARENT,ESCX_StateMachineSendRequest(&machine,&request));
    EXPECT_EQ(ESCX_ERROR_COMMUNICATION,ESCX_StateMachineGetLastError(&machine));
    EXPECT_EQ(ESCX_STATUS_NO_PARENT,ESCX_StateMachineGetLastErrorStatus(&machine));
}
TEST_F(ESCXMachineTest, PublicErrorReportRecordsCauseAndClearResetsIt)
{
    EXPECT_EQ(ESCX_STATUS_EXECUTION_ERROR,ESCX_StateMachineReportError(&machine,ESCX_ERROR_PLATFORM,ESCX_STATUS_EXECUTION_ERROR));
    EXPECT_EQ(ESCX_ERROR_PLATFORM,ESCX_StateMachineGetLastError(&machine));
    ESCX_StateMachineClearError(&machine);
    EXPECT_EQ(ESCX_ERROR_NONE,ESCX_StateMachineGetLastError(&machine));
    EXPECT_EQ(ESCX_STATUS_PROCESSED,ESCX_StateMachineGetLastErrorStatus(&machine));
}
TEST_F(ESCXMachineTest, ConfiguredErrorEventCanBeDeliveredWithoutRecursion)
{
    ASSERT_EQ(ESCX_STATUS_PROCESSED,ESCX_StateMachineConfigureErrorEvents(&machine,2,3));
    EXPECT_EQ(ESCX_STATUS_INVALID_TARGET,ESCX_StateMachineReportError(&machine,ESCX_ERROR_COMMUNICATION,ESCX_STATUS_INVALID_TARGET));
    EXPECT_EQ(ESCX_ERROR_COMMUNICATION,ESCX_StateMachineGetLastError(&machine));
}
} }
