#include "interrupt.h"
#include "os_type.h"
#include "os_modules.h"
#include "os_constant.h"
#include "asm_utils.h"
#include "std_io.h"
#include "program.h"

extern Std_IO stdio;

int times = 0;
int rightward = 1;
int downward = 1;

void im_init(InterruptManeger *im) {
    times = 0;

    im->IDT = (uint32 *)IDT_START_ADDRESS;
    asm_lidt(IDT_START_ADDRESS, 256 * 8 - 1);

    for (uint i = 0; i < 256; ++i) {
        set_interrupt_descriptor(im, i, (uint32)asm_unhandled_interrupt, 0);     
    }

    init_8259A(im);
}

void set_interrupt_descriptor(InterruptManeger *im, uint32 index, uint32 address, byte DPL) {
    im->IDT[index * 2] = (CODE_SELECTOR << 16) | (address & 0xffff);
    im->IDT[index * 2 + 1] = (address & 0xffff0000) | (0x1 << 15) | (DPL << 13) | (0xe << 8);
}

void init_8259A(InterruptManeger *im) {
    // ICW 1
    asm_out_port(0x20, 0x11);
    asm_out_port(0xa0, 0x11);
    // ICW 2
    im->IRQ0_8259A_MASTER = 0x20;
    im->IRQ0_8259A_SLAVE = 0x28;
    asm_out_port(0x21, im->IRQ0_8259A_MASTER);
    asm_out_port(0xa1, im->IRQ0_8259A_SLAVE);
    // ICW 3
    asm_out_port(0x21, 4);
    asm_out_port(0xa1, 2);
    // ICW 4
    asm_out_port(0x21, 1);
    asm_out_port(0xa1, 1);

    // OCW 1 屏蔽主片所有中断，但主片的IRQ2需要开启
    asm_out_port(0x21, 0xfb);
    // OCW 1 屏蔽从片所有中断
    asm_out_port(0xa1, 0xff);
}

void enable_time_interrupt() {
    uint8 value;
    // 读入主片OCW
    asm_in_port(0x21, &value);
    // 开启主片时钟中断，置0开启
    value = value & 0xfe;
    asm_out_port(0x21, value);
}

void disable_time_interrupt() {
    uint8 value;
    asm_in_port(0x21, &value);
    // 关闭时钟中断，置1关闭
    value = value | 0x01;
    asm_out_port(0x21, value);
}

void set_time_interrupt(InterruptManeger *im, void *handler) {
    set_interrupt_descriptor(im, im->IRQ0_8259A_MASTER, (uint32)handler, 0);
}

bool get_interrupt_status(InterruptManeger *im) {
    return asm_interrupt_status() ? true : false;
}

void enable_interrupt() {
    asm_enable_interrupt();
}

void disable_interrupt() {
    asm_disable_interrupt();
}

void set_interrupt_status(InterruptManeger *im, bool status) {
    if (status) {
        enable_interrupt();
    } else {
        disable_interrupt();
    }
}

// 中断处理函数
extern void c_time_interrupt_handler() {
    PCB *cur = program_maneger.running;
    if (cur->ticks) {
        cur->ticks -= 1;
        cur->ticks_passed_by += 1;
    } else {
        schedule(&program_maneger);
    }
    cur = program_maneger.running;
    
}