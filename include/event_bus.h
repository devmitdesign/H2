\
#pragma once
#include <Arduino.h>
#include <freertos/FreeRTOS.h>
#include <freertos/queue.h>
#include <freertos/event_groups.h>
#include "log_record.h"

extern QueueHandle_t gLogQueue;
extern QueueHandle_t gStreamQueue;
extern EventGroupHandle_t gSysEvents;

static constexpr EventBits_t EV_SD_READY = (1<<0);
static constexpr EventBits_t EV_WIFI_UP  = (1<<1);
static constexpr EventBits_t EV_MQTT_UP  = (1<<2);
static constexpr EventBits_t EV_LTE_UP   = (1<<3);
static constexpr EventBits_t EV_NET_UP   = (1<<4);
static constexpr EventBits_t EV_PWR_FAIL = (1<<5);
static constexpr EventBits_t EV_CAN1_UP  = (1<<6);
static constexpr EventBits_t EV_CAN2_UP  = (1<<7);

bool event_bus_init();
bool logq_push(const LogRecord32& rec, TickType_t toTicks = 0);
bool streamq_push(const LogRecord32& rec);
