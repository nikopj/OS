#ifndef __SPINLIB_H
int spin_lock(volatile char *lock);
int spin_unlock(volatile char *lock);
#define __SPINLIB_H
#endif

