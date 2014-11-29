#include "mem.h"
#include "defs.h"
#include "kernel.h"
#include "screen.h"

#define SMALL_CHUNK_SIZE 64
#define LARGE_CHUNK_SIZE 1024
#define NUM_SMALL_CHUNKS 1000
#define NUM_LARGE_CHUNKS 1000

void* operator new (size_t size)
{
    return (void*)PhysHeap::getInstance().alloc(size);
}

void* operator new[] (size_t size)
{
    return (void*)PhysHeap::getInstance().alloc(size);
}

void operator delete(void* ptr)
{
    PhysHeap::getInstance().dealloc((uint32_t)ptr);
}

void operator delete[](void* ptr)
{
    PhysHeap::getInstance().dealloc((uint32_t)ptr);
}

void memcpy(char* dst, char* src, size_t size)
{
    for (size_t i=0; i<size; ++i) {
        dst[i] = src[i];
    }
}

void* memset(void* ptr, int value, size_t num)
{
    char* memPtr = (char*)ptr;
    for (size_t i=0; i<num; ++i) {
        memPtr[i] = (uchar)value;
    }
    return ptr;
}

void* kmalloc(size_t s)
{
    return (void*)PhysHeap::getInstance().alloc(s);
}

void kfree(void* ptr)
{
    if (ptr) {
        PhysHeap::getInstance().dealloc((uint32_t)ptr);
    }
}

extern "C" uint32_t end;

void init_paging()
{
    uint32_t memoryStart = (uint32_t)&end;
    kprintf("Memory start: %x\n", memoryStart);
    PhysHeap& heap = PhysHeap::getInstance();
    heap.initialize(memoryStart, memoryStart + 0x1000000);
    uint32_t mmStart = 0xc0000000;
    uint32_t mmSize = NUM_SMALL_CHUNKS*SMALL_CHUNK_SIZE + NUM_LARGE_CHUNKS*LARGE_CHUNK_SIZE + 2 * sizeof(uint32_t);
    uint32_t mmEnd = mmStart + mmSize;
    MemManager* mm = new MemManager();
    PageDir* dir = new PageDir(mm);
    kprintf("AAA: %x %x\n", mm, dir);
    for (int i = 0; i < 5000; ++i) {
        dir->addPage(i * 0x1000, true);
    }
    for (uint32_t i = mmStart; i < mmEnd; i += 0x1000) {
        dir->addPage(i, false);
    }
    dir->activate();
    mm->init(mmStart);
}

PhysHeap::PhysHeap():currDir_(NULL)
{

}

PhysHeap::~PhysHeap()
{

}

PhysHeap& PhysHeap::getInstance()
{
    static PhysHeap instance;
    return instance;
}

void PhysHeap::initialize(uint32_t start, uint32_t end)
{
    memStart_ = start;
    memEnd_ = end;
}

uint32_t PhysHeap::alloc(int sz, bool pageAllocated/* = false*/, uint32_t* addr/* = NULL*/)
{
    if (currDir_) {
        return (uint32_t)currDir_->alloc(sz);
    }

    uint32_t res = memStart_;
    
    if (pageAllocated && res & 0xfffff000) {
        res &= 0xfffff000;
        res += 0x1000;
    }

    if (addr) {
        *addr = res;
    }

    memStart_ = res;
    memStart_ += sz;
    if (res >= memEnd_) {
        panic("Allocation failed. No free space.");
    }

    return res;
}

void PhysHeap::dealloc(uint32_t addr)
{
    if (currDir_) {
        currDir_->dealloc((void*)addr);
        return;
    }
    panic("Freeing pages is not supported");
}

void PhysHeap::switchToPaged(PageDir* dir)
{
    currDir_ = dir;
}

PageDir::PageDir(MemManager* mm) :active_(false), memManager_(mm)
{
    tables_ = (table_t**)PhysHeap::getInstance().alloc(sizeof(table_t)*1024, true);
    for (int i=0; i<1024; ++i) {
        tables_[i] = NULL;
    }
    tablesPhys_ = (uint32_t*)PhysHeap::getInstance().alloc(sizeof(uint32_t)*1024, true);
    for (int i=0; i<1024; ++i) {
        tablesPhys_[i] = 0x2;
    }
}

