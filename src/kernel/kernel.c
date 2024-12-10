#include <stddef.h>
#include <stdint.h>
#include <kernel/uart.h>
#include <kernel/mem.h>
#include <kernel/atag.h>
#include <common/stdio.h>
#include <common/stdlib.h>

int custom_strcmp(const char *str1, const char *str2) {
    while (*str1 && *str2) {
        if (*str1 != *str2) {
            return *str1 - *str2; // Return difference if characters don't match
        }
        str1++;
        str2++;
    }
    // If one string ends, check if there's a difference in length
    return *str1 - *str2;
}

int custom_atoi(const char *str) {
    int result = 0;  // Final integer result
    int sign = 1;    // To handle negative numbers

    // Check for a negative sign at the beginning
    if (*str == '-') {
        sign = -1;  // Negative number
        str++;      // Skip the negative sign
    }

    // Iterate through the string and convert each digit
    while (*str >= '0' && *str <= '9') { // Check if the character is a digit
        result = result * 10 + (*str - '0'); // Accumulate the integer value
        str++; // Move to the next character
    }

    return result * sign; // Apply the sign to the result
}

void print_int(int num) {
	if (num < 0){
		putc('-');
		num = -num;
	}
	
	if (num / 10 ) {
		print_int(num/10);
	}
	
	putc((num%10) + '0');
}

void printf(const char *format, ...) {
    char *arg_list = (char *)&format + sizeof(format); // Pointer to the arguments
    char *str_arg;     // Pointer for string arguments
    int int_arg;       // Integer argument

    // Iterate through the format string
    while (*format) {
        if (*format == '%' && *(format + 1)) { // Check for format specifier
            format++; // Move to the specifier character
            if (*format == 's') { // Handle %s (string)
                str_arg = *(char **)arg_list; // Retrieve string argument
                arg_list += sizeof(char *);  // Advance to the next argument
                puts(str_arg);               // Print the string
            } else if (*format == 'd') { // Handle %d (integer)
                int_arg = *(int *)arg_list; // Retrieve integer argument
                arg_list += sizeof(int);   // Advance to the next argument
                print_int(int_arg);        // Print the integer (using print_int from before)
            } else { // Unsupported specifier, just print as-is
                putc('%');
                putc(*format);
            }
        } else { // Normal character, print as-is
            putc(*format);
        }
        format++; // Move to the next character
    }
}


void kernel_main(uint32_t r0, uint32_t r1, uint32_t atags)
{
    char buf[256];
    char command[32];
    char args[224];
    
    // Declare as unused
    (void) r0;
    (void) r1;
    (void) atags;

    // Initialize UART and memory
    uart_init();
    puts("Initializing Memory Module\n");
    mem_init((atag_t *)atags);

    puts("Hello, kernel World!\n");

    while (1) {
		puts("> ");
		gets(buf, 256); // Read user input into buffer

		// Clear the command and args buffers
		for (int i = 0; i < 32; i++) {
		    command[i] = '\0';
		}
		for (int i = 0; i < 224; i++) {
		    args[i] = '\0';
		}

		// Parse the command and arguments manually
		int i = 0, j = 0;
		// Extract the command (up to the first space or end of string)
		while (buf[i] != ' ' && buf[i] != '\0') {
		    command[i] = buf[i];
		    i++;
		}
		command[i] = '\0'; // Null-terminate the command

		// Skip the space and copy the rest into args
		if (buf[i] == ' ') {
		    i++; // Move past the space
		}
		while (buf[i] != '\0') {
		    args[j++] = buf[i++];
		}
		args[j] = '\0'; // Null-terminate the arguments

		// Compare the command using custom_strcmp
		if (custom_strcmp(command, "help") == 0) {
		    printf("Available commands:\n");
		    printf("help - Show help message\n");
		    printf("echo [message] - Print a message\n");
		    printf("square [number] - Print the square of a number\n");
		    printf("exit - Exit the CLI\n");
		} else if (custom_strcmp(command, "echo") == 0) {
		    printf("%s\n", args); // Print the argument
		} else if (custom_strcmp(command, "square") == 0) {
		    int number = custom_atoi(args); // Convert the argument to an integer
		    int square = number * number;
		    printf("The square is: %d\n", square); // Print the result
		} else if (custom_strcmp(command, "exit") == 0) {
		    printf("Exiting CLI...\n");
		    break; // Exit the loop
		} else {
		    printf("Unknown command. Type 'help' for available commands.\n");
		}

		// Clear the input buffer
		for (int i = 0; i < 256; i++) {
		    buf[i] = '\0';
		}

		putc('\n'); // Print a newline after processing the command
	}

}
