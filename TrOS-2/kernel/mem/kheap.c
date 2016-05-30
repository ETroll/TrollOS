// kheap.c
// Simple kernel heap implementation
// Disclaimer! Not optimal at all! Uses *too much* memspace, and cpu cycles atm..

#include <tros/tros.h>
#include <tros/kheap.h>
#include <tros/pmm.h>
#include <tros/vmm.h>

#define KERNEL_HEAP_START   0xD0000000
#define KERNEL_HEAP_END     0xDFFFFFFF
#define CHUNK_ID            0xCAFEB00D
#define BLOCK_SIZE          4096
#define BLOCK_ALIGN         BLOCK_SIZE

struct heap_chunk_t
{
    struct heap_chunk_t* next;
    struct heap_chunk_t* prev;
    unsigned int size;
    unsigned int chend;
};

static struct heap_chunk_t* _kheap_start = 0;
static struct heap_chunk_t* _kheap_free = 0;

static struct heap_chunk_t* kheap_extend(unsigned int amount);
static struct heap_chunk_t* kheap_first_free(unsigned int amount);
static void kheap_add_chunk_heap(struct heap_chunk_t* chunk);
static void kheap_remove_chunk_heap(struct heap_chunk_t* chunk);
static void kheap_add_chunk_free(struct heap_chunk_t* chunk);
static void kheap_remove_chunk_free(struct heap_chunk_t* chunk);
static void kheap_add_overflow_to_free(struct heap_chunk_t* chunk, unsigned int size);
static void kheap_merge_freelist_bottom(struct heap_chunk_t* chunk);
static void kheap_merge_freelist_top(struct heap_chunk_t* chunk);

void kheap_initialize()
{
    _kheap_free = 0;
    _kheap_start = 0;
}

void* kmalloc(unsigned int size)
{
    //printk("Allocating: %d bytes ", size);
    struct heap_chunk_t* chunk = 0;

    if(_kheap_free == 0)
    {
        //printk("E ");
        chunk = kheap_extend(size);
        //printk("%x ", chunk);
    }
    else
    {
        //printk("S ");
        chunk = kheap_first_free(size);
        if(chunk == 0)
        {
            //printk("F0 %x ",_kheap_free);
            // Is the address of the last free chunk larger than last chunk on the heaplist?
            //   Yes:    Increase with (wanted size)-(sizeof last free chunk)
            //           Combine the old chunk with the new chunk and add to heaplist
            //   No:     Increase with full wanted size and add to heaplist
            if(_kheap_start != 0 && _kheap_free->prev > _kheap_start->prev)
            {
                struct heap_chunk_t* last = _kheap_free->prev;
                //TODO: if((int)(size - last->size) > 0)

                kheap_remove_chunk_free(last);
                //printk("L %d %d ", last->size, size - last->size);
                chunk = kheap_extend(size - last->size);
                //printk("E %d ", size - last->size);
                last->size += sizeof(struct heap_chunk_t) + chunk->size;
                chunk = last;
            }
            else
            {
                //printk("E %d ", size);
                chunk = kheap_extend(size);
            }
        }
        else
        {
            //printk("FR ");
            kheap_remove_chunk_free(chunk);
        }
    }
    //printk("A ");
    kheap_add_chunk_heap(chunk);
    if(chunk->size > size)
    {
        //printk("OV %d/%d ", chunk->size, size);
        kheap_add_overflow_to_free(chunk, size);
    }
    //printk("\n");
    return (void*)((unsigned int)chunk +sizeof(struct heap_chunk_t));
}

void kfree(void* ptr)
{
    struct heap_chunk_t* chunk = (struct heap_chunk_t*) (ptr-sizeof(struct heap_chunk_t));

    if(chunk->chend == CHUNK_ID)
    {
        //printk("Free - C: %x S: %d\n", chunk, chunk->size);
        kheap_remove_chunk_heap(chunk);
        kheap_add_chunk_free(chunk);
    }
    else
    {
        //PANIC!
        printk("ERROR! - Trying to free unallocated memory at %x\n", ptr);
    }
    //printk("\n");
}

static struct heap_chunk_t* kheap_extend(unsigned int size)
{
    static unsigned int _kheap_next_virtual_addr = KERNEL_HEAP_START;
    unsigned int chunk_start_addr = _kheap_next_virtual_addr;

    int amount = ((size + sizeof(struct heap_chunk_t)) / BLOCK_SIZE);
    if(((size + sizeof(struct heap_chunk_t)) % BLOCK_SIZE) != 0)
    {
        amount++;
    }
    for(int i = 0; i<amount; i++)
    {
        void* phys = pmm_alloc_block();
        vmm_map_page(phys, (void*)_kheap_next_virtual_addr);
        _kheap_next_virtual_addr += BLOCK_SIZE;
    }

    struct heap_chunk_t* newchunk = (struct heap_chunk_t*) chunk_start_addr;
    newchunk->size = (amount*BLOCK_SIZE) - 16;
    newchunk->chend = CHUNK_ID;

    return newchunk;
}

static struct heap_chunk_t* kheap_first_free(unsigned int amount)
{
    if(_kheap_free != 0)
    {
        struct heap_chunk_t* iterator = _kheap_free;
        struct heap_chunk_t* ret = 0;
        do
        {
            //printk("%d/%d ", iterator->size, amount);
            if(iterator->size >= amount)
            {
                //printk("F ");
                ret = iterator;
                break;
            }
            iterator = iterator->next;
        } while(iterator != _kheap_free);
        return ret;
    }
    else
    {
        //printk("NF ");
        return 0;
    }
}

