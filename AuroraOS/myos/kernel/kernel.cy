#include <stdint.h>

#define VIDEO_MEMORY 0xB8000
#define SCREEN_WIDTH 80
#define SCREEN_HEIGHT 25

// --- STATI ---
int mode = 0; // 0: Terminale, 1: Desktop
int row = 0, col = 0;
uint8_t desktop_bg = 0x00; 

// --- I/O PORT HELPERS ---
static inline uint8_t inb(uint16_t port) {
    uint8_t ret;
    asm volatile("inb %1, %0" : "=a"(ret) : "Nd"(port));
    return ret;
}

static inline void outb(uint16_t port, uint8_t val) {
    asm volatile("outb %0, %1" : : "a"(val), "Nd"(port));
}

// --- FUNZIONI VIDEO ---
void disable_cursor() {
    outb(0x3D4, 0x0A);
    outb(0x3D5, 0x20); // Spegne la barretta lampeggiante
}

void put_char(char c, uint8_t color, int r, int cl) {
    volatile uint16_t* vga = (uint16_t*) VIDEO_MEMORY;
    vga[r * SCREEN_WIDTH + cl] = (uint16_t)c | (uint16_t)color << 8;
}

void print(const char* str, uint8_t color, int* r, int* cl) {
    for (int i = 0; str[i] != '\0'; i++) {
        if (*r >= SCREEN_HEIGHT) *r = SCREEN_HEIGHT - 1;
        put_char(str[i], color, *r, (*cl)++);
    }
}

// --- DESKTOP MODE ---
void draw_desktop() {
    volatile uint16_t* vga = (uint16_t*) VIDEO_MEMORY;
    uint8_t attr = (desktop_bg << 4) | 0x0F;
    for (int i = 0; i < SCREEN_WIDTH * SCREEN_HEIGHT; i++) vga[i] = (uint16_t)' ' | (attr << 8);
    
    // Header
    for (int i = 0; i < SCREEN_WIDTH; i++) vga[i] = (uint16_t)' ' | 0x7000;
    int tr = 0, tc = 2; print("AURORA DESKTOP - Mode: Graphics Style", 0x70, &tr, &tc);

    // Icone (statiche)
    put_char(201, 0x0B, 5, 10); put_char('%', 0x0B, 5, 11); put_char(187, 0x0B, 5, 12);
    tr = 6; tc = 9; print("Calc", 0x0F, &tr, &tc);

    put_char(201, 0x0E, 5, 25); put_char(240, 0x0E, 5, 26); put_char(187, 0x0E, 5, 27);
    tr = 6; tc = 24; print("Files", 0x0F, &tr, &tc);

    // Footer
    for (int i = 0; i < SCREEN_WIDTH; i++) vga[(SCREEN_HEIGHT-1)*SCREEN_WIDTH + i] = (uint16_t)' ' | 0x7000;
    tr = 24; tc = 2; print("[F1] Terminal | [1-3] Cambia Colore Sfondo", 0x74, &tr, &tc);
}

// --- TERMINAL MODE ---
void show_terminal_welcome() {
    volatile uint16_t* vga = (uint16_t*) VIDEO_MEMORY;
    for (int i = 0; i < SCREEN_WIDTH * SCREEN_HEIGHT; i++) vga[i] = (uint16_t)' ' | 0x0700;
    int r_w = 0, c_w = 56; print("Developer: Ulizi Davide", 0x0E, &r_w, &c_w);
    row = 0; col = 0; print("AURORA OS - Terminal Mode", 0x0B, &row, &col);
    row = 2; col = 0; print("Comandi: about, clear, desktop, exit, help", 0x07, &row, &col);
    row = 4; col = 0;
}

int strcmp(const char* s1, const char* s2) {
    while (*s1 && (*s1 == *s2)) { s1++; s2++; }
    return *(const unsigned char*)s1 - *(const unsigned char*)s2;
}

// --- MAIN ---
void kernel_main(void) {
    char buffer[80]; int buf_idx = 0;
    disable_cursor(); 
    show_terminal_welcome();
    print("> ", 0x0F, &row, &col);

    while (1) {
        if (inb(0x64) & 1) {
            uint8_t scancode = inb(0x60);
            
            // F1 per tornare al terminale
            if (scancode == 0x3B) { 
                mode = 0; show_terminal_welcome(); print("> ", 0x0F, &row, &col);
                continue;
            }

            if (mode == 1) { // Input Desktop
                if (scancode == 0x02) { desktop_bg = 0x0; draw_desktop(); } // 1: Nero
                if (scancode == 0x03) { desktop_bg = 0x1; draw_desktop(); } // 2: Blu
                if (scancode == 0x04) { desktop_bg = 0x4; draw_desktop(); } // 3: Rosso
            } else { // Input Terminale
                if (!(scancode & 0x80)) {x
