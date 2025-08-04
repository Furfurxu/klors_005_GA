/**
 ****************************************************************************************************
 * @file        beep.h
 * @author      正点原子团队(ALIENTEK)
 * @version     V1.0
 * @date        2024-01-01
 * @brief       蜂鸣器 驱动代码
 * @license     Copyright (c) 2020-2032, 广州市星翼电子科技有限公司
 ****************************************************************************************************
 * @attention
 *
 * 实验平台:正点原子 STM32F103开发板
 * 在线视频:www.yuanzige.com
 * 技术论坛:www.openedv.com
 * 公司网址:www.alientek.com
 * 购买地址:openedv.taobao.com
 *
 * 修改说明
 * V1.0 20240101
 * 第一次发布
 *
 ****************************************************************************************************
 */

#ifndef BEEP_BEEP_H_
#define BEEP_BEEP_H_
#include "../../SYSTEM/sys/sys.h"

/******************************************************************************************/

/* 蜂鸣器控制 */
#define BEEP(x)         do{ x ? \
                            HAL_GPIO_WritePin(BEEP_GPIO_Port, BEEP_Pin, GPIO_PIN_SET) : \
                            HAL_GPIO_WritePin(BEEP_GPIO_Port, BEEP_Pin, GPIO_PIN_RESET); \
                        }while(0)

/* BEEP状态翻转 */
#define BEEP_TOGGLE()   do{ HAL_GPIO_TogglePin(BEEP_GPIO_Port, BEEP_Pin); }while(0)     /* BEEP = !BEEP */


#endif /* BEEP_BEEP_H_ */
