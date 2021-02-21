#include "types.h"
#include "errno.h"
#include "logging.h"

u64 get_file_size(char *filepath, s32 *error_code = 0)
{
    if(error_code) *error_code = 0;
    FILE* fp = fopen(filepath, "r"); 
    if (fp == NULL) 
    { 
        LOG_ERR("Cannot open file %s, errno: %d\n", filepath, errno);
        if(error_code) *error_code = errno;
        return 0; 
    }
    
    s32 status = fseek(fp, 0L, SEEK_END); 
    if(status != 0)
    {
        LOG_ERR("Cannot seek end of file %s, errno: %d\n", filepath, errno);
        if(error_code) *error_code = errno;
        return 0; 
    }
    
    
    s64 res = ftell(fp);
    if(res < 0)
    {
        LOG_ERR("Cannot get size of file %s, errno: %d\n", filepath, errno);
        if(error_code) *error_code = errno;
        return 0; 
    }
    
    
    fclose(fp); 
    return (u64)res; 
}


u64 read_entire_file(char *filepath, u8* buffer, u64 buffer_size)
{
    FILE* fp = fopen(filepath, "r"); 
    if (fp == NULL) 
    { 
        LOG_ERR("Cannot open file %s, errno: %d\n", filepath, errno);
        return errno; 
    }
    
    u64 bytes_read = fread(buffer, sizeof(u8), buffer_size, fp);
    fclose(fp);
    return bytes_read;
}