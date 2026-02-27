#pragma once
#include <Arduino.h>
#include <freertos/FreeRTOS.h>
#include <freertos/queue.h>

#include "log_types.h"

extern QueueHandle_t gLogQueue;

bool logger_init_sd();
void logger_task(void*);

void log_enqueue(const LogRecord& rec);