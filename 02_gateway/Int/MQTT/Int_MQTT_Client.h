#ifndef __INT_MQTT_CLIENT_H__
#define __INT_MQTT_CLIENT_H__
#include "Int_W5500.h"
#include "MQTTClient.h"
#include "socket.h"
#include "Com_Debug.h"
// 1.初始化
void Int_MQTTClient_Init(void);
// 2.处理网络信息方法
void Int_MQTTClient_Yield(void);
// 3.MQTT客户端发布消息
void Int_MQTTClient_PublishMessage(uint8_t *send_data);

// 4.提供注册回调方法
typedef void (*CallBack)(char *mqtt_message);

void Int_MQTTClientRegisterCallBack(CallBack cb);

#endif
