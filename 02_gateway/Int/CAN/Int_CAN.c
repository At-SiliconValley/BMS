#include "Int_CAN.h"

// 1.初始化
void Int_CAN_Init(void)
{

    // 1.CAN控制器过滤器配置
    CAN_FilterTypeDef can_filter;
    // F103系列,单CAN,过滤器最多14[0-13]
    can_filter.FilterBank = 8;
    // 过滤器开启----->接受报文相关配置
    can_filter.FilterActivation = CAN_FILTER_ENABLE;
    // 过滤器模式设置:list精准匹配 mask:模糊匹配
    can_filter.FilterMode = CAN_FILTERMODE_IDLIST;
    // 接受报文:仲裁段的消息ID->11位标准  29:拓展版
    can_filter.FilterScale = CAN_FILTERSCALE_16BIT;
    // 设置接受数据的FIFO
    can_filter.FilterFIFOAssignment = CAN_FILTER_FIFO0;

    HAL_CAN_ConfigFilter(&hcan, &can_filter);

    // 2.开启CAN
    HAL_CAN_Start(&hcan);
}

// 2.发送报文
void Int_CAN_SendData(uint32_t stdID, uint8_t *data, uint32_t DLC)
{

    // 1.先判断发送邮箱是否空闲,有空闲邮箱,发送报文,没有等待!
    while (HAL_CAN_GetTxMailboxesFreeLevel(&hcan) == 0)
    {
        Com_Delay_ms(10);
    }

    // 主备报文:结构体+数组
    CAN_TxHeaderTypeDef tx_header;
    // 数据帧的:消息ID
    tx_header.StdId = stdID;
    // 发送报文:数据帧
    tx_header.RTR = CAN_RTR_DATA;
    // 标准数据帧
    tx_header.IDE = CAN_ID_STD;
    tx_header.DLC = DLC;

    uint32_t sendMessageEmail = 6;
    // 2.有空闲邮箱!!!!!
    HAL_CAN_AddTxMessage(&hcan, &tx_header, data, &sendMessageEmail);

    // 测试代码:把发送数据打印出来
    printf("send data:");
    for (uint8_t i = 0; i < DLC; i++)
    {
        printf("%d ", data[i]);
    }
    printf("\r\n");
}

// 3.接受CAN总线上的报文
void Int_CAN_ReceiveData(Can_Frame_Data buffers[], uint32_t *receive_data_number)
{

    // 1.先判断FIFI0接收到报文数据
    *receive_data_number = HAL_CAN_GetRxFifoFillLevel(&hcan, CAN_FILTER_FIFO0);

    // 2.提取报文数据
    for (uint8_t i = 0; i < *receive_data_number; i++)
    {
        // 此方法调用一次接受一个数据帧
        CAN_RxHeaderTypeDef rxHeader;
        HAL_CAN_GetRxMessage(&hcan, CAN_FILTER_FIFO0, &rxHeader, buffers[i].data);
        // 把接收到数据帧给外面
        buffers[i].StdId = rxHeader.StdId;
        buffers[i].RTR = rxHeader.RTR;
        buffers[i].IDE = rxHeader.IDE;
        buffers[i].DLC = rxHeader.DLC;

        // 测试代码
        printf("receive data");
        for (uint8_t j = 0; j < buffers[i].DLC; j++)
        {
            printf("%d ", buffers[i].data[j]);
        }
        printf("\r\n");
    }
}
