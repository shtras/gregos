#pragma once
#include "defs.h"

#define VIDEO_MEM 0xb8000
#define ROWS 25
#define COLS 80

#define REG_SCREEN_CTRL 0x3d4
#define REG_SCREEN_DATA 0x3d5

void scr_test();

class ScreenManager
{
public:
    enum VGA_COLOR
    {
        COLOR_BLACK = 0,
        COLOR_BLUE = 1,
        COLOR_GREEN = 2,
        COLOR_CYAN = 3,
        COLOR_RED = 4,
        COLOR_MAGENTA = 5,
        COLOR_BROWN = 6,
        COLOR_LIGHT_GREY = 7,
        COLOR_DARK_GREY = 8,
        COLOR_LIGHT_BLUE = 9,
        COLOR_LIGHT_GREEN = 10,
        COLOR_LIGHT_CYAN = 11,
        COLOR_LIGHT_RED = 12,
        COLOR_LIGHT_MAGENTA = 13,
        COLOR_LIGHT_BROWN = 14,
        COLOR_WHITE = 15,
    };
    static ScreenManager& getInstance();
    void putCursor(int row, int col);
    void returnCursor();
    void print(const char* str);
    void print(int val);
    void printHex(uint32_t val);
    void putChar(char c);
    void clearScreen(uchar color = COLOR_BLACK);
    void setColor(uchar fg, uchar bg);
private:
    ScreenManager();
    ~ScreenManager();

    void moveCursor(int row, int col);
    void shiftCursor();
    int getOffset(int row, int col);
    void scrollLine();
    void clearLine(int line, uchar color = COLOR_BLACK);

    int row_;
    int col_;
    int oldRow_;
    int oldCol_;
    char* vMem_;
    bool disableScroll_;
    uchar color_;
};

void kprintf(const char* fmt, ...);