PageDir::~PageDir()
{

}

void PageDir::addPage(uint32_t addr, bool directMapping)
{
    uint32_t tmp = addr;
    addr /= 0x1000;
    uint32_t physAddr = addr;
    if (!directMapping) {
        PhysHeap::getInstance().alloc(0x1000, true, &physAddr);
        physAddr /= 0x1000;
    }
    int tableIdx = addr / 1024;
    int pageIdx = addr % 1024;
    if (!tables_[tableIdx]) {
        kprintf("Creating table %d. Vaddr: %x ", tableIdx, tmp);
        createTable(tableIdx, physAddr);
    }
    table_t* table = tables_[tableIdx];
    if (table->pages[pageIdx].present) {
        panic("Page already exists.");
    }
    page_t page;
    *(uint32_t*)&page = 0;
    page.present = 1;
    page.rw = 1;
    page.user = 0;
    page.frame = physAddr;
    table->pages[pageIdx] = page;
    //kprintf("Page %d: %x\n", pageIdx, *(int*)&page);
};

extern "C" void loadPageDirectory(uint32_t*);
extern "C" void enablePaging();

void PageDir::activate()
{
    asm volatile("mov %0, %%cr3":: "r"(tablesPhys_));
    uint32_t cr0;
    asm volatile("mov %%cr0, %0": "=r"(cr0));
    cr0 |= 0x80000000; // Enable paging!
    asm volatile("mov %0, %%cr0":: "r"(cr0));
    PhysHeap::getInstance().switchToPaged(this);
    active_ = true;
}

void PageDir::createTable(int idx, uint32_t startAddr)
{
    startAddr *= 0x1000;
    if (tables_[idx]) {
        panic("Cannot create table. Already present.");
    }
    uint32_t phys;
    table_t* table = (table_t*)PhysHeap::getInstance().alloc(sizeof(table_t), true, &phys);
    kprintf("Paddr: %x\n", phys);
    //memset(table->pages, 0, 0x1000);
    for (int i=0; i<1024; ++i) {
        *(uint32_t*)&(table->pages[i]) = startAddr + (i*0x1000 | 2);
    }
    tablesPhys_[idx] = phys | 0x3;
    tables_[idx] = table;
}

void PageDir::deactivate()
{
    active_ = false;
}

void* PageDir::alloc(size_t size)
{
    kassert(active_);
    kassert(memManager_);
    return memManager_->allocate(size);
}

void PageDir::dealloc(void* ptr)
{
    kassert(active_);
    kassert(memManager_);
    memManager_->deallocate(ptr);
}

FrameManager& FrameManager::getInstance()
{
    static FrameManager instance;
    return instance;
}

FrameManager::FrameManager():frame_(0)
{

}

FrameManager::~FrameManager()
{

}

uint32_t FrameManager::getFrame()
{
    uint32_t res = frame_;
    frame_ += 0x1000;
    return res;
}

#define SZ(n) *((uint32_t*)n-1)
#define NEXT(n) *(uint32_t*)n
#define AFTER(n) ((uint32_t)n+SZ(n)+sizeof(uint32_t))

MemManager::MemManager() :chunks_(0), size_(0)
{

}

MemManager::~MemManager()
{
    kassert((uint32_t)smallHeap_ == (uint32_t)startSmall_ + (uint32_t)sizeof(uint32_t));
    kassert((uint32_t)largeHeap_ == (uint32_t)startLarge_ + (uint32_t)sizeof(uint32_t));
    kassert(SZ(smallHeap_) == NUM_SMALL_CHUNKS*SMALL_CHUNK_SIZE);
    kassert(SZ(largeHeap_) == NUM_LARGE_CHUNKS*LARGE_CHUNK_SIZE);
    //free(startSmall_);
    //free(startLarge_);
}

