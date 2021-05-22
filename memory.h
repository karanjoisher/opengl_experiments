#pragma once

#define ARRAY_LENGTH(a) (sizeof(a)/sizeof(a[0]))

#define PUSH_ARRAY(memory, type, count, ...) ((type*) (push_size((memory), (count) * sizeof(type), __VA_ARGS__)))

#define PUSH_TYPE(memory, type, ...) PUSH_ARRAY(memory, type, 1, __VA_ARGS__)
#define COPY(source, destination, size) copy((u8*)(source), (u8*)(destination), size)

struct Memory
{
    u8 *data;
    s32 used;
    s32 size;
};

Memory create_memory(s32 size);
void* push_size(Memory *memory, s32 size, bool clear=true);

