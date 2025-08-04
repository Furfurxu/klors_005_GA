/*
 * interface.c
 *
 *  Created on: Jul 31, 2025
 *      Author: klors
 *  for TI 2025 Q(G)
 */

#include "../../BSP/LED/led.h"
#include "../../BSP/LCD/lcd.h"
#include "../../BSP/TOUCH/touch.h"
#include "../../BSP/KEY/key.h"
#include "../../SYSTEM/delay/delay.h"

#include "interface.h"
#include "AD9959.h"
#include "adc.h"
#include "tim.h"
#include <string.h>
#include <stdlib.h>

#define INPUT_MAX_LEN 10

#define SWEEP_START_FREQ 1000
#define SWEEP_END_FREQ   50000
#define SWEEP_STEP       200
#define SWEEP_POINTS     (((SWEEP_END_FREQ)-(SWEEP_START_FREQ))/(SWEEP_STEP)+1)

uint32_t dds_value = 0;     //ddsgui
uint32_t dds_freq[4] = {0, 0, 0, 0};
uint16_t dds_amp[4] = {0, 0, 0, 0};
uint16_t dds_phase[4] = {0, 0, 0, 0};
uint8_t dds_channel = 0;

uint32_t sweep_freqs[SWEEP_POINTS] = {0};   //扫频
uint16_t sweep_amps[SWEEP_POINTS] = {0};

PageType current_page = PAGE_MAIN;
PageType last_page;
NumType num_type = NUM_NONE;
void maingui()
{
    lcd_clear(GRAY);
    g_back_color = GRAY;
    lcd_show_string(88, 25, 100, 40, 32, "Menu", BLUE);

    // 主功能按钮参数
    uint16_t btn_w = 180, btn_h = 40;
    uint16_t btn_x = 30;
    uint16_t btn1_y = 100;
    uint16_t btn2_y = 150;
    uint16_t btn3_y = 200;
    uint8_t font_size = 24;

    // 居中显示的y坐标
    uint16_t text_y_offset = (btn_h - font_size) / 2;

    // DDS按键
    lcd_fill(btn_x, btn1_y, btn_x + btn_w / 2 - 6, btn1_y + btn_h, LGRAY);
    g_back_color = LGRAY;
    lcd_draw_rectangle(btn_x, btn1_y, btn_x + btn_w / 2 - 6, btn1_y + btn_h, BLACK);
    lcd_show_string(btn_x + 10, btn1_y + text_y_offset, btn_w, btn_h, font_size, "DDS", BLACK);

    // SDD按键
    lcd_fill(btn_x + btn_w / 2 + 6, btn1_y, btn_x + btn_w, btn1_y + btn_h, LGRAY);
    g_back_color = LGRAY;
    lcd_draw_rectangle(btn_x + btn_w / 2 + 6, btn1_y, btn_x + btn_w, btn1_y + btn_h, BLACK);
    lcd_show_string(btn_x + btn_w / 2 + 20, btn1_y + text_y_offset, btn_w, btn_h, font_size, "SDD", BLACK);
    
    // Learn按键
    lcd_fill(btn_x, btn2_y, btn_x + btn_w, btn2_y + btn_h, LGRAY);
    g_back_color = LGRAY;
    lcd_draw_rectangle(btn_x, btn2_y, btn_x + btn_w, btn2_y + btn_h, BLACK);
    lcd_show_string(btn_x + (btn_w - 10 * strlen("Learn")) / 2, btn2_y + text_y_offset, btn_w, btn_h, font_size, "Learn", BLACK);

    // Explore按键
    lcd_fill(btn_x, btn3_y, btn_x + btn_w, btn3_y + btn_h, LGRAY);
    g_back_color = LGRAY;
    lcd_draw_rectangle(btn_x, btn3_y, btn_x + btn_w, btn3_y + btn_h, BLACK);
    lcd_show_string(btn_x + (btn_w - 10 * strlen("Explore")) / 2, btn3_y + text_y_offset, btn_w, btn_h, font_size, "Explore", BLACK);

    // 左下角彩蛋字（INFO）
	g_back_color = GRAY;
    lcd_show_string(3, 305, 40, 16, 12, "INFO", RED);

    // Author按键
    lcd_show_string(201, 305, 40, 16, 12, "Author", RED);



    // 你可以在主循环中添加触摸检测，实现界面跳转
    while (1) {
        tp_dev.scan(0);
        if (tp_dev.sta & TP_PRES_DOWN) {
			LED0(0);
            uint16_t tx = tp_dev.x[0], ty = tp_dev.y[0];
            // DDS按键
            if (tx > btn_x && tx < btn1_y && btn_x + btn_w / 2 - 6 && ty > btn1_y && ty < btn1_y + btn_h) {
                last_page = current_page; // 保存当前页面   
                current_page = PAGE_DDS;
                return;
            }
            // SDD按键
            if (tx > btn_x + btn_w / 2 + 6 && tx < btn_x + btn_w && ty > btn1_y && ty < btn1_y + btn_h) {
                last_page = current_page; // 保存当前页面   
                current_page = PAGE_SDD;
                return;
            }
            // learn按键
            if (tx > btn_x && tx < btn_x + btn_w && ty > btn2_y && ty < btn2_y + btn_h) {
                last_page = current_page; // 保存当前页面   
                current_page = PAGE_LEARN;
                return;
            }
            // Explore按键
            if (tx > btn_x && tx < btn_x + btn_w && ty > btn3_y && ty < btn3_y + btn_h) {
                last_page = current_page; // 保存当前页面   
                current_page = PAGE_EXPLORE;
                return;
            }
            // INFO彩蛋
            if (tx > 0 && tx < 30 && ty > 303 && ty < 320) {
                last_page = current_page; // 保存当前页面   
                current_page = PAGE_INFO;
                return;
            }
            // Author按键
            if (tx > 200 && tx < 240 && ty > 303 && ty < 320) {
                last_page = current_page; // 保存当前页面   
                current_page = PAGE_Author;
                return;
            }

            while (tp_dev.sta & TP_PRES_DOWN) tp_dev.scan(0);
        }
		else {
			LED0(1);
		}
        delay_ms(20);
    }
}

