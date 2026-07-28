#ifndef __INT_CAN_H__
#define __INT_CAN_H__
#include "can.h"
#include "Com_Delay.h"
#include "stdio.h"

// 用一个结构体标识一个报文
typedef struct
{
    // 报文:标准ID
    uint32_t StdId;
    // 标准、拓展
    uint32_t IDE;
    // 数据、远程
    uint32_t RTR;
    // 数据长度
    uint32_t DLC;
    //数据
    uint8_t data[8];
} Can_Frame_Data;

// 节点的消息ID
#define CAN_GATEWAY_TO_CONSOLE 0x0001
#define CAN_CONSOLE_TO_GATEWAY 0x0002



// 1.初始化CAN控制器
void Int_CAN_Init(void);

// 2.用于发送报文
void Int_CAN_SendData(uint32_t stdID, uint8_t *data, uint32_t DLC);

// 3.用于接受总线上报文
void Int_CAN_ReceiveData(Can_Frame_Data buffers[],uint32_t * receive_data_number);

#endif /* __INT_CAN_H__ */
