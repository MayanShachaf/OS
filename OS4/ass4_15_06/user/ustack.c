
#include "kernel/stat.h"
#include "user/user.h"
#include "kernel/param.h"
#include "user/ustack.h"

#define MAX_ALLOWED_SIZE 512 // Example maximum allowed size
#define PGSIZE 4096          // Page size (assumed to be 4096 bytes)

typedef struct Header
{
    struct Header *prev;
    uint size;
} Header;

