#pragma once
#include <Arduino.h>
#include <Client.h>

void start_sim7600_task();
bool sim7600_is_net_up();
Client& sim7600_client();
