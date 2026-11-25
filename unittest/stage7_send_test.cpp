#include "support/ESCXMachineTest.h"

namespace escx
{
    namespace unittest
    {
        namespace
        {
            struct FakeTimer
            {
                ESCX_TimerId next;
                unsigned starts;
                unsigned cancels;
                ESCX_TimerId last;
            };

            ESCX_TimerId Start(
                ESCX_StateMachine*,
                unsigned long delay,
                void* ctx)
            {
                FakeTimer* t = static_cast<FakeTimer*>(ctx);

                if (delay == 0UL)
                {
                    return ESCX_INVALID_TIMER_ID;
                }

                ++t->starts;
                return t->next++;
            }

            ESCX_BOOL_T Cancel(
                ESCX_StateMachine*,
                ESCX_TimerId id,
                void* ctx)
            {
                FakeTimer* t = static_cast<FakeTimer*>(ctx);

                ++t->cancels;
                t->last = id;

                return ESCX_TRUE;
            }
        }

        TEST_F(
            ESCXMachineTest,
            DelayedSendUsesAndCancelsClientTimer)
        {
            FakeTimer t =
            {
                100,
                0,
                0,
                ESCX_INVALID_TIMER_ID
            };

            /*
             * A positive-delay send must use a delay-capable target.
             * The internal queue permits only zero-delay sends.
             */
            ESCX_SendRequest q =
            {
                1,
                ESCX_SEND_TARGET_SELF,
                0,
                7,
                10,
                0
            };

            ASSERT_EQ(
                ESCX_STATUS_PROCESSED,
                ESCX_StateMachineConfigureTimers(
                    &machine,
                    Start,
                    Cancel,
                    &t));

            EXPECT_EQ(
                ESCX_STATUS_PROCESSED,
                ESCX_StateMachineSendRequest(&machine, &q));

            EXPECT_EQ(1U, t.starts);

            EXPECT_EQ(
                ESCX_STATUS_PROCESSED,
                ESCX_StateMachineCancelSend(&machine, 7));

            EXPECT_EQ(1U, t.cancels);
            EXPECT_EQ(100L, t.last);

            EXPECT_EQ(
                ESCX_STATUS_IGNORED,
                ESCX_StateMachineTimerExpired(&machine, 100));
        }

        TEST_F(
            ESCXMachineTest,
            ExpirationDispatchesOnlyOnce)
        {
            FakeTimer t =
            {
                25,
                0,
                0,
                ESCX_INVALID_TIMER_ID
            };

            ESCX_SendRequest q =
            {
                1,
                ESCX_SEND_TARGET_SELF,
                0,
                8,
                5,
                0
            };

            ASSERT_EQ(
                ESCX_STATUS_PROCESSED,
                ESCX_StateMachineConfigureTimers(
                    &machine,
                    Start,
                    Cancel,
                    &t));

            ASSERT_EQ(
                ESCX_STATUS_PROCESSED,
                ESCX_StateMachineSendRequest(&machine, &q));

            EXPECT_EQ(
                ESCX_STATUS_PROCESSED,
                ESCX_StateMachineTimerExpired(&machine, 25));

            EXPECT_EQ(
                ESCX_STATUS_IGNORED,
                ESCX_StateMachineTimerExpired(&machine, 25));
        }

        TEST_F(
            ESCXMachineTest,
            DelayedSendRequiresTimerClient)
        {
            ESCX_SendRequest q =
            {
                1,
                ESCX_SEND_TARGET_SELF,
                0,
                9,
                5,
                0
            };

            EXPECT_EQ(
                ESCX_STATUS_TIMER_START_FAILED,
                ESCX_StateMachineSendRequest(&machine, &q));
        }
    }
}