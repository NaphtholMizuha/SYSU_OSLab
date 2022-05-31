#include "os_type.h"

void swap_char(char *m, char *n);

/**
 * @brief 将非负int类型转换为指定进制数表示的字符串
 * 
 * @param numStr 转换后的字符串
 * @param num 转换前的数
 * @param mod 进制
 */
void itos(char *numStr, uint32 num, uint32 mod);

void memset(void *memory, char value, int length);

int ceil(const int dividend, const int divisor);

void memcpy(void *src, void *dst, uint32 length);

void strcpy(char *src, char *dst);