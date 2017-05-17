#ifndef SHM_MALLOC_H

#define SHM_MALLOC_H

#include <memory.h>
#include <semaphore.h>

void* shm_try_open(size_t size);

void* shm_malloc(size_t size);

void shm_unlink_all();

void shm_unmap_and_close_all();

void shm_free_records();

sem_t* shm_create_semaphore();

void shm_close_sems();

void shm_destroy_sems();

void shm_free_sems();

#endif