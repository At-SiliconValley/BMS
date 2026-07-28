#include "Int_W5500.h"

wiz_NetInfo default_net_info = {
    .mac = {0x00, 0x08, 0xdc, 0x12, 0x22, 0x12},
    .ip = {192, 168, 50, 170},
    .gw = {192, 168, 50, 1},
    .sn = {255, 255, 255, 0},
};

// 测试函数:判断是否加入局域网
static void print_network_info(void)
{
    uint8_t curip[4] = {0};
    uint8_t curgw[4] = {0};
    uint8_t curmask[4] = {0};
    uint8_t curmac[6] = {0};
    getGAR(curgw);
    getSUBR(curmask);
    getSHAR(curmac);
    getSIPR(curip);
    printf("w5500 ip:%d.%d.%d.%d\n", curip[0], curip[1], curip[2], curip[3]);
    printf("w5500 gw:%d.%d.%d.%d\n", curgw[0], curgw[1], curgw[2], curgw[3]);
    printf("w5500 mask:%d.%d.%d.%d\n", curmask[0], curmask[1], curmask[2], curmask[3]);
    // 打印当前mac
    printf("w5500 mac:%02x:%02x:%02x:%02x:%02x:%02x\n", curmac[0], curmac[1], curmac[2], curmac[3], curmac[4], curmac[5]);
}

// 1.初始化方法
void Int_W5500_Init(void)
{
    // 1.w5500结构体成员进行赋值->补齐SPI驱动底层逻辑
    user_register_callBack();

    // 2.复位w5500
    HAL_GPIO_WritePin(W5500_RST_GPIO_Port, W5500_RST_Pin, GPIO_PIN_RESET);
    Com_Delay_ms(10);
    HAL_GPIO_WritePin(W5500_RST_GPIO_Port, W5500_RST_Pin, GPIO_PIN_SET);
    Com_Delay_ms(10);

    // 3.配置加入局域网的网络信息
    setSHAR(default_net_info.mac); // MAC地址
    setGAR(default_net_info.gw);   // 网关
    setSUBR(default_net_info.sn);  // 子网掩码
    setSIPR(default_net_info.ip);  // IP地址

    Com_Delay_s(10);
    print_network_info();
}
