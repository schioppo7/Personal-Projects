#include <stdint.h>

#define VIDEO_MEMORY 0xB8000
#define SCREEN_WIDTH 80
#define SCREEN_HEIGHT 25
#define HISTORY_MAX 100

static inline uint8_t inb(uint16_t port) {
    uint8_t ret;
    asm volatile("inb %1, %0" : "=a"(ret) : "Nd"(port));
    return ret;
}

static inline void outb(uint16_t port, uint8_t val) {
    asm volatile("outb %0, %1" : : "a"(val), "Nd"(port));
}

uint16_t history_buffer[HISTORY_MAX][SCREEN_WIDTH];
int history_idx = 0;
int view_offset = 0;
int mode = 0; // 0 = Terminal, 1 = Aurora Mode

void update_cursor(int row, int col) {
    if(mode == 1) return;
    uint16_t pos = row * SCREEN_WIDTH + col;
    outb(0x3D4, 0x0F); outb(0x3D5, (uint8_t)(pos & 0xFF));
    outb(0x3D4, 0x0E); outb(0x3D5, (uint8_t)((pos >> 8) & 0xFF));
}

void save_to_history() {
    volatile uint16_t* vga = (uint16_t*) VIDEO_MEMORY;
    for (int i = 0; i < SCREEN_WIDTH; i++) {
        history_buffer[history_idx % HISTORY_MAX][i] = vga[i];
    }
    history_idx++;
}

void scroll() {
    save_to_history();
    volatile uint16_t* vga = (uint16_t*) VIDEO_MEMORY;
    for (int i = 0; i < (SCREEN_HEIGHT - 1) * SCREEN_WIDTH; i++) {
        vga[i] = vga[i + SCREEN_WIDTH];
    }
    for (int i = (SCREEN_HEIGHT - 1) * SCREEN_WIDTH; i < SCREEN_HEIGHT * SCREEN_WIDTH; i++) {
        vga[i] = (uint16_t)' ' | 0x0700;
    }
}

void draw_gui() {
    volatile uint16_t* vga = (uint16_t*) VIDEO_MEMORY;
    for (int i = 0; i < SCREEN_WIDTH * SCREEN_HEIGHT; i++) vga[i] = (uint16_t)' ' | 0x1F00;
    for (int i = 0; i < SCREEN_WIDTH; i++) vga[i] = (uint16_t)205 | 0x7000;
    const char* title = " AURORA DESKTOP - Developer: Ulizi Davide ";
    for (int i = 0; title[i] != '\0'; i++) vga[2 + i] = (uint16_t)title[i] | 0x7000;
    for (int i = 0; i < SCREEN_WIDTH; i++) vga[(SCREEN_HEIGHT-1)*SCREEN_WIDTH + i] = (uint16_t)' ' | 0x7000;
    const char* footer = " [F1] Terminal  |  [Start] Menu ";
    for (int i = 0; footer[i] != '\0'; i++) vga[(SCREEN_HEIGHT-1)*SCREEN_WIDTH + i] = (uint16_t)footer[i] | 0x7400;
}

void put_char(char c, uint8_t color, int row, int col) {
    volatile uint16_t* vga = (uint16_t*) VIDEO_MEMORY;
    vga[row * SCREEN_WIDTH + col] = (uint16_t)c | (uint16_t)color << 8;
}

void print(const char* str, uint8_t color, int* row, int* col) {
    for (int i = 0; str[i] != '\0'; i++) {
        if (*row >= SCREEN_HEIGHT) { scroll(); *row = SCREEN_HEIGHT - 1; }
        put_char(str[i], color, *row, (*col)++);
    }
}

void clear_screen(int* row, int* col) {
    volatile uint16_t* vga = (uint16_t*) VIDEO_MEMORY;
    for (int i = 0; i < SCREEN_WIDTH * SCREEN_HEIGHT; i++) vga[i] = (uint16_t)' ' | 0x0700;
    int r = 0, c = 56; print("Developer: Ulizi Davide", 0x0E, &r, &c);
    *row = 0; *col = 0;
}

int strcmp(const char* s1, const char* s2) {
    while (*s1 && (*s1 == *s2)) { s1++; s2++; }
    return *(const unsigned char*)s1 - *(const unsigned char*)s2;
}

char get_ascii(uint8_t scancode) {
    char ascii[128] = {
        0,  27, '1', '2', '3', '4', '5', '6', '7', '8', '9', '0', '-', '=', '\b',
        '\t', 'q', 'w', 'e', 'r', 't', 'y', 'u', 'i', 'o', 'p', '[', ']', '\n',
        0, 'a', 's', 'd', 'f', 'g', 'h', 'j', 'k', 'l', ';', '\'', '`', 0,
        '\\', 'z', 'x', 'c', 'v', 'b', 'n', 'm', ',', '.', '/', 0, '*', 0, ' '
    };
    return (scancode < 128) ? ascii[scancode] : 0;
}

void kernel_main(void) {
    int row = 0, col = 0; uint8_t text_color = 0x0F;
    clear_screen(&row, &col);
    print("AURORA OS - Multi-Mode", 0x0B, &row, &col);
    row = 2; col = 0; print("Type 'gui' to enter Aurora Mode or 'help' for commands.", 0x07, &row, &col);
    row = 4; col = 0; char buffer[80]; int buf_idx = 0;
    print("> ", 0x0F, &row, &col); update_cursor(row, col);

    while (1) {
        if (inb(0x64) & 1) {
            uint8_t scancode = inb(0x60);
            if (scancode == 0x3B) { // F1 Key
                mode = 0; clear_screen(&row, &col);
                print("AURORA OS - Terminal Mode", 0x0B, &row, &col);
                row = 2; col = 0; print("> ", 0x0F, &row, &col);
            }
            if (!(scancode & 0x80)) {
                if (mode == 1) continue;
                char c = get_ascii(scancode);
                if (c == '\n') {
                    buffer[buf_idx] = '\0'; row++; col = 0;
                    if (row >= SCREEN_HEIGHT) { scroll(); row = SCREEN_HEIGHT - 1; }
                    if (strcmp(buffer, "gui") == 0) { mode = 1; draw_gui(); }
                    else if (strcmp(buffer, "about") == 0) { print("Created by Ulizi Davide", 0x0A, &row, &col); }
                    else if (strcmp(buffer, "clear") == 0) { clear_screen(&row, &col); row=0; }
                    else if (strcmp(buffer, "exit") == 0) { print("System stop.", 0x0C, &row, &col); asm volatile("hlt"); }
                    else if (strcmp(buffer, "help") == 0) { print("about, clear, exit, gui, help", 0x0F, &row, &col); }
                    buf_idx = 0; row++; col = 0;
                    if (row >= SCREEN_HEIGHT) { scroll(); row = SCREEN_HEIGHT - 1; }
                    print("> ", 0x0F, &row, &col);
                } else if (c == '\b' && buf_idx > 0) {
                    buf_idx--; col--; put_char(' ', text_color, row, col);
                } else if (c > 0 && buf_idx < 79) {
                    buffer[buf_idx++] = c; put_char(c, text_color, row, col++);
                }
                update_cursor(row, col);
            }
        }
    }
}
