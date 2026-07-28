#include "Int_Modbus.h"

void Int_Modbus_SendCMD(uint8_t *cmd, uint16_t length)
{
    // 主设备通过串口2,将modbus命令发送给从设备->对于四个数组进行操作
    HAL_UART_Transmit(&huart2, cmd, length, 1000);
    // RS485半双工,命令发出去以后(不能再无脑继续发送命令),等相应的数据返回,再发命令
    Com_Delay_ms(100);

    // 测试代码:发送命令输出查看
    printf("gateway send cmd");
    for (uint8_t i = 0; i < length; i++)
    {
        printf("%02x ", cmd[i]);
    }
    printf("\r\n");
}

// 1.读取从设备的线圈数组的数据
// dev_id = 5  start_addr = 2    num = 4
void Int_Modbus_ReadCoil(uint8_t dev_id, uint16_t start_addr, uint16_t num)
{

    // 1.动态'组装',读取线圈数组命令
    uint8_t cmd[8] = {0};
    cmd[0] = dev_id;                   // 从机地址
    cmd[1] = 0x01;                     // 功能码,读取从设备线圈数组的数据
    cmd[2] = (start_addr >> 8) & 0xFF; // 读取地址
    cmd[3] = start_addr & 0xFF;
    cmd[4] = (num >> 8) & 0xFF; // 读取数量
    cmd[5] = num & 0xFF;
    uint16_t crc = usMBCRC16(cmd, 6);
    cmd[6] = crc & 0xFF; // crc校验结果
    cmd[7] = (crc >> 8) & 0xFF;

    // 2.发送读取线圈的命令
    Int_Modbus_SendCMD(cmd, 8);
}

// 2.读取离散数组的数据
void Int_Modbus_ReadInputStatus(uint8_t dev_id, uint16_t start_addr, uint16_t num)
{

    // 1.动态'组装',读取离散数组命令
    uint8_t cmd[8] = {0};
    cmd[0] = dev_id;                   // 从机地址
    cmd[1] = 0x02;                     // 功能码,读取从设备线圈数组的数据
    cmd[2] = (start_addr >> 8) & 0xFF; // 读取地址
    cmd[3] = start_addr & 0xFF;
    cmd[4] = (num >> 8) & 0xFF; // 读取数量
    cmd[5] = num & 0xFF;
    uint16_t crc = usMBCRC16(cmd, 6);
    cmd[6] = crc & 0xFF; // crc校验结果
    cmd[7] = (crc >> 8) & 0xFF;

    // 2.发送读取离散的命令
    Int_Modbus_SendCMD(cmd, 8);
}

// 3.读取保持寄存器
void Int_Modbus_ReadHoldingReg(uint8_t dev_id, uint16_t start_addr, uint16_t num)
{
    // 1.动态'组装',读取保持数组命令
    uint8_t cmd[8] = {0};
    cmd[0] = dev_id;                   // 从机地址
    cmd[1] = 0x03;                     // 功能码,读取从设备保持数组的数据
    cmd[2] = (start_addr >> 8) & 0xFF; // 读取地址
    cmd[3] = start_addr & 0xFF;
    cmd[4] = (num >> 8) & 0xFF; // 读取数量
    cmd[5] = num & 0xFF;
    uint16_t crc = usMBCRC16(cmd, 6);
    cmd[6] = crc & 0xFF; // crc校验结果
    cmd[7] = (crc >> 8) & 0xFF;

    // 2.发送读取保持的命令
    Int_Modbus_SendCMD(cmd, 8);
}

// 4.读取输入寄存器
void Int_Modbus_ReadInputReg(uint8_t dev_id, uint16_t start_addr, uint16_t num)
{
    // 1.动态'组装',读取输入数组命令
    uint8_t cmd[8] = {0};
    cmd[0] = dev_id;                   // 从机地址
    cmd[1] = 0x04;                     // 功能码,读取从设备输入数组的数据
    cmd[2] = (start_addr >> 8) & 0xFF; // 读取地址
    cmd[3] = start_addr & 0xFF;
    cmd[4] = (num >> 8) & 0xFF; // 读取数量
    cmd[5] = num & 0xFF;
    uint16_t crc = usMBCRC16(cmd, 6);
    cmd[6] = crc & 0xFF; // crc校验结果
    cmd[7] = (crc >> 8) & 0xFF;

    // 2.发送读取输入的命令
    Int_Modbus_SendCMD(cmd, 8);
}

// 5.向线圈数组写入单个元素
//  dev_id = 5    start_addr= 2   value = 1
void Int_Modbus_WriteCoil(uint8_t dev_id, uint16_t start_addr, uint8_t value)
{

    // 1.动态'组装',读取输入数组命令
    uint8_t cmd[8] = {0};
    cmd[0] = dev_id;                   // 从机地址
    cmd[1] = 0x05;                     // 功能码,向线圈数组写入单个元素
    cmd[2] = (start_addr >> 8) & 0xFF; // 读取地址
    cmd[3] = start_addr & 0xFF;

    // 向线圈数组写入内容判断: value = 1  0xFF00   value = 0  0x0000
    if (value)
    {
        cmd[4] = 0xFF;
    }
    else
    {
        cmd[4] = 0x00;
    }
    cmd[5] = 0x00;
    uint16_t crc = usMBCRC16(cmd, 6);
    cmd[6] = crc & 0xFF; // crc校验结果
    cmd[7] = (crc >> 8) & 0xFF;
    // 2.发送读取输入的命令
    Int_Modbus_SendCMD(cmd, 8);
}

