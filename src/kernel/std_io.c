#include "std_io.h"
#include "os_type.h"
#include "asm_utils.h"
#include "os_modules.h"
#include "std_arg.h"
#include "std_lib.h"
#include "syscall.h"

void io_init(Std_IO *io) {
    io->screen = (uint8 *)0xc00b8000; 
}

void io_clear(Std_IO *io) {
    move_cursor(0);
    for (int i = 0; i < 25 * 80; ++i) {
        print_char(io, i / 80, i % 80, ' ', 0x00);
    }
    move_cursor(0);
}

void print_char(Std_IO *io, uint x, uint y, uint8 c, uint8 color) {
    if (x >= 25 || y >= 80) {
        return;
    }

    uint pos = x * 80 + y;
    io->screen[2 * pos] = c;
    io->screen[2 * pos + 1] = color;
}

int print_str(Std_IO *io, const char *const str) {
    int i = 0;

    for (i = 0; str[i]; ++i) {
        switch (str[i]) {
            case '\n': {
                uint row;
                row = get_cursor(io) / 80;
                if (row == 24)
                    roll_up(io);
                else
                    ++row;
                move_cursor(row * 80);
                break;
            }   

            default:
                print_to_cursor(io, str[i], 0x07);
                break;
        }
    }
    return i;
}

int printf_buffer(char *buffer, char c, int *idx, const int BUF_LEN) {
    int counter = 0;

    buffer[*idx] = c;
    ++(*idx);

    if (*idx == BUF_LEN)
    {
        buffer[*idx] = '\0';
        counter = write(buffer);
        *idx = 0;
    }

    return counter;
}

int printf(const char* const fmt, ...) {
    const int BUF_LEN = 32;

    char buffer[BUF_LEN + 1];
    char number[33];

    int idx, counter;
    va_list ap;

    va_start(ap, fmt);
    idx = 0;
    counter = 0;

    for (int i = 0; fmt[i]; ++i) {
        if (fmt[i] != '%') {
            counter += printf_buffer(buffer, fmt[i], &idx, BUF_LEN);
        }
        else {
            i++;
            if (fmt[i] == '\0') {
                break;
            }

            switch (fmt[i]) {
                case '%':
                    counter += printf_buffer(buffer, fmt[i], &idx, BUF_LEN);
                    break;

                case 'c':
                    counter += printf_buffer(buffer, va_arg(ap, int), &idx, BUF_LEN);
                    break;

                case 's':
                    buffer[idx] = '\0';
                    idx = 0;
                    counter += write(buffer);
                    counter += write(va_arg(ap, const char *));
                    break;

                case 'd':
                case 'x': {
                    int temp = va_arg(ap, int);

                    if (temp < 0 && fmt[i] == 'd') {
                        counter += printf_buffer(buffer, '-', &idx, BUF_LEN);
                        temp = -temp;
                    }

                    itos(number, temp, (fmt[i] == 'd' ? 10 : 16));

                    for (int j = 0; number[j]; ++j) {
                        counter += printf_buffer(buffer, number[j], &idx, BUF_LEN);
                    }
                    break;
                }
            }
        }
    }

    buffer[idx] = '\0';
    counter += write(buffer);

    return counter;
}

void print_to_cursor(Std_IO *io, uint8 c, uint8 color) {
    uint cursor = get_cursor(io);
    io->screen[2 * cursor] = c;
    io->screen[2 * cursor + 1] = color;
    ++cursor;

    if (cursor == 25 * 80) {
        roll_up(io);
        cursor = 24 * 80;
    }

    move_cursor(cursor);
}

void move_cursor(uint position) {
    if (position >= 80 * 25) {
        return;
    }

    uint8 temp;

    temp = (position >> 8) & 0xff;
    asm_out_port(0x3d4, 0x0e);
    asm_out_port(0x3d5, temp);

    temp = position & 0xff;
    asm_out_port(0x3d4, 0x0f);
    asm_out_port(0x3d5, temp);
}

uint get_cursor() {
    uint pos;
    uint8 temp;

    pos = 0;
    temp = 0;

    asm_out_port(0x3d4, 0x0e);
    asm_in_port(0x3d5, &temp);
    pos = ((uint)temp) << 8;

    asm_out_port(0x3d4, 0x0f);
    asm_in_port(0x3d5, &temp);
    pos = pos | ((uint)temp);

    return pos;
}

void move_cursor_xy(uint x, uint y) {
    if (x >= 25 || y >= 80) {
        return;
    }

    move_cursor(x * 80 + y);
}

void roll_up(Std_IO *io) {
    uint length;
    length = 25 * 80;

    for (uint i = 80; i < length; ++i) {
        io->screen[2 * (i - 80)] = io->screen[2 * i];
        io->screen[2 * (i - 80) + 1] = io->screen[2 * i + 1];
    }

    for (uint i = 24 * 80; i < length; ++i) {
        io->screen[2 * i] = ' ';
        io->screen[2 * i + 1] = 0x07;
    }
}
