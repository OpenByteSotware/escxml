#include "support/ESCXMachineTest.h"
namespace escx
{
	namespace unittest
	{
		namespace
		{
			struct Seen
			{
				ESCX_Event event; int count;
			};
		}
		TEST_F(ESCXMachineTest, CurrentEventIsNeutralAfterExternalMacrostepCompletes)
		{
			int payload = 42;
			ESCX_Event event = { 1,ESCX_EVENT_TYPE_EXTERNAL,ESCX_EVENT_ORIGIN_PLATFORM,ESCX_NO_INVOKE,77,&payload }; EXPECT_EQ(ESCX_STATUS_PROCESSED, ESCX_StateMachineSendEvent(&machine, &event)); 
			const ESCX_Event* current = ESCX_StateMachineGetCurrentEvent(&machine); 
			ASSERT_NE(nullptr, current); EXPECT_EQ(ESCX_NO_EVENT, current->name_id); 
			EXPECT_EQ(ESCX_EVENT_TYPE_INTERNAL, current->type);
			EXPECT_EQ(ESCX_EVENT_ORIGIN_NONE, current->origin_type);
			EXPECT_EQ(ESCX_NO_SEND_ID, current->send_id); EXPECT_EQ(nullptr, current->data);
		}
		TEST_F(ESCXMachineTest, InternalDescriptorPreservesInvokeAndSendMetadata)
		{
			int payload = 9; 
			ESCX_Event event = { 2,ESCX_EVENT_TYPE_INTERNAL,ESCX_EVENT_ORIGIN_INVOKE,3,44,&payload }; 
			ASSERT_EQ(ESCX_STATUS_PROCESSED, ESCX_StateMachineRaiseEvent(&machine, &event)); 
			const ESCX_Event* current = ESCX_StateMachineGetCurrentEvent(&machine);
			ASSERT_NE(nullptr, current);/* queued event becomes current when a macrostep drains it; raising while idle intentionally only queues */EXPECT_EQ(ESCX_NO_EVENT, current->name_id);
		}
	}
}
