#include "screen.h"
#include "ports.h"
#include "string.h"
#include <stdarg.h>
#include "mem.h"

void scr_test()
{
    outb(REG_SCREEN_CTRL, 14);
    outb(REG_SCREEN_DATA, 10);
    outb(REG_SCREEN_CTRL, 15);
}

ScreenManager::ScreenManager():row_(0), col_(0), vMem_((char*)VIDEO_MEM), disableScroll_(false)
{
    setColor(COLOR_WHITE, COLOR_BLACK);
}

ScreenManager::~ScreenManager()
{

}

ScreenManager& ScreenManager::getInstance()
{
    static ScreenManager instance;
    return instance;
}

void ScreenManager::moveCursor(int row, int col)
{
    row_ = row;
    col_ = col;
    int offset = (row_*COLS+col_);
    outb(REG_SCREEN_CTRL, 14);
    outb(REG_SCREEN_DATA, (uchar)(offset >> 8));
    outb(REG_SCREEN_CTRL, 15);
    outb(REG_SCREEN_DATA, (uchar)(offset & 0xff));
}

void ScreenManager::print(const char* str)
{
    while (*str) {
        putChar(*str++);
    }
}

void ScreenManager::print(int val)
{
    if (val<0) {
        putChar('-');
        val = -val;
    }
    if (val == 0) {
        putChar('0');
        return;
    }
    int tmp = val;
    int div = 1;
    while (tmp) {
        tmp /= 10;
        div *= 10;
    }
    while(div > 9) {
        div /= 10;
        int c = val / div;
        putChar(c + '0');
        val -= c*div;
    }
}

void ScreenManager::printHex(uint32_t val)
{

    putChar('0');
    putChar('x');
    if (val == 0) {
        putChar('0');
        return;
    }
    int digits = 0;
    uint32_t tmp = val;
    while (tmp) {
        ++digits;
        tmp >>= 4;
    }
    while (digits) {
        --digits;
        int c = (val & (0xf << (digits*4))) >> (digits*4);
        if (c <= 9) {
            putChar(c+'0');
        } else {
            putChar(c-10+'a');
        }
    }
}

void ScreenManager::putChar(char c)
{
    if (c == '\n') {
        moveCursor(row_, COLS);
    } else {
        int offset = getOffset(row_, col_);
        vMem_[offset] = c;
        vMem_[offset+1] = color_;
    }
    shiftCursor();
}

void ScreenManager::shiftCursor()
{
    ++col_;
    if (col_ > COLS) {
        ++row_;
        col_ = 0;
    }
    if (row_ >= ROWS) {
        scrollLine();
        row_ = ROWS-1;
        col_ = 0;
    }
    moveCursor(row_, col_);
}

int ScreenManager::getOffset( int row, int col )
{
    return (row * COLS + col)*2;
}

void ScreenManager::clearScreen(uchar color/* = COLOR_BLACK*/)
{
    for (int i=0; i<ROWS; ++i) {
        clearLine(i, color);
    }
}

void ScreenManager::scrollLine()
{
    if (disableScroll_) {
        return;
    }
    for (int i=0; i<ROWS-1; ++i) {
        memcpy(vMem_+i*COLS*2, vMem_+(i+1)*COLS*2, COLS*2);
    }
    clearLine(ROWS);
}

void ScreenManager::clearLine(int line, uchar color/* = COLOR_BLACK*/)
{
    disableScroll_ = true;
    int row = row_;
    int col = col_;
    int currColor = color_;
    setColor(COLOR_WHITE, color);
    moveCursor(line, 0);
    for (int i=0; i<COLS; ++i) {
        putChar(' ');
    }
    color_ = currColor;
    row_ = row;
    col_ = col;
    disableScroll_ = false;
}

void ScreenManager::setColor( uchar fg, uchar bg )
{
    color_ = fg | bg << 4;
}

void ScreenManager::returnCursor()
{
    moveCursor(oldRow_, oldCol_);
}

void ScreenManager::putCursor(int row, int col)
{
    oldRow_ = row_;
    oldCol_ = col_;
    moveCursor(row, col);
}

void kprintf(const char* fmt, ...)
{
    ScreenManager& manager = ScreenManager::getInstance();
    va_list vl;
    int sz = strlen(fmt);
    va_start(vl, fmt);
    for (int i=0; i<sz; ++i) {
        if (fmt[i] != '%') {
            manager.putChar(fmt[i]);
            continue;
        }
        char type = fmt[++i];
        switch(type) {
        case 'd':
        case 'l':
            manager.print(va_arg(vl, int));
            break;
        case 's':
            manager.print(va_arg(vl, char*));
            break;
        case 'x':
            manager.printHex(va_arg(vl, uint32_t));
            break;
        case '%':
            manager.putChar('%');
            break;
        default:
            break;
        }
    }
    va_end(vl);
}