void ddsgui(void)
{
    lcd_clear(GRAY);
    g_back_color = GRAY;
    lcd_show_string(96, 20, 100, 40, 32, "DDS", BLUE);

    // 返回按钮
    uint16_t back_x = 10, back_y = 10, back_w = 40, back_h = 40;
    lcd_fill(back_x, back_y, back_x + back_w, back_y + back_h, LGRAY);
    g_back_color = LGRAY;
    lcd_draw_rectangle(back_x, back_y, back_x + back_w, back_y + back_h, BLACK);
    lcd_show_string(back_x + 10, back_y + 4, 16, 24, 32, "<", BLACK);

    // 通道切换按钮
    uint16_t chg_x = 160, chg_y = 20, chg_w = 65, chg_h = 40;
    lcd_fill(chg_x, chg_y, chg_x + chg_w, chg_y + chg_h, YELLOW);
    g_back_color = YELLOW;
    lcd_draw_rectangle(chg_x, chg_y, chg_x + chg_w, chg_y + chg_h, BLACK);
    char chstr[16];
    sprintf(chstr, "CH:%d", dds_channel);
    lcd_show_string(chg_x + 10, chg_y + 8, chg_w - 20, chg_h - 16, 24, chstr, BLUE);

    // 按钮参数
    uint16_t btn_w = 200, btn_h = 60;
    uint16_t btn_x = 20;
    uint16_t btn1_y = 70;
    uint16_t btn2_y = 150;
    uint16_t btn3_y = 230;
    uint8_t font_size = 16;

    char value_str[32];

    // Frep 按钮
    lcd_fill(btn_x, btn1_y, btn_x + btn_w, btn1_y + btn_h, LGRAY);
    g_back_color = LGRAY;
    lcd_draw_rectangle(btn_x, btn1_y, btn_x + btn_w, btn1_y + btn_h, BLACK);
    lcd_show_string(btn_x + 10, btn1_y + 8, btn_w - 20, btn_h - 16, 24, "Frep", BLACK);
    sprintf(value_str, "%lu Hz", dds_freq[dds_channel]);
    lcd_show_string(btn_x + 100, btn1_y + 35, btn_w - 110, 20, font_size, value_str, RED);

    // Amplitudea按键
    lcd_fill(btn_x, btn2_y, btn_x + btn_w, btn2_y + btn_h, LGRAY);
    g_back_color = LGRAY;
    lcd_draw_rectangle(btn_x, btn2_y, btn_x + btn_w, btn2_y + btn_h, BLACK);
    lcd_show_string(btn_x + 10, btn2_y + 8, btn_w - 20, btn_h - 16, 24, "Amp", BLACK);
    sprintf(value_str, "%u mV", dds_amp[dds_channel]);
    lcd_show_string(btn_x + 100, btn2_y + 35, btn_w - 140, 20, font_size, value_str, RED);

    // Phase 按键
    uint16_t phase_btn_w = 120;
    lcd_fill(btn_x, btn3_y, btn_x + phase_btn_w, btn3_y + btn_h, LGRAY);
    g_back_color = LGRAY;
    lcd_draw_rectangle(btn_x, btn3_y, btn_x + phase_btn_w, btn3_y + btn_h, BLACK);
    lcd_show_string(btn_x + 10, btn3_y + 8, phase_btn_w - 20, btn_h - 16, 24, "Phase", BLACK);
    sprintf(value_str, "%u deg", dds_phase[dds_channel]);
    lcd_show_string(btn_x + 60, btn3_y + 35, phase_btn_w - 70, 20, font_size, value_str, RED);

    // 输出按钮
    uint16_t out_btn_x = btn_x + phase_btn_w + 20;
    uint16_t out_btn_w = 60;
    lcd_fill(out_btn_x, btn3_y, out_btn_x + out_btn_w, btn3_y + btn_h, GREEN);
    g_back_color = GREEN;
    lcd_draw_rectangle(out_btn_x, btn3_y, out_btn_x + out_btn_w, btn3_y + btn_h, BLACK);
    lcd_show_string(out_btn_x + 14, btn3_y + 18, out_btn_w - 16, btn_h - 36, 24, "On", BLACK);

    // 触摸检测
    while (1) {
        tp_dev.scan(0);
        if (tp_dev.sta & TP_PRES_DOWN) {
            LED0(0);
            uint16_t tx = tp_dev.x[0], ty = tp_dev.y[0];
            // 返回
            if (tx > back_x && tx < back_x + back_w && ty > back_y && ty < back_y + back_h) {
                current_page = PAGE_MAIN; // 返回上一个页面
                return;
            }
            // 通道切换
            if (tx > chg_x && tx < chg_x + chg_w && ty > chg_y && ty < chg_y + chg_h) {
                dds_channel = (dds_channel + 1) % 4;
                return; // 更新通道后返回
            }
            // Frep
            if (tx > btn_x && tx < btn_x + btn_w && ty > btn1_y && ty < btn1_y + btn_h) {
                num_type = NUM_FREQ;
                last_page = current_page;
                current_page = PAGE_NUM;
                return;
            }
            // Amplitude
            if (tx > btn_x && tx < btn_x + btn_w && ty > btn2_y && ty < btn2_y + btn_h) {
                num_type = NUM_AMP;
                last_page = current_page;
                current_page = PAGE_NUM;
                return;
            }
            // Phase
            if (tx > btn_x && tx < btn_x + phase_btn_w && ty > btn3_y && ty < btn3_y + btn_h) {
                num_type = NUM_PHASE;
                last_page = current_page;
                current_page = PAGE_NUM;
                return;
            }
            // 输出按钮
            if (tx > out_btn_x && tx < out_btn_x + out_btn_w && ty > btn3_y && ty < btn3_y + btn_h) {
                Write_Freq(dds_channel, dds_freq[dds_channel]);
                Write_Amplitude(dds_channel, dds_amp[dds_channel] * 4.32);
                Write_Phase(dds_channel, dds_phase[dds_channel]);
                IO_Update(); // // 调用AD9959的IO_Update函数
                LED1(0);
                delay_ms(100);
            }
            while (tp_dev.sta & TP_PRES_DOWN) tp_dev.scan(0);
        }
        else {
            LED0(1);
            LED1(1);
        }
        delay_ms(20);
    }
}

