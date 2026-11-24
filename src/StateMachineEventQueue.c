#include "StateMachineEventQueue.h"
#include "AssertHandler.h"

void ESCX_EventQueueInit(ESCX_EventQueue *queue)
{
    unsigned int i;
    if (queue == (ESCX_EventQueue *)0)
    {
        ESCX_ASSERT_TRUE(0);
        return;
    }
    queue->front = (ESCX_BYTE_T)0U;
    queue->count = (ESCX_BYTE_T)0U;
    for (i = 0U; i < (unsigned int)ESCX_QUEUE_SIZE; ++i)
    {
        queue->items[i].event_id = (ESCX_BYTE_T)0U;
        queue->items[i].event_type = (ESCX_BYTE_T)0U;
        queue->items[i].origin_type = (ESCX_BYTE_T)0U;
        queue->items[i].invoke_id = (ESCX_BYTE_T)255U;
        queue->items[i].send_id = (ESCX_UINT16_T)0U;
#if ESCX_ENABLE_EVENT_DATA
        queue->items[i].event_data = (void *)0;
#endif
    }
}

ESCX_BOOL_T ESCX_EventQueueIsEmpty(const ESCX_EventQueue *queue)
{
    if (queue == (const ESCX_EventQueue *)0)
    {
        ESCX_ASSERT_TRUE(0);
        return ESCX_TRUE;
    }
    return (queue->count == (ESCX_BYTE_T)0U) ? ESCX_TRUE : ESCX_FALSE;
}

ESCX_BOOL_T ESCX_EventQueueIsFull(const ESCX_EventQueue *queue)
{
    if (queue == (const ESCX_EventQueue *)0)
    {
        ESCX_ASSERT_TRUE(0);
        return ESCX_TRUE;
    }
    return ((unsigned int)queue->count >= (unsigned int)ESCX_QUEUE_SIZE) ? ESCX_TRUE : ESCX_FALSE;
}

ESCX_BOOL_T ESCX_EventQueuePushEx(ESCX_EventQueue *queue, ESCX_BYTE_T event_id,
    ESCX_BYTE_T event_type, ESCX_BYTE_T origin_type, ESCX_BYTE_T invoke_id,
    ESCX_UINT16_T send_id, void *event_data)
{
    unsigned int index;
    if (queue == (ESCX_EventQueue *)0) { ESCX_ASSERT_TRUE(0); return ESCX_FALSE; }
    if (ESCX_EventQueueIsFull(queue) != ESCX_FALSE) return ESCX_FALSE;
    index = (unsigned int)queue->front + (unsigned int)queue->count;
    if (index >= (unsigned int)ESCX_QUEUE_SIZE) index -= (unsigned int)ESCX_QUEUE_SIZE;
    queue->items[index].event_id=event_id; queue->items[index].event_type=event_type;
    queue->items[index].origin_type=origin_type; queue->items[index].invoke_id=invoke_id;
    queue->items[index].send_id=send_id;
#if ESCX_ENABLE_EVENT_DATA
    queue->items[index].event_data=event_data;
#else
    (void)event_data;
#endif
    queue->count=(ESCX_BYTE_T)((unsigned int)queue->count+1U); return ESCX_TRUE;
}
ESCX_BOOL_T ESCX_EventQueuePush(ESCX_EventQueue *queue, ESCX_BYTE_T event_id, void *event_data)
{
    return ESCX_EventQueuePushEx(queue,event_id,1U,0U,255U,0U,event_data);
}
ESCX_BOOL_T ESCX_EventQueuePop(ESCX_EventQueue *queue, ESCX_QueuedEvent *event_out)
{
    unsigned int next_front;
    if ((queue == (ESCX_EventQueue *)0) ||
        (event_out == (ESCX_QueuedEvent *)0))
    {
        ESCX_ASSERT_TRUE(0);
        return ESCX_FALSE;
    }
    if (ESCX_EventQueueIsEmpty(queue) != ESCX_FALSE)
    {
        return ESCX_FALSE;
    }
    *event_out = queue->items[queue->front];
#if ESCX_ENABLE_EVENT_DATA
    queue->items[queue->front].event_data = (void *)0;
#endif
    next_front = (unsigned int)queue->front + 1U;
    if (next_front >= (unsigned int)ESCX_QUEUE_SIZE)
    {
        next_front = 0U;
    }
    queue->front = (ESCX_BYTE_T)next_front;
    queue->count = (ESCX_BYTE_T)((unsigned int)queue->count - 1U);
    return ESCX_TRUE;
}