void MemManager::init(uint32_t startAddr)
{
     int smallSize = NUM_SMALL_CHUNKS*SMALL_CHUNK_SIZE + sizeof(uint32_t);
     int largeSize = NUM_LARGE_CHUNKS*LARGE_CHUNK_SIZE + sizeof(uint32_t);
     smallHeap_ = (void*)startAddr;
     memset(smallHeap_, 0xc, smallSize);
     largeHeap_ = (void*)(startAddr + NUM_SMALL_CHUNKS*SMALL_CHUNK_SIZE + sizeof(uint32_t));
     startSmall_ = smallHeap_;
     startLarge_ = largeHeap_;
     endSmall_ = (char*)startSmall_ + smallSize;
     endLarge_ = (char*)startLarge_ + largeSize;
     smallHeap_ = (uint32_t*)smallHeap_ + 1;
     largeHeap_ = (uint32_t*)largeHeap_ + 1;
     SZ(smallHeap_) = NUM_SMALL_CHUNKS*SMALL_CHUNK_SIZE;
     NEXT(smallHeap_) = 0;
     SZ(largeHeap_) = NUM_LARGE_CHUNKS*LARGE_CHUNK_SIZE;
     NEXT(largeHeap_) = 0;
}

void* MemManager::allocate(size_t size)
{
    ++chunks_;
    size_ += (int)size;
    void* res = NULL;
    if (size <= SMALL_CHUNK_SIZE) {
        res = alloc(size, smallHeap_);
    } else {
        res = alloc(size, largeHeap_);
    }
    return res;
}

void MemManager::deallocate(void* ptr)
{
    if (ptr) {
        dealloc(ptr);
    }
}

void MemManager::dbgheap(void* heap)
{
    void* start = NULL;
    void* end = NULL;
    if (heap >= startSmall_ && heap <= endSmall_) {
        start = startSmall_;
        end = endSmall_;
    }
    if (heap >= startLarge_ && heap <= endLarge_) {
        start = startLarge_;
        end = endLarge_;
    }
    while (heap) {
        kassert((void*)NEXT(heap) != heap);
        heap = (void*)NEXT(heap);
        kassert(!heap || (heap >= start && heap <= end));
    }
}

void* MemManager::alloc(size_t size, void*& heap)
{
    if (size < sizeof(uint32_t)) {
        size = sizeof(uint32_t);
    }
    size += sizeof(uint32_t) - size%sizeof(uint32_t);
    kassert(size % sizeof(uint32_t) == 0);
    void* itr = heap;
    void* res = 0;
    void* prev = NULL;
    while (itr) {
        if (SZ(itr) >= (uint32_t)size) {
            break;
        }
        prev = itr;
        itr = (void*)NEXT(itr);
    }
    kassert(itr);
    res = itr;
    uint32_t sz = SZ(itr);
    uint32_t next = NEXT(itr);
    if (sz - size > sizeof(uint32_t)) {
        itr = (char*)itr + size + sizeof(uint32_t);
        SZ(itr) = sz - (uint32_t)size - sizeof(uint32_t);
        SZ(res) = (uint32_t)size;
        NEXT(itr) = next;
        next = (uint32_t)itr;
    }
    if (prev) {
        NEXT(prev) = next ? next : AFTER(res);
    }
    if (res == heap) {
        kassert(!prev);
        heap = next ? (void*)next : (void*)AFTER(res);
    }
    return res;
}

void MemManager::dealloc(void* ptr)
{
    void* heap;
    void** origHeap;
    --chunks_;
    size_ -= SZ(ptr);
    if (ptr > startSmall_ && ptr < endSmall_) {
        heap = smallHeap_;
        origHeap = &smallHeap_;
    }
    else if (ptr > startLarge_ && ptr < endLarge_) {
        heap = largeHeap_;
        origHeap = &largeHeap_;
    }
    else {
        kassert(0);
    }
    void* prev = NULL;
    void* next = NULL;
    while (heap && heap < ptr) {
        prev = heap;
        heap = (void*)NEXT(heap);
    }
    next = heap;
    if (next && (void*)AFTER(ptr) == next) {
        SZ(ptr) = (uint32_t)(SZ(next) + SZ(ptr) + sizeof(uint32_t));
        NEXT(ptr) = NEXT(next);
    }
    else if (next) {
        NEXT(ptr) = (uint32_t)next;
    }
    if (prev && (void*)AFTER(prev) == ptr) {
        SZ(prev) = (uint32_t)(SZ(ptr) + SZ(prev) + sizeof(uint32_t));
        NEXT(prev) = NEXT(ptr);
    }
    else if (prev) {
        NEXT(prev) = (uint32_t)ptr;
    }
    else {
        *origHeap = ptr;
    }
}