void sddgui(void)
{
    lcd_clear(GRAY);
    g_back_color = GRAY;
    lcd_show_string(96, 20, 100, 40, 32, "SDD", DARKBLUE);

    // 返回按钮
    uint16_t back_x = 10, back_y = 10, back_w = 40, back_h = 40;
    lcd_fill(back_x, back_y, back_x + back_w, back_y + back_h, LGRAY);
    g_back_color = LGRAY;
    lcd_draw_rectangle(back_x, back_y, back_x + back_w, back_y + back_h, BLACK);
    lcd_show_string(back_x + 10, back_y + 4, 16, 24, 32, "<", BLACK);

    // 通道切换按钮
    uint16_t chg_x = 160, chg_y = 20, chg_w = 65, chg_h = 40;
    lcd_fill(chg_x, chg_y, chg_x + chg_w, chg_y + chg_h, YELLOW);
    g_back_color = YELLOW;
    lcd_draw_rectangle(chg_x, chg_y, chg_x + chg_w, chg_y + chg_h, BLACK);
    char chstr[16];
    sprintf(chstr, "CH:%d", dds_channel);
    lcd_show_string(chg_x + 10, chg_y + 8, chg_w - 20, chg_h - 16, 24, chstr, BLUE);

    // 按钮参数
    uint16_t btn_w = 200, btn_h = 60;
    uint16_t btn_x = 20;
    uint16_t btn1_y = 70;
    uint16_t btn2_y = 150;
    uint16_t btn3_y = 230;
    uint8_t font_size = 16;

    char value_str[32];

    // Frep 按钮
    lcd_fill(btn_x, btn1_y, btn_x + btn_w, btn1_y + btn_h, LGRAY);
    g_back_color = LGRAY;
    lcd_draw_rectangle(btn_x, btn1_y, btn_x + btn_w, btn1_y + btn_h, BLACK);
    lcd_show_string(btn_x + 10, btn1_y + 8, btn_w - 20, btn_h - 16, 24, "Frep", BLACK);
    sprintf(value_str, "%lu Hz", dds_freq[dds_channel]);
    lcd_show_string(btn_x + 100, btn1_y + 35, btn_w - 110, 20, font_size, value_str, RED);

    // Amplitudea按键
    lcd_fill(btn_x, btn2_y, btn_x + btn_w, btn2_y + btn_h, LGRAY);
    g_back_color = LGRAY;
    lcd_draw_rectangle(btn_x, btn2_y, btn_x + btn_w, btn2_y + btn_h, BLACK);
    lcd_show_string(btn_x + 10, btn2_y + 8, btn_w - 20, btn_h - 16, 24, "Amp", BLACK);
    sprintf(value_str, "%u mV", dds_amp[dds_channel]);
    lcd_show_string(btn_x + 100, btn2_y + 35, btn_w - 140, 20, font_size, value_str, RED);

    // Phase 按键
    uint16_t phase_btn_w = 120;
    lcd_fill(btn_x, btn3_y, btn_x + phase_btn_w, btn3_y + btn_h, LGRAY);
    g_back_color = LGRAY;
    lcd_draw_rectangle(btn_x, btn3_y, btn_x + phase_btn_w, btn3_y + btn_h, BLACK);
    lcd_show_string(btn_x + 10, btn3_y + 8, phase_btn_w - 20, btn_h - 16, 24, "Phase", BLACK);
    sprintf(value_str, "%u deg", dds_phase[dds_channel]);
    lcd_show_string(btn_x + 60, btn3_y + 35, phase_btn_w - 70, 20, font_size, value_str, RED);

    // 输出按钮
    uint16_t out_btn_x = btn_x + phase_btn_w + 20;
    uint16_t out_btn_w = 60;
    lcd_fill(out_btn_x, btn3_y, out_btn_x + out_btn_w, btn3_y + btn_h, GREEN);
    g_back_color = GREEN;
    lcd_draw_rectangle(out_btn_x, btn3_y, out_btn_x + out_btn_w, btn3_y + btn_h, BLACK);
    lcd_show_string(out_btn_x + 14, btn3_y + 18, out_btn_w - 16, btn_h - 36, 24, "On", BLACK);

    // 触摸检测
    while (1) {
        tp_dev.scan(0);
        if (tp_dev.sta & TP_PRES_DOWN) {
            LED0(0);
            uint16_t tx = tp_dev.x[0], ty = tp_dev.y[0];
            // 返回
            if (tx > back_x && tx < back_x + back_w && ty > back_y && ty < back_y + back_h) {
                current_page = PAGE_MAIN;
                return;
            }
            // 通道切换
            if (tx > chg_x && tx < chg_x + chg_w && ty > chg_y && ty < chg_y + chg_h) {
                dds_channel = (dds_channel + 1) % 4;
                return; // 更新通道后返回
            }
            // Frep
            if (tx > btn_x && tx < btn_x + btn_w && ty > btn1_y && ty < btn1_y + btn_h) {
                num_type = NUM_FREQ;
                last_page = current_page;
                current_page = PAGE_NUM;
                return;
            }
            // Amplitude
            if (tx > btn_x && tx < btn_x + btn_w && ty > btn2_y && ty < btn2_y + btn_h) {
                num_type = NUM_AMP;
                last_page = current_page;
                current_page = PAGE_NUM;
                return;
            }
            // Phase
            if (tx > btn_x && tx < btn_x + phase_btn_w && ty > btn3_y && ty < btn3_y + btn_h) {
                num_type = NUM_PHASE;
                last_page = current_page;
                current_page = PAGE_NUM;
                return;
            }
            // 输出按钮
            if (tx > out_btn_x && tx < out_btn_x + out_btn_w && ty > btn3_y && ty < btn3_y + btn_h) {
                Write_Freq(dds_channel, dds_freq[dds_channel]);
                Write_Amplitude(dds_channel, (uint16_t)((dds_amp[dds_channel] * 0.0503 - 26.01) * 4.32));//待定   1k : 
                Write_Phase(dds_channel, dds_phase[dds_channel]);
                IO_Update(); // // 调用AD9959的IO_Update函数
                LED1(0);
                delay_ms(100);
            }
            while (tp_dev.sta & TP_PRES_DOWN) tp_dev.scan(0);
        }
        else {
            LED0(1);
            LED1(1);
        }
        delay_ms(20);
    }
}

