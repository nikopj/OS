#ifndef __SEM_H
struct sem;
void sem_init(struct sem *s, int count);
int sem_try(struct sem *s);
void sem_wait(struct sem *s);
void sem_inc(struct sem *s);
#define __SEM_H
#endif
