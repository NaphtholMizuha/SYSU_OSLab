#ifndef INTERRUPT_H
#define INTERRUPT_H

#include "os_type.h"
#include "os_constant.h"

typedef struct {
    uint32 *IDT;              // IDT起始地址
    uint32 IRQ0_8259A_MASTER; // 主片中断起始向量号
    uint32 IRQ0_8259A_SLAVE;  // 从片中断起始向量号
} InterruptManeger;

void im_init(InterruptManeger *im);

/// \brief 设置中断描述符
/// \param index   第index个描述符，index=0, 1, ..., 255
/// \param address 中断处理程序的起始地址
/// \param DPL     中断描述符的特权级
void set_interrupt_descriptor(InterruptManeger *im, uint32 index, uint32 address, byte DPL);
    
/// 开启时钟中断
void enable_time_interrupt();
/// 禁止时钟中断
void disable_time_interrupt();

void enable_interrupt();

void disable_interrupt();

bool get_interrupt_status(InterruptManeger *im);

void set_interrupt_status(InterruptManeger *im, bool status);

/// \brief 设置时钟中断处理函数
/// \param handler 句柄, 即处理函数
void set_time_interrupt(InterruptManeger *im, void *handler);

/// 初始化8259A芯片
void init_8259A(InterruptManeger *im);

#endif