void numgui(void)
{
    lcd_clear(GRAY);

    // 输入框参数（缩短输入框，避免和返回按钮重叠）
    uint16_t input_x1 = 60, input_y1 = 10, input_x2 = 230, input_y2 = 65;
    uint8_t input_font_size = 32;
    char input_str[INPUT_MAX_LEN + 1] = {0};
    uint8_t input_len = 0;

    // 按键参数
    uint16_t btn_w = 60, btn_h = 40;
    uint16_t gap_x = 15, gap_y = 15;
    uint16_t start_x = 20, start_y = 90;
    uint8_t font_size = 24;
    char numstr[2] = {0};

    // 绘制输入框
    lcd_draw_rectangle(input_x1, input_y1, input_x2, input_y2, BLACK);
    lcd_fill(input_x1 + 2, input_y1 + 2, input_x2 - 2, input_y2 - 2, WHITE);

    // 绘制1~9数字
    int num = 1;
    for (int row = 0; row < 3; row++) {
        for (int col = 0; col < 3; col++) {
            uint16_t x1 = start_x + col * (btn_w + gap_x);
            uint16_t y1 = start_y + row * (btn_h + gap_y);
            uint16_t x2 = x1 + btn_w;
            uint16_t y2 = y1 + btn_h;
            lcd_fill(x1, y1, x2, y2, LGRAY);
			g_back_color = LGRAY;
            lcd_draw_rectangle(x1, y1, x2, y2, BLACK);
            numstr[0] = '0' + num;
            numstr[1] = 0;
            lcd_show_string(x1 + 22, y1 + 8, btn_w, btn_h, font_size, numstr, BLACK);
            num++;
        }
    }

    // 0
    uint16_t x0 = start_x + (btn_w + gap_x);
    uint16_t y0 = start_y + 3 * (btn_h + gap_y);
    lcd_fill(x0, y0, x0 + btn_w, y0 + btn_h, LGRAY);
	g_back_color = LGRAY;
    lcd_draw_rectangle(x0, y0, x0 + btn_w, y0 + btn_h, BLACK);
    lcd_show_string(x0 + 22, y0 + 8, btn_w, btn_h, font_size, "0", BLACK);

    // del
    uint16_t del_x = start_x;
    uint16_t del_y = y0;
    lcd_fill(del_x, del_y, del_x + btn_w, del_y + btn_h, LIGHTBLUE);
	g_back_color = LIGHTBLUE;
    lcd_draw_rectangle(del_x, del_y, del_x + btn_w, del_y + btn_h, BLACK);
    lcd_show_string(del_x + 10, del_y + 8, btn_w, btn_h, font_size, "DEL", BLACK);

    // ok
    uint16_t ok_x = start_x + 2 * (btn_w + gap_x);
    uint16_t ok_y = y0;
    lcd_fill(ok_x, ok_y, ok_x + btn_w, ok_y + btn_h, GREEN);
	g_back_color = GREEN;
    lcd_draw_rectangle(ok_x, ok_y, ok_x + btn_w, ok_y + btn_h, BLACK);
    lcd_show_string(ok_x + 12, ok_y + 8, btn_w, btn_h, font_size, "OK", BLACK);

    // 返回
    uint16_t back_x = 10, back_y = 10, back_w = 40, back_h = 40;
    lcd_fill(back_x, back_y, back_x + back_w, back_y + back_h, LGRAY);
    g_back_color = LGRAY;
    lcd_draw_rectangle(back_x, back_y, back_x + back_w, back_y + back_h, BLACK);
    lcd_show_string(back_x + 10, back_y + 4, 16, 24, 32, "<", BLACK);

    while (1) {
        tp_dev.scan(0);
        if (tp_dev.sta & TP_PRES_DOWN) {
			LED0(0);
			// 获取触摸坐标
            uint16_t tx = tp_dev.x[0];
            uint16_t ty = tp_dev.y[0];

            // <返回
            if (tx > back_x && tx < back_x + back_w && ty > back_y && ty < back_y + back_h) {
                current_page = last_page;
                return;
            }
            // 1~9数字
            for (int row = 0; row < 3; row++) {
                for (int col = 0; col < 3; col++) {
                    uint16_t x1 = start_x + col * (btn_w + gap_x);
                    uint16_t y1 = start_y + row * (btn_h + gap_y);
                    uint16_t x2 = x1 + btn_w;
                    uint16_t y2 = y1 + btn_h;
                    if (tx > x1 && tx < x2 && ty > y1 && ty < y2) {
                        if (input_len < INPUT_MAX_LEN) {
                            input_str[input_len++] = '1' + row * 3 + col;
                            input_str[input_len] = 0;
                        }
                    }
                }
            }
            // 0
            if (tx > x0 && tx < x0 + btn_w && ty > y0 && ty < y0 + btn_h) {
                if (input_len < INPUT_MAX_LEN) {
                    input_str[input_len++] = '0';
                    input_str[input_len] = 0;
                }
            }
            // del
            if (tx > del_x && tx < del_x + btn_w && ty > del_y && ty < del_y + btn_h) {
                if (input_len > 0) {
                    input_str[--input_len] = 0;
                }
            }
            // ok
            if (tx > ok_x && tx < ok_x + btn_w && ty > ok_y && ty < ok_y + btn_h) {
                if (input_len > 0) {
                    uint32_t value = atoi(input_str);
                    if (num_type == NUM_FREQ) {
                        dds_freq[dds_channel] = value;
                    } else if (num_type == NUM_AMP) {
                        dds_amp[dds_channel] = value;
                    } else if (num_type == NUM_PHASE) {
                        dds_phase[dds_channel] = value;
                    }   
                }
                LED1(0);
                current_page = last_page;
                return;
            }

            // 更新输入框内容
            lcd_fill(input_x1 + 2, input_y1 + 2, input_x2 - 2, input_y2 - 2, WHITE);
			g_back_color = WHITE;
            lcd_show_string(
                input_x1 + 6,
                input_y1 + 10,
                input_x2 - input_x1 - 20,
                input_y2 - input_y1 - 20,
                input_font_size,
                input_str,
                RED
            );

            while (tp_dev.sta & TP_PRES_DOWN) tp_dev.scan(0);
        }
		else {
			LED0(1);
            LED1(1);
		}
        delay_ms(50); // 减少CPU占用
    }
}

