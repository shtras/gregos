#pragma once
#include "defs.h"

void memcpy(char* dst, char* src, size_t size);
void* memset(void* ptr, int value, size_t num);

void* kmalloc(size_t s);
void kfree(void* ptr);

void* operator new (size_t size);
void* operator new[] (size_t size);
void operator delete(void* ptr);
void operator delete[](void* ptr);

void init_paging();

class MemManager;
class PageDir;

class PhysHeap
{
public:
    static PhysHeap& getInstance();
    void initialize(uint32_t start, uint32_t end_);
    uint32_t alloc(int sz, bool pageAllocated = false, uint32_t* addr = NULL);
    void dealloc(uint32_t addr);
    void switchToPaged(PageDir* dir);
private:
    PhysHeap();
    ~PhysHeap();
    uint32_t memStart_;
    uint32_t memEnd_;
    PageDir* currDir_;
};

class FrameManager
{
public:
    static FrameManager& getInstance();
    uint32_t getFrame();
private:
    FrameManager();
    ~FrameManager();
    uint32_t frame_;
};

class PageDir
{
public:
    PageDir(MemManager* mm);
    ~PageDir();
    void addPage(uint32_t addr, bool directMapping);
    void activate();
    void deactivate();
    void* alloc(size_t size);
    void dealloc(void* ptr);
private:
    void createTable(int idx, uint32_t startAddr);
    struct page_t
    {
        uint32_t present    : 1;
        uint32_t rw         : 1;
        uint32_t user       : 1;
        uint32_t accessed   : 1;
        uint32_t dirty      : 1;
        uint32_t unused     : 7;
        uint32_t frame      : 20;
    };

    struct table_t
    {
        page_t pages[1024];
    };

    bool active_;
    MemManager* memManager_;
    table_t** tables_;
    uint32_t* tablesPhys_;
    uint32_t addrPhys_;
};

class MemManager
{
public:
    MemManager();
    ~MemManager();
    void init(uint32_t startAddr);
    void* allocate(size_t size);
    void deallocate(void* ptr);
private:
    void* alloc(size_t size, void*& heap);
    void dealloc(void* ptr);
    void dbgheap(void* heap);
    void* smallHeap_;
    void* largeHeap_;
    void* startSmall_;
    void* startLarge_;
    void* endSmall_;
    void* endLarge_;
    int chunks_;
    int size_;
};