// 6.向保持寄存器写入单个元素
// dev_id = 5  start_addr = 2   value=0x6789
void Int_Modbus_WriteHoldingReg(uint8_t dev_id, uint16_t start_addr, uint16_t value)
{
    // 1.动态'组装',保持寄存器写入单个元素
    uint8_t cmd[8] = {0};
    cmd[0] = dev_id;                   // 从机地址
    cmd[1] = 0x06;                     // 功能码,向线圈数组写入单个元素
    cmd[2] = (start_addr >> 8) & 0xFF; // 读取地址
    cmd[3] = start_addr & 0xFF;
    cmd[4] = (value >> 8) & 0xFF;
    cmd[5] = value & 0xFF;
    uint16_t crc = usMBCRC16(cmd, 6);
    cmd[6] = crc & 0xFF; // crc校验结果
    cmd[7] = (crc >> 8) & 0xFF;
    // 2.发送读取输入的命令
    Int_Modbus_SendCMD(cmd, 8);
}

// 7.线圈数组同时写入多个元素
// dev_id = 5  start_addr = 0  num = 9   value = "1100,0101,1100,1010"
//  {0x05,0x0F,0x00,0x00,0x00,0x09,0x02, 10100011,000000001,CRC}
void Int_Modbus_WriteCoils(uint8_t dev_id, uint16_t start_addr, uint16_t num, uint8_t *value)
{

    // 1.计算出需要写入数据到底需要几个字节
    uint8_t sendByte_num = num % 8 == 0 ? (num / 8) : (num / 8 + 1);
    // 2.计算出命令需要总字节数
    uint8_t total_number = sendByte_num + 9;
    // 3.创建动态数组,整理写入线圈命令
    uint8_t *cmd = pvPortMalloc(total_number); // uint_8 cmd[total_number] = {0};
    cmd[0] = dev_id;
    cmd[1] = 0x0F;
    cmd[2] = (start_addr >> 8) & 0xFF;
    cmd[3] = start_addr & 0xFF;
    cmd[4] = (num >> 8) & 0xFF;
    cmd[5] = num & 0xFF;
    cmd[6] = sendByte_num;

    // 4.携带的线圈的数据
    // 外层表示字节
    for (uint8_t i = 0; i < sendByte_num; i++)
    {
        uint8_t byte = 0;
        // 表示每一个字节位设置
        for (uint8_t j = 0; j < 8; j++)
        {
            // value:取出来某一位是1,在进行运算!!!!!!!
            if (value[j + i * 8])
            {
                byte |= (value[j + i * 8] << j);
            }
        }
        cmd[7 + i] = byte;
    }

    // 5.计算CRC
    uint16_t crc = usMBCRC16(cmd, total_number - 2);
    cmd[total_number - 2] = crc & 0xFF; // crc校验结果
    cmd[total_number - 1] = (crc >> 8) & 0xFF;

    // 6.发送命令
    Int_Modbus_SendCMD(cmd, total_number);
    // 7.释放动态内存,如果freeRTOS使用内存管理方法.没有在考虑malloc
    vPortFree(cmd);
}

// 8.保持寄存器写入多个元素
// dev_id = 5  start_addr = 0 reg_len = 2   reg_data =0x1234 0x4567
void Int_Modbus_WriteHoldingRegs16(uint8_t dev_id, uint16_t start_addr, uint16_t *reg_data, uint16_t reg_len)
{

    // 1.计算出命令发送过去数据字节个数
    uint16_t send_byte_num = reg_len * 2;
    uint16_t total_byte_num = send_byte_num + 9;
    // 2.开辟空间
    uint8_t *cmd = pvPortMalloc(total_byte_num);
    cmd[0] = dev_id;
    cmd[1] = 0x10;
    cmd[2] = (start_addr >> 8) & 0xFF;
    cmd[3] = start_addr & 0xFF;
    cmd[4] = (reg_len >> 8) & 0xFF;
    cmd[5] = reg_len & 0xFF;
    cmd[6] = send_byte_num;

    // 3.写入数据
    for (uint16_t i = 0; i < reg_len; i++)
    {

        cmd[7 + i * 2] = (reg_data[i] >> 8) & 0xFF;
        cmd[8 + i * 2] = reg_data[i] & 0xFF;
    }

    // 4.计算CRC校验结果
    uint16_t crc = usMBCRC16(cmd, total_byte_num - 2);
    cmd[total_byte_num - 2] = crc & 0xFF;
    cmd[total_byte_num - 1] = (crc >> 8) & 0xFF;

    // 5.发送命令
    Int_Modbus_SendCMD(cmd, total_byte_num);
    vPortFree(cmd);
}
