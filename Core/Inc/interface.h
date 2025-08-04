/*
 * interface.h
 *
 *  Created on: Jul 31, 2025
 *      Author: klors
 */

#ifndef INC_INTERFACE_H_
#define INC_INTERFACE_H_

#include "main.h"

// 页面类型声明
typedef enum {
    PAGE_MAIN,
    PAGE_DDS,
    PAGE_SDD,
    PAGE_LEARN,
    PAGE_EXPLORE,
    PAGE_NUM,
    PAGE_Author,
    PAGE_INFO
} PageType;
// 当前页面变量声明
extern PageType current_page;

// numgui参数类型声明
typedef enum {
    NUM_NONE,
    NUM_FREQ,
    NUM_AMP,
    NUM_PHASE
} NumType;
// 当前numgui参数变量声明
extern NumType num_type;

// 各界面函数声明
void maingui(void);
void numgui(void);
void ddsgui(void);
void learngui(void);
void exploregui(void);
void infogui(void);

#endif /* INC_INTERFACE_H_ */
