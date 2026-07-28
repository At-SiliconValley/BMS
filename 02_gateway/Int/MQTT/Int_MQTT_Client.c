#include "Int_MQTT_Client.h"
CallBack gloal_cb;

// 创建通道的参数
Network mqtt_network;
uint8_t SERVER_IP[4] = {192, 168, 50, 51}; // 每天来的时候这玩意更新!!!!先查看一下自己的IP地址!
#define SERVER_PORT 1883
#define SN 0

// MQTT客户端的参数
MQTTClient mqtt_client;
#define MAXSIZE 1024
uint8_t mqtt_publish_buffer[MAXSIZE];
uint8_t mqtt_subscribe_buffer[MAXSIZE];

// 订阅消息的主题
#define TOPIC1 "CONSOLE_TO_BMS"
#define TOPIC2 "gateway_to_console"

void mqtt_clinet_receiveCallBack(MessageData *receive_buffer);

// 创建通信通道
static void Int_MQTTClient_CreateSocketConnectServer(void);
// 创建MQTT客户端
static void Int_MQTTClient_CreateClientConnectServer(void);

void Int_MQTTClient_Init(void)
{
    // 1.MQTT客户端,发布消息、订阅消息(broker),必须联网!!!!
    Int_W5500_Init();

    // 2.创建通信通道,链接服务器
    Int_MQTTClient_CreateSocketConnectServer();

    // 3.创建MQTT客户端
    Int_MQTTClient_CreateClientConnectServer();
}

// 类似于:电脑插上网线,可以上网
static void Int_MQTTClient_CreateSocketConnectServer(void)
{

    // 1.创建通信的SOCKET通道
    NewNetwork(&mqtt_network, SN);

    int result = ConnectNetwork(&mqtt_network, SERVER_IP, SERVER_PORT);
    if (result == SOCK_OK)
    {
        COM_DEBUG_LN("创建通道成功,通道与服务器建立链接成功");
    }
    else
    {
        COM_DEBUG_LN("创建通道成功,通道与服务器建立链接失败");
    }
}

// 类似于:电脑安装"MQTT客户端"
static void Int_MQTTClient_CreateClientConnectServer(void)
{

    // 1.MQTT客户端初始化
    // 1.客户端句柄  2.通道  3.缓冲区与大小
    MQTTClientInit(&mqtt_client, &mqtt_network, 1000, mqtt_publish_buffer, MAXSIZE, mqtt_subscribe_buffer, MAXSIZE);

    // 2.MQTT客户端连接broker服务器
    MQTTPacket_connectData connect_data = MQTTPacket_connectData_initializer;
    int result = MQTTConnect(&mqtt_client, &connect_data);
    if (result == SUCCESS)
    {
        COM_DEBUG_LN("MQTT客户端连接BROKER成功");
    }
    else
    {
        COM_DEBUG_LN("MQTT客户端连接BROKER失败");
    }

    // 3.订阅消息
    MQTTSubscribe(&mqtt_client, TOPIC1, QOS0, mqtt_clinet_receiveCallBack);
}

// 当MQTT客户端订阅消息有了,次函数执行一次!!!!!
void mqtt_clinet_receiveCallBack(MessageData *receive_buffer)
{
    // 传递参数
    gloal_cb((char *)receive_buffer->message->payload);
}

void Int_MQTTClient_Yield(void)
{
    MQTTYield(&mqtt_client, 1000);
}

// 发布消息
void Int_MQTTClient_PublishMessage(uint8_t *send_data)
{

    // 发送消息
    MQTTMessage msg;
    // 发送消息内容
    msg.payload = send_data;
    // 发送消息长度
    msg.payloadlen = strlen((char *)send_data);
    // 发送消息质量等级
    msg.qos = QOS0;
    MQTTPublish(&mqtt_client, TOPIC2, &msg);
}

// 注册回调
void Int_MQTTClientRegisterCallBack(CallBack cb)
{
    gloal_cb = cb;
}
