#include "memory.h"
#include "logging.h"
#include <stdlib.h>
#include <string.h>

// TODO(Karan): This kind of memory handling is TEMPORARY! Replace with a better system as we understand the need of the application.
Memory create_memory(s32 size)
{
    Memory memory = {};
    memory.data = (u8*)calloc(size, sizeof(u8));
    if(memory.data)
    {
        memory.size = size;
    }
    return memory;
}

void* push_size(Memory *memory, s32 size, bool clear)
{
    u8 *data = 0;
    if(size > 0)
    {
        s32 free = (memory->size - memory->used);
        ASSERT(free >= 0, "Free size in memory block is negative: %d !", free);
        if(free >= size)
        {
            data = memory->data + memory->used;
            if(clear) memset(data, 0, size);
            memory->used += size; 
        }
        else
        {
            LOG_ERR("Cannot push %d bytes in memory. %d/%d used.\n", size, memory->used, memory->size);
        }
    }
    
    return data;
}

void copy(u8 *source, u8* destination, s32 size)
{
    if(source && destination) memcpy(destination, source, size);
}

s32 len_cstring(const char *str)
{
    return (s32)strlen(str);
}