//—————————————————————————————— ————————— ———————————————————————————————————
//—————————————————————————————— ————————— ———————————————————————————————————
//——————————————————————————————  发挥部分  ———————————————————————————————————
//—————————————————————————————— ————————— ———————————————————————————————————
//—————————————————————————————— ————————— ———————————————————————————————————
void learngui(void)
{
    lcd_clear(GRAY);
    g_back_color = GRAY;
    lcd_show_string(80, 20, 100, 40, 32, "Learn", BLUE);

    // 显示返回按钮
    uint16_t back_x = 10, back_y = 10, back_w = 40, back_h = 40;
    lcd_fill(back_x, back_y, back_x + back_w, back_y + back_h, LGRAY);
    g_back_color = LGRAY;
    lcd_draw_rectangle(back_x, back_y, back_x + back_w, back_y + back_h, BLACK);
    lcd_show_string(back_x + 10, back_y + 4, 16, 24, 32, "<", BLACK);
    
    // 进度条尺寸
    uint16_t progress_x = 20;
    uint16_t progress_y = 160;
    uint16_t progress_width = 200;
    uint16_t progress_height = 20;

    // 扫频
    int idx = 0;
    int total_points = SWEEP_POINTS;
    for (uint32_t f = SWEEP_START_FREQ; f <= SWEEP_END_FREQ; f += SWEEP_STEP, idx++) {
        sweep_freqs[idx] = f;
        Write_Freq(0, f); // DDS设置频率
        Write_Amplitude(0, 200 * 4.32); // 设置幅度为200mV
        Write_Phase(0, 0);
        IO_Update();
        delay_ms(200); // 等待DDS设置稳定

        // 读取ADC数据
        uint32_t adc_max = 0, adc_min = 0xFFFF;
        uint32_t sample_count = 1000; // 采样点数

        for (uint32_t i = 0; i < sample_count; i++) {
            uint32_t val = adc_get_result(ADC_CHANNEL_1) * 3300 / 4096;
            if (val > adc_max) adc_max = val;
        }
        sweep_amps[idx] = adc_max;

        // 绘制进度条
        lcd_draw_rectangle(progress_x, progress_y, progress_x + progress_width, progress_y + progress_height, BLACK);
        
        // 计算填充宽度
        uint16_t progress_width_filled = (idx * progress_width) / total_points;
        
        // 填充进度条
        if (progress_width_filled > 0) {
            lcd_fill(progress_x + 1, progress_y + 1, 
                progress_x + progress_width_filled, progress_y + progress_height - 1, GREEN);
        }
            // 填充剩余部分为白色
        if (progress_width_filled < progress_width - 1) {
            lcd_fill(progress_x + progress_width_filled + 1, progress_y + 1, 
                progress_x + progress_width - 1, progress_y + progress_height - 1, WHITE);
        }
        
        // 显示进度%
        uint8_t percentage = (idx * 100) / total_points;
        char percent_str[30];
        sprintf(percent_str, "Learning %d%%", percentage);
        g_back_color = GRAY;
        lcd_show_string(50, 120, 240, 20, 24, percent_str, BLACK);
        
        // 显示进度条
        char buf[32];
        sprintf(buf, "Freq: %lu Hz  ", f);
        lcd_show_string(60, 200, 200, 24, 16, buf, BLACK);  
        sprintf(buf, "Amp: %lu mV  ", sweep_amps[idx]);
        lcd_show_string(60, 220, 200, 24, 16, buf, BLACK);         
        
        // 检查返回按钮（随时返回）
        tp_dev.scan(0);
        if (tp_dev.sta & TP_PRES_DOWN) {
            LED0(0);
            uint16_t tx = tp_dev.x[0], ty = tp_dev.y[0];
            if (tx > back_x && tx < back_x + back_w && ty > back_y && ty < back_y + back_h) {
                current_page = last_page;

                // 关闭DDS输出
                Write_Freq(0, 0);
                Write_Amplitude(0, 0);
                Write_Phase(0, 0);
                IO_Update();
                return;
            }
            while (tp_dev.sta & TP_PRES_DOWN) tp_dev.scan(0);
        }
        else {
            LED0(1);
        }
    }
            
    // 结束时填充进度条
    lcd_fill(progress_x + 1, progress_y + 1, 
    progress_x + progress_width - 1, progress_y + progress_height - 1, GREEN);
    lcd_show_string(50, 120, 240, 20, 24, "Learning 100%", BLACK);

    // 找到最大幅度及其对应的频率
    uint16_t max_amp = 0;
    uint32_t max_amp_freq = 0;
    for (int i = 0; i < SWEEP_POINTS; i++) {
        if (sweep_amps[i] > max_amp) {
            max_amp = sweep_amps[i];
            max_amp_freq = sweep_freqs[i];
        }
    }
    // 找到最小幅度及其对应的频率
    uint16_t min_amp = 0xFFFF;
    uint32_t min_amp_freq = 0;
    for (int i = 0; i < SWEEP_POINTS; i++) {
        if (sweep_amps[i] < min_amp) {
            min_amp = sweep_amps[i];
            min_amp_freq = sweep_freqs[i];
        }
    }

    // 计算平均幅度
    uint32_t total_amp = 0;
    for (int i = 0; i < SWEEP_POINTS; i++) {
        total_amp += sweep_amps[i];
    }
    uint16_t avg_amp = total_amp / SWEEP_POINTS;

    // 根据幅度特性判断滤波器类型
    char filter_type[32] = {0};

    // 如果最大幅度在低频段且平均幅度较低，判断为低通滤波器
    if (max_amp_freq <= (SWEEP_START_FREQ + SWEEP_END_FREQ) / 3 && 
        max_amp > avg_amp * 1.5) {
        strcpy(filter_type, "low-pass");
    }
    // 如果最大幅度在高频段且平均幅度较低，判断为高通滤波器
    else if (max_amp_freq >= (SWEEP_START_FREQ + SWEEP_END_FREQ) * 2 / 3 && 
            max_amp > avg_amp * 1.5) {
        strcpy(filter_type, "high-pass");
    }
    // 如果最大幅度在中间频段且平均幅度较低，判断为带通滤波器
    else if (max_amp_freq > (SWEEP_START_FREQ + SWEEP_END_FREQ) / 3 && 
            max_amp_freq < (SWEEP_START_FREQ + SWEEP_END_FREQ) * 2 / 3 && 
            max_amp > avg_amp * 1.5) {
        strcpy(filter_type, "band-pass");
    }
    // 带阻滤波器：最小幅度在中间频段，且最小幅度显著低于平均幅度
    else if (min_amp_freq > (SWEEP_START_FREQ + SWEEP_END_FREQ) / 3 && 
         min_amp_freq < (SWEEP_START_FREQ + SWEEP_END_FREQ) * 2 / 3 && 
         min_amp < avg_amp / 1.5) {  // 最小幅度远低于平均
    strcpy(filter_type, "band-stop");
    }
    // 其他情况判断为未知类型
    else {
        strcpy(filter_type, "unknown");
    }   
    lcd_show_string(70, 250, 240, 20, 24, filter_type, RED);
    
    while (1) {
        tp_dev.scan(0);
        if (tp_dev.sta & TP_PRES_DOWN) {
            LED0(0);
            uint16_t tx = tp_dev.x[0], ty = tp_dev.y[0];
            if (tx > back_x && tx < back_x + back_w && ty > back_y && ty < back_y + back_h) {
                current_page = last_page;

                // 关闭DDS输出
                Write_Freq(1, 0);
                Write_Amplitude(1, 0);
                Write_Phase(1, 0);
                IO_Update();
                return;
            }
            while (tp_dev.sta & TP_PRES_DOWN) tp_dev.scan(0);
        }
		else {
			LED0(1);
		}
        delay_ms(20);
    }
}

