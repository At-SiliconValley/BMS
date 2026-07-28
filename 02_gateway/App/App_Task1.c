#include "App_Task1.h"
void task1(void *params)
{
    // 0.任务1注册回调函数,目的获取MQTT客户端订阅消息
    Int_MQTTClientRegisterCallBack(task1_callBack);
    // 1.MQTT客户端初始化一次
    Int_MQTTClient_Init();
    while (1)
    {
        // 2.客户端处理数据
        Int_MQTTClient_Yield();

        vTaskDelay(50);
    }
}

// 网关下发数据的任务
void task1_callBack(char *mqtt_message)
{

    // 1.JSON解析
    cJSON *root = cJSON_Parse(mqtt_message);

    // 解析type
    // BMS:charge充电  discharge:放电
    char *type = cJSON_GetObjectItem(root, "type")->valuestring;
    char *status = cJSON_GetObjectItem(root, "status")->valuestring;
    cJSON_Delete(root);

    // 分析:网关CAN节点如何向总线发送数据帧,BMS能区分.1.充电、放电  2.开启还是关闭
    // 消息ID【1】 +data【1】
    // data:[2]
}
