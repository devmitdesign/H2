#include "event_bus.h"
#include "config.h"

QueueHandle_t gLogQueue = nullptr;
QueueHandle_t gStreamQueue = nullptr;
EventGroupHandle_t gSysEvents = nullptr;

bool event_bus_init() {
  gSysEvents = xEventGroupCreate();
  if (!gSysEvents) return false;
  gLogQueue = xQueueCreate(LOG_QUEUE_DEPTH, sizeof(LogRecord32));
  if (!gLogQueue) return false;
  gStreamQueue = xQueueCreate(STREAM_QUEUE_DEPTH, sizeof(LogRecord32));
  if (!gStreamQueue) return false;
  return true;
}

bool logq_push(const LogRecord32& rec, TickType_t toTicks) {
  return gLogQueue && xQueueSend(gLogQueue, &rec, toTicks) == pdTRUE;
}
bool streamq_push(const LogRecord32& rec) {
  return gStreamQueue && xQueueSend(gStreamQueue, &rec, 0) == pdTRUE;
}