extern volatile uint32_t period;
extern volatile uint8_t  capture_flag;

void exploregui(void)
{
    lcd_clear(GRAY);
    g_back_color = GRAY;
    lcd_show_string(80, 20, 100, 40, 32, "Explore", BLUE);

    // 返回按钮
    uint16_t back_x = 10, back_y = 10, back_w = 40, back_h = 40;
    lcd_fill(back_x, back_y, back_x + back_w, back_y + back_h, LGRAY);
    g_back_color = LGRAY;
    lcd_draw_rectangle(back_x, back_y, back_x + back_w, back_y + back_h, BLACK);
    lcd_show_string(back_x + 10, back_y + 4, 16, 24, 32, "<", BLACK);

    g_back_color = GRAY;
    lcd_show_string(15, 120, 200, 24, 24, "Detecting Freq...", BLACK);
    
    // 显示当前状态标签
    lcd_show_string(30, 180, 200, 24, 16, "Input Freq:   Hz", BLACK);
    lcd_show_string(30, 200, 200, 24, 16, "Output Amp:   mV", BLACK);

    HAL_TIM_Base_Start(&htim2);
    __HAL_TIM_SET_COUNTER(&htim2, 0);  // 清零定时器计数器
    HAL_TIM_IC_Stop_IT(&htim2, TIM_CHANNEL_1);
    HAL_TIM_IC_Start_IT(&htim2, TIM_CHANNEL_1);
    
    // 持续检测和调整
    while (1) {
        // 检查返回按钮
        tp_dev.scan(0);
        if (tp_dev.sta & TP_PRES_DOWN) {
            LED0(0);
            uint16_t tx = tp_dev.x[0], ty = tp_dev.y[0];
            if (tx > back_x && tx < back_x + back_w && ty > back_y && ty < back_y + back_h) {
                current_page = last_page;

                // 关闭DDS输出
                Write_Freq(0, 0);
                Write_Amplitude(0, 0);
                Write_Phase(0, 0);
                IO_Update();

                // 停止定时器
                HAL_TIM_IC_Stop_IT(&htim2, TIM_CHANNEL_1);                
                __HAL_TIM_SET_COUNTER(&htim2, 0);  // 清零定时器计数器
                HAL_TIM_Base_Stop(&htim2);
                return;
            }
            while (tp_dev.sta & TP_PRES_DOWN) tp_dev.scan(0);
        }
        else {
            LED0(1);
        }
        
        // 检测频率输入并调整输出
        static uint32_t last_capture_time = 0;
        static uint32_t last_freq = 0;
        
        if (capture_flag) {
            // 重置标志
            capture_flag = 0;
            
            // 获取输入频率
            uint32_t timer_clk = 1000000;
            uint32_t freq = timer_clk / period;
            
            // 只有当频率发生变化时才更新输出
            if (freq != last_freq || (HAL_GetTick() - last_capture_time > 500)) {
                last_freq = freq;
                last_capture_time = HAL_GetTick();
                
                // 在sweep_freqs中找到最接近的频率点
                int best_idx = 0;
                uint32_t min_diff = 0xFFFFFFFF;
                for (int i = 0; i < SWEEP_POINTS; i++) {
                    uint32_t diff = (freq > sweep_freqs[i]) ? (freq - sweep_freqs[i]) : (sweep_freqs[i] - freq);
                    if (diff < min_diff) {
                        min_diff = diff;
                        best_idx = i;
                    }
                }

                // 用DDS输出该频率对应的幅值
                uint32_t out_freq = sweep_freqs[best_idx];
                uint16_t out_amp_shown = sweep_amps[best_idx];
                float out_amp =  out_amp_shown / 10;
                // 限制输出幅度最大为235mV
                if (out_amp > 235) {
                    out_amp = 235;
                }
                
                Write_Freq(0, out_freq);
                Write_Amplitude(0, (uint16_t)(out_amp * 4.32));
                Write_Phase(0, 0);
                IO_Update();

                // 显示结果
                char buf[64];
                sprintf(buf, "Input Freq: %lu Hz    ", freq);
                lcd_show_string(30, 180, 200, 24, 16, buf, BLUE);
                sprintf(buf, "Output Frep: %u Hz    ", out_freq);
                lcd_show_string(30, 200, 200, 24, 16, buf, RED);
                sprintf(buf, "Output Amp: %u mV    ", out_amp_shown);
                lcd_show_string(30, 220, 200, 24, 16, buf, RED);
            }
        }
        
        delay_ms(20);
    }
}

