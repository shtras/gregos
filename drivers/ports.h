#pragma once
#include "defs.h"

uchar inb(ushort port);

void outb(ushort port, uchar data);

ushort inw(ushort port);

void outw(ushort port, ushort data);

