#include "shm_malloc.h"

#include <assert.h>
#include <stdlib.h>

#include <unistd.h>
#include <sys/mman.h>
#include <fcntl.h>

typedef struct _alloc_record_struct {
    struct _alloc_record_struct *next;
    char *shmName;
    int shmFileDescriptor;
    void *shmAddress;
    size_t shmSize;
} AllocRecord;

AllocRecord* createAllocRecord(char *shmName, int shmFileDescriptor,
    void* shmAddress, size_t shmSize) {
    AllocRecord *newRecord = (AllocRecord*)malloc(sizeof(AllocRecord));

    newRecord->next = NULL;
    newRecord->shmName = (char*)malloc(sizeof(char) * (strlen(shmName) + 1));
    strcpy(newRecord->shmName, shmName);
    newRecord->shmFileDescriptor = shmFileDescriptor;
    newRecord->shmAddress = shmAddress;
    newRecord->shmSize = shmSize;

    return newRecord;
}

static int allocsCount = 0;

static AllocRecord *allocList = NULL;

void appendAlloc(AllocRecord *record) {
    record->next = allocList;
    allocList = record;
}

void* shm_malloc(size_t size) {
    char name[] = "/tmp/shm_life_000000";
    int tmpCount = allocsCount++;
    int index = strlen(name) - 1;
    while (tmpCount) {
        assert(index > strlen(name) - 6);
        name[index--] = tmpCount % 10 + '0';
        tmpCount /= 10;
    }

    int fileDescriptor = open(name, O_RDWR | O_CREAT, 0777);
    ftruncate(fileDescriptor, size);
    void *address = mmap(NULL, size, PROT_WRITE | PROT_READ,
                         MAP_SHARED, fileDescriptor, 0);

    AllocRecord *newRecord = createAllocRecord(name, fileDescriptor, address, size);
    appendAlloc(newRecord);
    return address;
}

void shm_unlink_all() {
    AllocRecord *currentRecord = allocList;
    while (currentRecord) {
        unlink(currentRecord->shmName);
        currentRecord = currentRecord->next;
    }
}

void shm_unmap_and_close_all() {
    AllocRecord *currentRecord = allocList;
    while (currentRecord) {
        munmap(currentRecord->shmAddress, currentRecord->shmSize);
        close(currentRecord->shmFileDescriptor);
        currentRecord = currentRecord->next;
    }
}

void shm_free_records() {
    AllocRecord *currentRecord = allocList;
    AllocRecord *nextRecord;
    while (currentRecord) {
        nextRecord = currentRecord->next;

        free(currentRecord->shmName);
        free(currentRecord);

        currentRecord = nextRecord;
    }

    allocList = NULL;
    allocsCount = 0;
}

typedef struct _semaphore_record_struct {
    struct _semaphore_record_struct *next;
    char *semaphoreName;
    sem_t *semaphore;
} SemaphoreRecord;

SemaphoreRecord* createSemaphoreRecord(char *semaphoreName, sem_t *semaphore) {
    SemaphoreRecord *newRecord = (SemaphoreRecord*)malloc(sizeof(SemaphoreRecord));
    newRecord->next = NULL;
    newRecord->semaphoreName = (char*)malloc(sizeof(char) * (strlen(semaphoreName) + 1));
    strcpy(newRecord->semaphoreName, semaphoreName);
    newRecord->semaphore = semaphore;
    return newRecord;
}

static int semaphoresCount = 0;

static SemaphoreRecord *semaphoreList = NULL;

void appendSemaphore(SemaphoreRecord *newRecord) {
    newRecord->next = semaphoreList;
    semaphoreList = newRecord;
}

sem_t* shm_create_semaphore() {
    char name[] = "/shm_life_sem_000000";
    int tmpCount = semaphoresCount++;
    int index = strlen(name) - 1;
    while (tmpCount) {
        assert(index > strlen(name) - 6);
        name[index--] = tmpCount % 10 + '0';
        tmpCount /= 10;
    }

    sem_t *newSemaphore = sem_open(name, O_EXCL | O_CREAT, 0777, 0);
    assert(newSemaphore > 0);
    appendSemaphore(createSemaphoreRecord(name, newSemaphore));
    return newSemaphore;
}

void shm_close_sems() {
    SemaphoreRecord *record = semaphoreList;
    while (record) {
        sem_close(record->semaphore);
        record = record->next;
    }
}

void shm_destroy_sems() {
    SemaphoreRecord *record = semaphoreList;
    while (record) {
        sem_unlink(record->semaphoreName);
        record = record->next;
    }
}

void shm_free_sems() {
    SemaphoreRecord *record = semaphoreList;
    SemaphoreRecord *nextRecord;
    while (record) {
        nextRecord = record->next;
        free(record);
        record = nextRecord;
    }
    semaphoreList = NULL;
    semaphoresCount = 0;
}