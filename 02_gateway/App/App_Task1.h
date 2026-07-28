#ifndef __APP_TASK1_H__
#define __APP_TASK1_H__
#include "FreeRTOS.h"
#include "task.h"
#include "Int_MQTT_Client.h"
#include "cJSON.h"
#include "Int_Modbus.h"
#include "semphr.h"
#include "Int_CAN.h"
#define TASK1_STACKDEPTH 1024
#define TASK1_PRIORITY 3


void task1(void *params);

//定义一个函数:用于接受mqtt接收到订阅的消息
void task1_callBack(char * mqtt_message);
#endif /* __APP_TASK1_H__ */
