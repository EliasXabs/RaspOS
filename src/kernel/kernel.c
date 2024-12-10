#include <stddef.h>
#include <stdint.h>
#include <kernel/uart.h>
#include <kernel/mem.h>
#include <kernel/atag.h>
#include <common/stdio.h>
#include <common/stdlib.h>

typedef struct Node {
    int data;
    struct Node *next;
} Node;

Node *create_node(int data) {
    Node *new_node = (Node *)kmalloc(sizeof(Node)); // Use kmalloc for memory allocation
    if (new_node == NULL) {
        puts("Failed to allocate memory for new node\n");
        return NULL; // Handle memory allocation failure
    }
    new_node->data = data;  // Store the integer data
    new_node->next = NULL;  // Initialize the next pointer to NULL
    return new_node;
}

Node *add_node(Node *head, int data) {
    Node *new_node = create_node(data); // Create a new node
    if (new_node == NULL) {
        return head; // If memory allocation fails, return the unchanged head
    }

    if (head == NULL) { // If the list is empty, the new node becomes the head
        return new_node;
    } else {
        Node *current = head;
        while (current->next != NULL) { // Traverse to the last node
            current = current->next;
        }
        current->next = new_node;
    }
    return head; // Return the updated head of the list
}

void display_list(Node *head) {
    if (head == NULL) {
        puts("The list is empty.\n");
        return;
    }

    Node *current = head;
    printf("LinkedList: ");
    while (current != NULL) {
        print_int(current->data); // Print the integer data
        putc(' ');               // Space between numbers
        current = current->next;
    }
    putc('\n'); // Newline after printing the list
}

void clear_list(Node **head) {
    printf("clearing\n");
    if (*head == NULL) {
        printf("The list is already empty\n");
        return;
    }

    Node *current = *head;
    Node *next;

    printf("looping\n");
    while (current != NULL) {
        printf("current node: %d, data: %d\n", (int)current, current->data);
        next = current->next; // Store the next node
        printf("next node: %d\n", (int)next);

        kfree(current); // Free the current node
        current = next; // Move to the next node

        // Add explicit termination check
        if (current == NULL) {
            printf("Current is NULL; exiting loop.\n");
            break;
        } else {
            printf("current updated to: %d\n", (int)current);
        }
    }

    printf("loop done\n");
    *head = NULL; // Set the head pointer to NULL
    printf("head set to NULL\n");
}


void test_linkedlist() {
    Node *head = NULL; // Initialize the LinkedList

    // Add nodes with integer data
    head = add_node(head, 10);
    head = add_node(head, 20);
    head = add_node(head, 30);

    // Display the list
    display_list(head);

    // Clear the list
    printf("Clearing the list...\n");
	printf("Before clearing: %p\n", head);
	clear_list(&head);
	printf("After clearing: %p\n", head);

    // Try to display the list again
    display_list(head);
}


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
			test_linkedlist();
		    printf("Unknown command. Type 'help' for available commands.\n");
		}

		// Clear the input buffer
		for (int i = 0; i < 256; i++) {
		    buf[i] = '\0';
		}

		putc('\n'); // Print a newline after processing the command
	}

}
