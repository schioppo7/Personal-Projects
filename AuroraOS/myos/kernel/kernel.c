#include <stdint.h>

#define VIDEO_MEMORY 0xB8000
#define SCREEN_WIDTH 80
#define SCREEN_HEIGHT 25

int mode = 0; 
int row = 0, col = 0;
int mouse_x = 40, mouse_y = 12;
uint8_t desktop_bg = 0x00; 
uint8_t current_text_color = 0x0F;

const char* files[] = {"KERNEL.BIN", "NOTES.TXT", "PICTURE.IMG", "SYSTEM.SYS"};
int file_idx = 0;

static inline uint8_t inb(uint16_t port) {
    uint8_t ret;
    asm volatile("inb %1, %0" : "=a"(ret) : "Nd"(port));
    return ret;
}

static inline void outb(uint16_t port, uint8_t val) {
    asm volatile("outb %0, %1" : : "a"(val), "Nd"(port));
}

void disable_cursor() {
    outb(0x3D4, 0x0A); outb(0x3D5, 0x20);
}

void put_char(char c, uint8_t color, int r, int cl) {
    volatile uint16_t* vga = (uint16_t*) VIDEO_MEMORY;
    vga[r * SCREEN_WIDTH + cl] = (uint16_t)c | (uint16_t)color << 8;
}

uint16_t get_char(int r, int cl) {
    volatile uint16_t* vga = (uint16_t*) VIDEO_MEMORY;
    return vga[r * SCREEN_WIDTH + cl];
}

void print(const char* str, uint8_t color, int* r, int* cl) {
    for (int i = 0; str[i] != '\0'; i++) {
        if (*cl >= SCREEN_WIDTH) { *cl = 0; (*r)++; }
        put_char(str[i], color, *r, (*cl)++);
    }
}

void wait(int ticks) {
    for (volatile int i = 0; i < ticks * 100000; i++);
}

int strcmp(const char* s1, const char* s2) {
    while (*s1 && (*s1 == *s2)) { s1++; s2++; }
    return *(const unsigned char*)s1 - *(const unsigned char*)s2;
}

uint8_t get_rtc_reg(int reg) {
    outb(0x70, reg);
    return inb(0x71);
}

void draw_clock() {
    uint8_t sec = get_rtc_reg(0x00);
    uint8_t min = get_rtc_reg(0x02);
    uint8_t hour = get_rtc_reg(0x04);
    int tr = 0, tc = 70;
    uint8_t clock_color = 0x7F; 
    put_char(' ', clock_color, tr, tc++);
    put_char('0' + (hour >> 4), clock_color, tr, tc++);
    put_char('0' + (hour & 0x0F), clock_color, tr, tc++);
    put_char(':', clock_color, tr, tc++);
    put_char('0' + (min >> 4), clock_color, tr, tc++);
    put_char('0' + (min & 0x0F), clock_color, tr, tc++);
    put_char(':', clock_color, tr, tc++);
    put_char('0' + (sec >> 4), clock_color, tr, tc++);
    put_char('0' + (sec & 0x0F), clock_color, tr, tc++);
}

void draw_calc() {
    int r = 10, c = 25;
    print(" +--- CALCULATOR ---+ ", 0x0B, &r, &c); r++; c = 25;
    print(" |  Example: 2+2=4  | ", 0x0F, &r, &c); r++; c = 25;
    print(" +------------------+ ", 0x0B, &r, &c);
}

void draw_files() {
    int r = 10, c = 25;
    print(" +--- FILE MANAGER ---+ ", 0x0E, &r, &c); r++; c = 25;
    for(int i=0; i<4; i++) {
        uint8_t color = (file_idx == i) ? 0x70 : 0x0F;
        print(files[i], color, &r, &c); r++; c = 25;
    }
    print(" +--------------------+ ", 0x0E, &r, &c);
}

void draw_images() {
    int r = 10, c = 25;
    print(" +--- IMAGE VIEWER ---+ ", 0x0D, &r, &c); r++; c = 25;
    print(" |    [ IMAGE LOADED ] | ", 0x07, &r, &c); r++; c = 25;
    print(" |      ( ^ _ ^ )      | ", 0x0D, &r, &c); r++; c = 25;
    print(" +--------------------+ ", 0x0D, &r, &c);
}

