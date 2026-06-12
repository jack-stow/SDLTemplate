#include "workerpool.h"

void WorkerPool_Run(WorkerPool* pool, int itemCount, int chunkSize, int activeThreads, WorkerJobFunc jobFunc, void* jobData)
{
    if (activeThreads > pool->numThreads)
        activeThreads = pool->numThreads;

    if (activeThreads < 1)
        activeThreads = 1;


    SDL_LockMutex(pool->mutex);

    pool->nextChunkStart = 0;
    pool->chunkSize = chunkSize;
    pool->itemCount = itemCount;
    pool->activeThreads = activeThreads;
    pool->jobFunc = jobFunc;
    pool->jobData = jobData;

    pool->completed = 0;
    pool->generation++;

    SDL_CondBroadcast(pool->startCond);

    while (pool->completed < pool->numThreads)
    {
        SDL_CondWait(pool->doneCond, pool->mutex);
    }

    SDL_UnlockMutex(pool->mutex);
}



int PersistentWorkerMainBalanced(void* data)
{
    int threadIndex = *(int*)data;
    int lastGeneration = 0;

    while (1)
    {
        SDL_LockMutex(workerPool.mutex);

        while (!workerPool.quit && workerPool.generation == lastGeneration)
        {
            SDL_CondWait(workerPool.startCond, workerPool.mutex);
        }

        if (workerPool.quit)
        {
            SDL_UnlockMutex(workerPool.mutex);
            return 0;
        }

        lastGeneration = workerPool.generation;

        if (threadIndex >= workerPool.activeThreads)
        {
            workerPool.completed++;

            if (workerPool.completed == workerPool.numThreads)
            {
                SDL_CondSignal(workerPool.doneCond);
            }

            SDL_UnlockMutex(workerPool.mutex);
            continue;
        }

        WorkerJobFunc jobFunc = workerPool.jobFunc;
        void* jobData = workerPool.jobData;

        SDL_UnlockMutex(workerPool.mutex);

        while (1)
        {
            SDL_LockMutex(workerPool.chunkMutex);

            int start = workerPool.nextChunkStart;
            workerPool.nextChunkStart += workerPool.chunkSize;

            SDL_UnlockMutex(workerPool.chunkMutex);

            if (start >= workerPool.itemCount)
            {
                break;
            }

            int end = start + workerPool.chunkSize;

            if (end > workerPool.itemCount)
            {
                end = workerPool.itemCount;
            }

            jobFunc(jobData, start, end, threadIndex);
        }

        SDL_LockMutex(workerPool.mutex);

        workerPool.completed++;

        if (workerPool.completed == workerPool.numThreads)
        {
            SDL_CondSignal(workerPool.doneCond);
        }

        SDL_UnlockMutex(workerPool.mutex);
    }
}