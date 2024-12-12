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

void* simple_heap = (void*)0x10000;
size_t heap_size = 0x10000;

void* simplified_kmalloc(uint32_t size) {
    if (size > heap_size) {
        puts("Out of memory\n");
        return NULL;
    }
    void* allocated_memory = simple_heap;
    simple_heap += size;
    heap_size -= size;
    return allocated_memory;
}

void simplified_kfree(void* ptr) {

    (void)ptr;

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
                puts(itoa(int_arg));
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

Node *create_node(int data) {
    Node *new_node = (Node *)simplified_kmalloc(sizeof(Node));
    if (new_node == NULL) {
        puts("Failed to allocate memory for new node\n");
        return NULL;
    }
    new_node->data = data;
    new_node->next = NULL;
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
        puts(itoa(current->data)); // Convert integer to string and print it
        putc(' ');                // Space between numbers
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

    while (current != NULL) {
        next = current->next; // Store the next node

        simplified_kfree(current); // Free the current node
        current = next; // Move to the next node
        
    }

    *head = NULL; // Set the head pointer to NULL
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



int validate_int(const char *prompt) {
    char input_buf[128];
    int valid = 0;
    int result = 0;

    while (!valid) {
        puts(prompt);
        gets(input_buf, sizeof(input_buf));

        // Check if the input is empty or contains only whitespace
        int i = 0;
        while (input_buf[i] == ' ' || input_buf[i] == '\t') { // Skip leading spaces or tabs
            i++;
        }
        if (input_buf[i] == '\0') { // If after skipping spaces, it's still empty
            puts("Input cannot be empty or whitespace. Please enter a valid integer.\n");
            continue; // Prompt the user again
        }

        // Check if the input is a valid integer
        valid = 1;
        if (input_buf[i] == '-') i++; // Allow negative numbers
        for (; input_buf[i] != '\0'; i++) {
            if (input_buf[i] < '0' || input_buf[i] > '9') {
                valid = 0;
                puts("Invalid input. Please enter a valid integer.\n");
                break;
            }
        }

        if (valid) {
            result = custom_atoi(input_buf);
        }
    }

    return result;
}



void kernel_main(uint32_t r0, uint32_t r1, uint32_t atags) {
    char buf[256];
    char command[32];
    Node *head = NULL; // Initialize LinkedList

    // Declare as unused
    (void) r0;
    (void) r1;
    (void) atags;

    // Initialize UART and memory
    uart_init();
    puts("Initializing Memory Module\n");
    mem_init((atag_t *)atags);

    // Welcome message
    puts("CSC440 Project Fall 2024!\n");

    while (1) {
        puts("> ");
        gets(buf, 256); // Read user input into buffer

        // Clear the command buffer
        for (int i = 0; i < 32; i++) {
            command[i] = '\0';
        }

        // Parse the command and arguments manually
        int i = 0;
        // Extract the command (up to the first space or end of string)
        while (buf[i] != ' ' && buf[i] != '\0') {
            command[i] = buf[i];
            i++;
        }
        command[i] = '\0'; // Null-terminate the command

        // Command handling
        if (custom_strcmp(command, "help") == 0) {
            printf("Available commands:\n");
            printf("help          - Show this help message\n");
            printf("sum           - Calculate the sum of two integers\n");
            printf("addnode       - Add an integer to the LinkedList\n");
            printf("displaylist   - Display the content of the LinkedList\n");
            printf("clearlist     - Clear the content of the LinkedList\n");
            printf("exit          - Exit the kernel loop\n");
        } else if (custom_strcmp(command, "sum") == 0) {
            // Prompt and validate integers
            int num1 = validate_int("Enter first number: ");
            int num2 = validate_int("Enter second number: ");

            printf("The sum is: %d\n", num1 + num2);
        } else if (custom_strcmp(command, "addnode") == 0) {
            // Prompt and validate integer for LinkedList
            int value = validate_int("Enter an integer to add to the LinkedList: ");
            head = add_node(head, value);
            printf("Node with value %d added to the LinkedList.\n", value);
        } else if (custom_strcmp(command, "displaylist") == 0) {
            display_list(head);
        } else if (custom_strcmp(command, "clearlist") == 0) {
            clear_list(&head);
        } else if (custom_strcmp(command, "exit") == 0) {
            puts("Exiting kernel loop...\n");
            break;
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

