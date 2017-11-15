#ifndef __READERS_WRITERS_H
#define __READERS_WRITERS_H



typedef void rw_func_t(void *arg);

void rw_init(void);
void rw_write(rw_func_t do_write, void *arg);
void rw_read(rw_func_t do_read, void *arg);

#endif
