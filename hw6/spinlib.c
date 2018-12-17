#include "spinlib.h"
#include "tas.h"
#include <sched.h>

int spin_lock(volatile char *lock)
{
	while(tas(lock))
		sched_yield();
	return 1;
}
int spin_unlock(volatile char *lock)
{
	*lock=0;
	return 0;
}
