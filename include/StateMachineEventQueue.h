#ifndef ESCX_STATE_MACHINE_EVENT_QUEUE_H
#define ESCX_STATE_MACHINE_EVENT_QUEUE_H

#include "DataTypes.h"
#include "ESCXConfig.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct ESCX_QueuedEvent
{
    ESCX_BYTE_T event_id;
    ESCX_BYTE_T event_type;
    ESCX_BYTE_T origin_type;
    ESCX_BYTE_T invoke_id;
    ESCX_UINT16_T send_id;
#if ESCX_ENABLE_EVENT_DATA
    void *event_data;
#endif
} ESCX_QueuedEvent;

typedef struct ESCX_EventQueue
{
    ESCX_QueuedEvent items[ESCX_QUEUE_SIZE];
    ESCX_BYTE_T front;
    ESCX_BYTE_T count;
} ESCX_EventQueue;

void ESCX_EventQueueInit(ESCX_EventQueue *queue);
ESCX_BOOL_T ESCX_EventQueueIsEmpty(const ESCX_EventQueue *queue);
ESCX_BOOL_T ESCX_EventQueueIsFull(const ESCX_EventQueue *queue);
ESCX_BOOL_T ESCX_EventQueuePush(ESCX_EventQueue *queue, ESCX_BYTE_T event_id, void *event_data);
ESCX_BOOL_T ESCX_EventQueuePushEx(ESCX_EventQueue *queue, ESCX_BYTE_T event_id,
    ESCX_BYTE_T event_type, ESCX_BYTE_T origin_type, ESCX_BYTE_T invoke_id,
    ESCX_UINT16_T send_id, void *event_data);
ESCX_BOOL_T ESCX_EventQueuePop(ESCX_EventQueue *queue, ESCX_QueuedEvent *event_out);

#ifdef __cplusplus
}
#endif
#endif
