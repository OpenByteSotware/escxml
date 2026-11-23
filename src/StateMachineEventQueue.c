#include "StateMachineEventQueue.h"
#include "AssertHandler.h"

void ESCX_EventQueueInit(ESCX_EventQueue *queue)
{
    unsigned int i;
    ESCX_ASSERT_TRUE(queue != (ESCX_EventQueue *)0);
    queue->front = 0U;
    queue->count = 0U;
    for (i = 0U; i < ESCX_QUEUE_SIZE; ++i)
    {
        queue->items[i].event_id = 0U;
        queue->items[i].event_data = (void *)0;
    }
}

ESCX_BOOL_T ESCX_EventQueueIsEmpty(const ESCX_EventQueue *queue)
{
    ESCX_ASSERT_TRUE(queue != (const ESCX_EventQueue *)0);
    return (queue->count == 0U) ? ESCX_TRUE : ESCX_FALSE;
}

ESCX_BOOL_T ESCX_EventQueueIsFull(const ESCX_EventQueue *queue)
{
    ESCX_ASSERT_TRUE(queue != (const ESCX_EventQueue *)0);
    return (queue->count >= ESCX_QUEUE_SIZE) ? ESCX_TRUE : ESCX_FALSE;
}

ESCX_BOOL_T ESCX_EventQueuePush(ESCX_EventQueue *queue, ESCX_BYTE_T event_id, void *event_data)
{
    unsigned int index;
    ESCX_ASSERT_TRUE(queue != (ESCX_EventQueue *)0);
    if (ESCX_EventQueueIsFull(queue) != ESCX_FALSE) return ESCX_FALSE;
    index = (queue->front + queue->count) % ESCX_QUEUE_SIZE;
    queue->items[index].event_id = event_id;
    queue->items[index].event_data = event_data;
    ++queue->count;
    return ESCX_TRUE;
}

ESCX_BOOL_T ESCX_EventQueuePop(ESCX_EventQueue *queue, ESCX_QueuedEvent *event_out)
{
    ESCX_ASSERT_TRUE(queue != (ESCX_EventQueue *)0);
    ESCX_ASSERT_TRUE(event_out != (ESCX_QueuedEvent *)0);
    if (ESCX_EventQueueIsEmpty(queue) != ESCX_FALSE) return ESCX_FALSE;
    *event_out = queue->items[queue->front];
    queue->items[queue->front].event_data = (void *)0;
    queue->front = (queue->front + 1U) % ESCX_QUEUE_SIZE;
    --queue->count;
    return ESCX_TRUE;
}
