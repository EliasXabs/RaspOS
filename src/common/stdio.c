#include <kernel/uart.h>
#include <common/stdio.h>
#include <common/stdlib.h>

char getc(void) {
    return uart_getc();
}

void putc(char c) {
    uart_putc(c);
}

void puts(const char * str) {
    int i;
    for (i = 0; str[i] != '\0'; i ++)
        putc(str[i]);
}

void gets(char *buf, int buflen) {
    int i = 0;
    char c;

    // Process characters in real time
    while (1) {
        c = getc();

        if (c == '\r' || c == '\n') { // enter
            putc('\n');
            buf[i] = '\0';
            return;                  
        } else if (c == '\b' || c == 127) { // backspace
            if (i > 0) {
                i--;
                puts("\b \b");
            }
        } else if (i < buflen - 1) { // chars
            buf[i++] = c;
            putc(c);
        }
    }
}	
