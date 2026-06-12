#define SDL_MAIN_HANDLED
#include "SDL2/SDL.h"
#include "core/common.h"
#include "render/draw.h"
#include "platform/init.h"
#include "platform/input.h"
#include "main.h"
#include "core/stats.h"
#include "core/workerpool.h"

App    app;
Stats stats;

int main(int argc, char* argv[])
{
	memset(&app, 0, sizeof(App));
	srand(RNG_SEED);
	initSDL();

	atexit(printStats);

	Uint64 lastCounter = SDL_GetPerformanceCounter();

	double fpsTimer = 0.0;
	Uint64 frameCount = 0;

	double statsTimer = 0.0;


	// Statistics
	Stats_Init(&stats);
	Stats titleStats;
	Stats_Init(&titleStats);

	FrameProfiler profiler = { 0 };

	// threads init
	int numThreads = SDL_GetCPUCount();
	int countThreads = 3;
	SDL_Thread** threads = malloc(sizeof(SDL_Thread*) * numThreads);
	char (*threadNames)[32] = malloc(sizeof(*threadNames) * numThreads);

	for (int i = 0; i < numThreads; i++)
	{
		snprintf(threadNames[i], sizeof(threadNames[i]), "BoidWorker %d", i);
	}


	// init workerPool
	workerPool.numThreads = numThreads;
	workerPool.quit = 0;
	workerPool.generation = 0;
	workerPool.completed = 0;
	workerPool.mutex = SDL_CreateMutex();
	workerPool.chunkMutex = SDL_CreateMutex();
	workerPool.startCond = SDL_CreateCond();
	workerPool.doneCond = SDL_CreateCond();

	int* threadIds = malloc(sizeof(int) * numThreads);
	if (threadIds == NULL)
	{
		SDL_Log("Failed to allocate threadIds");
		exit(1);
	}

	for (int i = 0; i < numThreads; i++)
	{
		threadIds[i] = i;
		threads[i] = SDL_CreateThread(
			PersistentWorkerMainBalanced,
			threadNames[i],
			&threadIds[i]
		);
	}
	
	while (1)
	{
		Uint64 frameStart = SDL_GetPerformanceCounter();
		double deltaTime = (double)(frameStart - lastCounter) / SDL_GetPerformanceFrequency();
		real deltaTimeReal = (real)deltaTime;
		lastCounter = frameStart;

		prepareScene();
		doInput();
		double fps = 1.0 / deltaTime;

		Stats_AddSample(&stats, STAT_FPS, fps);
		Stats_AddSample(&titleStats, STAT_FPS, fps);
		Profiler_Begin(&profiler, STAT_UPDATE);
		// UPDATE LOGIC HERE
		Profiler_End(&profiler, &stats, &titleStats, STAT_UPDATE);


		Profiler_Begin(&profiler, STAT_DRAW);
		// DRAW LOGIC HERE
		Profiler_End(&profiler, &stats, &titleStats, STAT_DRAW);
		
		Profiler_End(&profiler, &stats, &titleStats, STAT_FRAME_WORK);

		// Update window title with stats
		Stats_UpdateTitle(&titleStats, app.window, deltaTime);

		presentScene();


		// Frame limiting to 60 FPS
		Uint64 frameEnd = SDL_GetPerformanceCounter();
		Uint64 performanceFreq = SDL_GetPerformanceFrequency();
		double frameSeconds = (double)(frameEnd - frameStart) / performanceFreq;
		double targetFrameTime = 1.0 / 60.0;
		stats.totalFrames++;
		if (frameSeconds < targetFrameTime)
		{
			SDL_Delay((Uint32)((targetFrameTime - frameSeconds) * MS_PER_SECOND));
		}
	}

	return 0;
}


void printStats(void)
{
	printf("\n--- Simulation Stats ---\n");

#ifdef _DEBUG
	printf("Build: Debug\n");
#else
	printf("Build: Release\n");
#endif

	printf("Real type: %s\n", REAL_TYPE_NAME);
	printf("Seed: %d\n", RNG_SEED);
	printf("Benchmark Frames: %d\n", BENCHMARK_FRAMES);
	printf("Warm-up Frames: %d\n", WARMUP_FRAMES);
	printf("Runtime: %.2f sec\n", stats.runTime);

	for (int i = 0; i < STAT_COUNT; i++)
	{
		StatMetric* metric = &stats.metrics[i];

		if (metric->samples == 0 || metric->name == NULL)
		{
			continue;
		}

		printf("%s min/avg/max: %.4f / %.4f / %.4f\n",
			metric->name,
			metric->min,
			metric->sum / metric->samples,
			metric->max);
	}

	printf("\n------------------------\n");

	cleanup();
}