void infogui()
{
	lcd_clear(GRAY);
    g_back_color = GRAY;

	//<
	uint16_t back_x = 10, back_y = 10, back_w = 40, back_h = 40;
	lcd_fill(back_x, back_y, back_x + back_w, back_y + back_h, LGRAY);
	g_back_color = LGRAY;
	lcd_draw_rectangle(back_x, back_y, back_x + back_w, back_y + back_h, BLACK);
	lcd_show_string(back_x + 10, back_y + 4, 16, 24, 32, "<", BLACK);
    
	// Info
	g_back_color = GRAY;
	lcd_show_string(88, 80, 280, 100, 32, "Info", BLUE);
	
	lcd_show_string(30, 160, 280, 100, 16, "Unkown Circuitry Device", BLACK);
	lcd_show_string(52, 220, 280, 100, 16, "Made for TI 2025", BLACK);
	lcd_show_string(32, 300, 280, 100, 16, "General Availability I", BLACK);

	while (1) {
		tp_dev.scan(0);
		if (tp_dev.sta & TP_PRES_DOWN) {
			LED0(0);
			uint16_t tx = tp_dev.x[0], ty = tp_dev.y[0];
			if (tx > back_x && tx < back_x + back_w && ty > back_y && ty < back_y + back_h) {
				current_page = last_page;
				return;
			}
			while (tp_dev.sta & TP_PRES_DOWN) tp_dev.scan(0);
		}
		else {
			LED0(1);
		}	
		delay_ms(20);
    }
}