static void kheap_add_overflow_to_free(struct heap_chunk_t* chunk, unsigned int size)
{
    //printk("AOVF ");
    struct heap_chunk_t* overflow = (struct heap_chunk_t*)((unsigned int)chunk
        + sizeof(struct heap_chunk_t)
        + size);
    overflow->size = chunk->size - size - sizeof(struct heap_chunk_t);
    overflow->chend = CHUNK_ID;
    chunk->size = size;
    kheap_add_chunk_free(overflow);
}

static void kheap_add_chunk_free(struct heap_chunk_t* chunk)
{
    //printk("ACF ");
    if(_kheap_free == 0)
    {
        _kheap_free = chunk;
        _kheap_free->next = _kheap_free;
        _kheap_free->prev = _kheap_free;
        //printk("F ");
    }
    else if(chunk < _kheap_free)
    {
        //printk("NH %x < %x ", chunk, _kheap_free);

        chunk->next = _kheap_free;
        chunk->prev = _kheap_free->prev;
        _kheap_free->prev->next = chunk;
        _kheap_free->prev = chunk;
        _kheap_free = chunk;

        //printk("\n P: ");
        // struct heap_chunk_t* iterator =  _kheap_free;
        // do
        // {
        //     printk("%x (%d)-",iterator, iterator->size);
        //     iterator = iterator->prev;
        // } while(iterator != _kheap_free);
        //printk("\n N: ");

        // iterator = _kheap_free;
        // do
        // {
            //printk("%x (%d)-",iterator, iterator->size);
        //     iterator = iterator->next;
        // } while(iterator != _kheap_free);
    }
    else
    {
        //printk("E ");
        struct heap_chunk_t* iterator =  _kheap_free->prev;
        do
        {
            //printk(".");
            if(iterator < chunk)
            {
                chunk->next = iterator->next;
                chunk->prev = iterator;
                iterator->next->prev = chunk;
                iterator->next = chunk;

                break;
            }
            iterator = iterator->prev;
        } while(iterator->prev != _kheap_free);
    }
    kheap_merge_freelist_bottom(chunk);
    kheap_merge_freelist_top(chunk);
}

static void kheap_add_chunk_heap(struct heap_chunk_t* chunk)
{
    if(chunk == _kheap_free)
    {
        if(_kheap_free->next == _kheap_free)
        {
            _kheap_free = 0;
        }
        else
        {
            _kheap_free->prev->next = _kheap_free->next;
            _kheap_free->next->prev = _kheap_free->prev;
            _kheap_free = _kheap_free->next;
        }
    }

    if(_kheap_start == 0)
    {
        _kheap_start = chunk;
        _kheap_start->next = _kheap_start;
        _kheap_start->prev = _kheap_start;
    }
    else if(chunk < _kheap_start)
    {
        chunk->next = _kheap_start;
        chunk->prev = _kheap_start->prev;
        _kheap_start->prev->next = chunk;
        _kheap_start->prev = chunk;
        _kheap_start = chunk;
    }
    else
    {
        struct heap_chunk_t* iterator =  _kheap_start->prev;
        do
        {
            if(iterator < chunk)
            {
                chunk->next = iterator->next;
                chunk->prev = iterator;
                iterator->next->prev = chunk;
                iterator->next = chunk;

                break;
            }
            iterator = iterator->prev;
        } while(iterator->prev != _kheap_start);
    }
}

static void kheap_remove_chunk(struct heap_chunk_t* chunk)
{
    chunk->prev->next = chunk->next;
    chunk->next->prev = chunk->prev;
    chunk->next = 0;
    chunk->prev = 0;
}

static void kheap_remove_chunk_heap(struct heap_chunk_t* chunk)
{
    if(chunk == _kheap_start)
    {
        if(_kheap_start->next == _kheap_start)
        {
            _kheap_start = 0;
        }
        else
        {
            _kheap_start = _kheap_start->next;
        }
    }
    kheap_remove_chunk(chunk);
}

static void kheap_remove_chunk_free(struct heap_chunk_t* chunk)
{
    if(chunk == _kheap_free)
    {
        if(_kheap_free->next == _kheap_free)
        {
            _kheap_free = 0;
        }
        else
        {
            _kheap_free = _kheap_free->next;
        }
    }
    kheap_remove_chunk(chunk);
}

static void kheap_merge_freelist_bottom(struct heap_chunk_t* chunk)
{
    struct heap_chunk_t* next = chunk->next;

    unsigned int chunk_end = (unsigned int)chunk
        + sizeof(struct heap_chunk_t)
        + chunk->size;

    if((struct heap_chunk_t*)chunk_end == chunk->next)
    {
        //printk("MB ");
        chunk->size += next->size + sizeof(struct heap_chunk_t);
        chunk->next = next->next;
        next->next->prev = chunk;
    }
}

static void kheap_merge_freelist_top(struct heap_chunk_t* chunk)
{
    struct heap_chunk_t* prev = chunk->prev;

    unsigned int prev_end = (unsigned int)prev
        + sizeof(struct heap_chunk_t)
        + prev->size;

    if((unsigned int)chunk == prev_end)
    {
        //printk("MT ");
        prev->size += chunk->size + sizeof(struct heap_chunk_t);
        prev->next = chunk->next;
        prev->next->prev = prev;
    }
}
