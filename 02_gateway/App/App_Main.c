#include "App_Main.h"
TaskHandle_t task1_handler;
void App_Main_Start(void)
{

  // 1.创建一个动态任务,调度器开启!!!
  xTaskCreate(task1, "task1", TASK1_STACKDEPTH, NULL, TASK1_PRIORITY, &task1_handler);
  // 2.开启调度器
  vTaskStartScheduler();
}
