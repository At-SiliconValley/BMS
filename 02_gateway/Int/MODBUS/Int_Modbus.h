#ifndef __INT_MODBUS_H__
#define __INT_MODBUS_H__
#include "usart.h"
#include "Com_Delay.h"
#include "stdio.h"
#include "mbcrc.h"
#include "FreeRTOS.h"
#include "task.h"
// 网关主设备通过此方法发送命令
void Int_Modbus_SendCMD(uint8_t *cmd, uint16_t length);

// 1.读取从设备的线圈数组的数据
void Int_Modbus_ReadCoil(uint8_t dev_id, uint16_t start_addr, uint16_t num);

// 2.读取离散数组的数据
void Int_Modbus_ReadInputStatus(uint8_t dev_id, uint16_t start_addr, uint16_t num);

// 3.读取保持寄存器
void Int_Modbus_ReadHoldingReg(uint8_t dev_id, uint16_t start_addr, uint16_t num);

// 4.读取输入寄存器
void Int_Modbus_ReadInputReg(uint8_t dev_id, uint16_t start_addr, uint16_t num);

// 5.向线圈数组写入单个元素
void Int_Modbus_WriteCoil(uint8_t dev_id, uint16_t start_addr, uint8_t value);

// 6.向保持寄存器写入单个元素

void Int_Modbus_WriteHoldingReg(uint8_t dev_id, uint16_t start_addr, uint16_t value);


//7.线圈数组同时写入多个元素
void Int_Modbus_WriteCoils(uint8_t dev_id, uint16_t start_addr, uint16_t num, uint8_t *value);

//8.保持寄存器写入多个元素
void Int_Modbus_WriteHoldingRegs16(uint8_t dev_id, uint16_t start_addr, uint16_t *reg_data, uint16_t reg_len);

#endif /* __INT_MODBUS_H__ */