void authorgui()
{
	lcd_clear(GRAY);
    g_back_color = GRAY;

	//<
	uint16_t back_x = 10, back_y = 10, back_w = 40, back_h = 40;
	lcd_fill(back_x, back_y, back_x + back_w, back_y + back_h, LGRAY);
	g_back_color = LGRAY;
	lcd_draw_rectangle(back_x, back_y, back_x + back_w, back_y + back_h, BLACK);
	lcd_show_string(back_x + 10, back_y + 4, 16, 24, 32, "<", BLACK);
    
	// Info
	g_back_color = GRAY;
	lcd_show_string(88, 20, 280, 100, 32, "Author", BLUE);
	lcd_show_string(72, 80, 280, 100, 24, "Klors Xu", BLACK);

	lcd_show_string(60, 130, 280, 100, 24, "Teammates: ", RED);
	lcd_show_string(84, 190, 280, 100, 16, "LiYuanjiang", BLACK);
	lcd_show_string(80, 210, 280, 100, 16, "XiaoJiashu", BLACK);

	lcd_show_string(78, 293, 280, 100, 12, "Working within", BROWN);
	lcd_show_string(78, 307, 280, 100, 12, "2025/7/6 ~ 8/3", BROWN);

	while (1) {
		tp_dev.scan(0);
		if (tp_dev.sta & TP_PRES_DOWN) {
			LED0(0);
			uint16_t tx = tp_dev.x[0], ty = tp_dev.y[0];
			if (tx > back_x && tx < back_x + back_w && ty > back_y && ty < back_y + back_h) {
				current_page = last_page;
				return;
			}
			while (tp_dev.sta & TP_PRES_DOWN) tp_dev.scan(0);
		}
		else {
			LED0(1);
		}	
		delay_ms(20);
    }
}