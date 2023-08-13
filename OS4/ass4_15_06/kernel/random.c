#include <stdarg.h>

#include "types.h"
#include "param.h"
#include "spinlock.h"
#include "sleeplock.h"
#include "fs.h"
#include "file.h"
#include "memlayout.h"
#include "riscv.h"
#include "defs.h"
#include "proc.h"

struct
{
    struct spinlock lock;
    uint8 random_seed;
} rand;
void randominit(void)
{
    initlock(&rand.lock, "random");

    rand.random_seed = 0x2A;

    devsw[RANDOM].read = randomread;
    devsw[RANDOM].write = randomwrite;
}

// Linear feedback shift register
// Returns the next pseudo-random number
// The seed is updated with the returned value
uint8 lfsr_char(uint8 lfsr)
{
    uint8 bit;
    bit = ((lfsr >> 0) ^ (lfsr >> 2) ^ (lfsr >> 3) ^ (lfsr >> 4)) & 0x01;
    lfsr = (lfsr >> 1) | (bit << 7);
    return lfsr;
}

int randomread(int user_dst, uint64 dst, int n)
{

    acquire(&rand.lock); // Acquire the lock to protect shared state
    int i;
    for (i = 0; i < n; i++)
    {
        rand.random_seed = lfsr_char(rand.random_seed);
        if (either_copyout(user_dst, dst, &rand.random_seed, 1) == -1)
            break;
        dst++;
    }

    release(&rand.lock); // Release the lock after updating the seed
    return n;
}

int randomwrite(int user_src, uint64 src, int n)
{
    uint8 buf;

    if (n != 1)
    {
        // Only a single byte can be used to seed the generator
        return -1;
    }
    if (either_copyin(&buf, user_src, src, 1) == -1)
    {
        return -1;
    }
    acquire(&rand.lock); // Acquire the lock to protect shared state
    rand.random_seed = buf;
    release(&rand.lock); // Release the lock after updating the seed

    return 1;
}