#ifndef STD_IO_H
#define STD_IO_H

#include "os_type.h"
#include "std_arg.h"

typedef struct {
    uint8 *screen;
} Std_IO;

void io_init(Std_IO *io);

void io_clear(Std_IO *io);

/**
 * @brief 打印字符c，颜色color到位置(x,y)
 * @param x 行坐标
 * @param y 列坐标
 * @param c 字符内容
 * @param color 字符颜色
 */
void print_char(Std_IO *io, uint x, uint y, uint8 c, uint8 color);

/**
 * @brief 
 * @param str 
 * @return int 
 */
int print_str(Std_IO *io, const char *const str);

/**
 * @brief 
 * 
 * @param buffer 字符缓冲
 * @param c 字符
 * @param idx 
 * @param BUF_LEN 缓冲区长度
 * @return int 
 */
int printf_buffer(char *buffer, char c, int *idx, const int BUF_LEN);

/**
 * @brief 
 * @param fmt 
 * @param ... 
 * @return int 
 */
int printf(const char* const fmt, ...);

/**
 * @brief 打印字符c，颜色color到光标位置
 * @param c 字符内容
 * @param color 字符颜色
 */
void print_to_cursor(Std_IO *io, uint8 c, uint8 color);

/// \brief 移动光标到一维位置
/// \param position 光标的目标位置
void move_cursor(uint position);

/// \brief 移动光标到二维位置
/// \param x 光标的x位置
/// \param y 光标的y位置
void move_cursor_xy(uint x, uint y);

// 获取光标位置
uint get_cursor();

// 滚屏
void roll_up(Std_IO *io);

#endif