#include <sched.h>
#include <tas.h>

#define N_PROC 64

volatile char *lock=0;
int spin_lock(volatile char *lock);
int spin_unlock(volatile char *lock);

int main(int argc, char **argv)
{
	
}

int spin_lock(volatile char *lock)
{
	while(TAS(lock))
		sched_yield();
	return 1;
}
int spin_unlock(volatile char *lock)
{
	*lock=0;
}