void draw_desktop() {
    volatile uint16_t* vga = (uint16_t*) VIDEO_MEMORY;
    uint8_t bg_attr = (desktop_bg << 4) | 0x0F;
    for (int i = 0; i < SCREEN_WIDTH * SCREEN_HEIGHT; i++) vga[i] = (uint16_t)' ' | (bg_attr << 8);
    for (int i = 0; i < SCREEN_WIDTH; i++) vga[i] = (uint16_t)' ' | 0x7000;
    int tr = 0, tc = 2; print("AURORA DESKTOP", 0x7F, &tr, &tc);
    draw_clock();
    put_char(254, 0x0B, 5, 10); tr = 6; tc = 9; print("7.Calc", 0x0F, &tr, &tc);
    put_char(240, 0x0E, 5, 25); tr = 6; tc = 24; print("8.Files", 0x0F, &tr, &tc);
    put_char(219, 0x0D, 5, 40); tr = 6; tc = 39; print("9.Images", 0x0F, &tr, &tc);
    for (int i = 0; i < SCREEN_WIDTH; i++) vga[(SCREEN_HEIGHT-1)*SCREEN_WIDTH + i] = (uint16_t)' ' | 0x7000;
    tr = 24; tc = 2; print("[F1] Term | [789] Apps | [ARROWS] Mouse", 0x70, &tr, &tc);
}

void show_terminal_welcome() {
    volatile uint16_t* vga = (uint16_t*) VIDEO_MEMORY;
    for (int i = 0; i < SCREEN_WIDTH * SCREEN_HEIGHT; i++) vga[i] = (uint16_t)' ' | 0x0700;
    row = 0; col = 0; print("AURORA OS - Terminal", 0x0B, &row, &col);
    row = 1; col = 0; print("if you need help write help", 0x07, &row, &col);
    row = 3; col = 0;
}

void kernel_main(void) {
    disable_cursor(); 
    show_terminal_welcome();
    print("> ", 0x0F, &row, &col);
    char buffer[80]; int buf_idx = 0;
    uint16_t under_mouse = (uint16_t)' ' | (desktop_bg << 12);

    while (1) {
        if (mode >= 1) {
            draw_clock();
            put_char(under_mouse & 0xFF, (under_mouse >> 8) & 0xFF, mouse_y, mouse_x);
            if (inb(0x64) & 1) {
                uint8_t scancode = inb(0x60);
                if (scancode == 0x3B) { mode = 0; show_terminal_welcome(); print("> ", 0x0F, &row, &col); continue; }
                if (scancode == 0x48 && mouse_y > 1) mouse_y--;
                if (scancode == 0x50 && mouse_y < 23) mouse_y++;
                if (scancode == 0x4B && mouse_x > 0) mouse_x--;
                if (scancode == 0x4D && mouse_x < 79) mouse_x++;
                if (scancode == 0x08) { draw_desktop(); mode = 2; draw_calc(); }
                if (scancode == 0x09) { draw_desktop(); mode = 3; draw_files(); }
                if (scancode == 0x0A) { draw_desktop(); mode = 4; draw_images(); }
                under_mouse = get_char(mouse_y, mouse_x);
                put_char('+', 0x0F, mouse_y, mouse_x);
            }
        } else {
            if (inb(0x64) & 1) {
                uint8_t scancode = inb(0x60);
                if (!(scancode & 0x80)) {
                    char ascii[] = {0,27,'1','2','3','4','5','6','7','8','9','0','-','=','\b','\t','q','w','e','r','t','y','u','i','o','p','[',']','\n',0,'a','s','d','f','g','h','j','k','l',';','\'','`',0,'\\','z','x','c','v','b','n','m',',','.','/',0,'*',0,' '};
                    char c = (scancode < 128) ? ascii[scancode] : 0;
                    if (c == '\n') {
                        buffer[buf_idx] = '\0'; row++; col = 0;
                        if (strcmp(buffer, "help") == 0) print("Commands: about, clear, desktop, color, exit", 0x0F, &row, &col);
                        else if (strcmp(buffer, "desktop") == 0) { mode = 1; draw_desktop(); under_mouse = get_char(mouse_y, mouse_x); }
                        else if (strcmp(buffer, "clear") == 0) show_terminal_welcome();
                        else if (strcmp(buffer, "exit") == 0) { print("System Halted.", 0x0C, &row, &col); asm volatile("hlt"); }
                        else if (buf_idx > 0) print("Unknown command.", 0x0C, &row, &col);
                        buf_idx = 0; row++; col = 0; print("> ", 0x0F, &row, &col);
                    } else if (c == '\b' && buf_idx > 0) {
                        buf_idx--; col--; put_char(' ', current_text_color, row, col);
                    } else if (c > 0 && buf_idx < 79) {
                        buffer[buf_idx++] = c; put_char(c, current_text_color, row, col++);
                    }
                }
            }
        }
    }
}
