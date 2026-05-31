#define SDL_MAIN_HANDLED
#include "SDL2/SDL.h"
#include "common.h"
#include "draw.h"
#include "init.h"
#include "input.h"
#include "main.h"
#include "stats.h"

App    app;
Stats stats;


void displayFPS(double fps)
{
	char title[TITLE_SIZE];
	snprintf(title, sizeof(title), "FPS: %.2f", fps);

	SDL_SetWindowTitle(app.window, title);
}

void printStats(void)
{
	printf("\n--- Stats ---\n");

	#ifdef _DEBUG
		printf("Build: Debug\n");
	#else
		printf("Build: Release\n");
	#endif

	printf("Real type: %s\n", REAL_TYPE_NAME);

	printf("Seed: %d\n", RNG_SEED);
	printf("Warm-up Frames: %d\n", WARMUP_FRAMES);

	printf("Runtime: %.2f sec\n", stats.runTime);

	printf("FPS min/avg/max: %.2f / %.2f / %.2f\n",
		stats.minFps,
		stats.fpsSum / stats.fpsSamples,
		stats.maxFps);
	
	printf("Update ms min/avg/max: %.4f / %.4f / %.4f\n",
		stats.minUpdateMs,
		stats.updateMsSum / stats.updateSamples,
		stats.maxUpdateMs);

	printf("Draw ms min/avg/max: %.4f / %.4f / %.4f\n",
		stats.minDrawMs,
		stats.drawMsSum / stats.drawSamples,
		stats.maxDrawMs);

	printf("Frame work ms min/avg/max: %.4f / %.4f / %.4f\n",
		stats.minFrameWorkMs,
		stats.frameWorkMsSum / stats.frameWorkSamples,
		stats.maxFrameWorkMs);


	printf("\n------------------------\n");

	cleanup();
}

void UpdateStats(double deltaTime, Uint64 updateStart, Uint64 updateEnd, Uint64 drawEnd, Uint64 frameStart, Uint64 frameEnd, Uint64 performanceFreq) {

	static double titleTimer = 0.0;
	static int titleFrameCount = 0;
	static double titleBuildGridMsSum = 0.0;
	static double titleUpdateMsSum = 0.0;
	static double titleDrawMsSum = 0.0;
	static int titleSamples = 0;

	double updateMs =
		(double)(updateEnd - updateStart) / performanceFreq * MS_PER_SECOND;

	double drawMs =
		(double)(drawEnd - updateEnd) / performanceFreq * MS_PER_SECOND;

	double presentMs =
		(double)(frameEnd - drawEnd) / performanceFreq * MS_PER_SECOND;

	double frameWorkMs =
		(double)(frameEnd - frameStart) / performanceFreq * MS_PER_SECOND;


	stats.runTime += deltaTime;

	if (stats.totalFrames > WARMUP_FRAMES) {

		if (deltaTime > 0.0)
		{
			double fps = 1.0 / deltaTime;

			if (fps < stats.minFps) stats.minFps = fps;
			if (fps > stats.maxFps) stats.maxFps = fps;

			stats.fpsSum += fps;
			stats.fpsSamples++;
		}

		if (updateMs < stats.minUpdateMs) stats.minUpdateMs = updateMs;
		if (updateMs > stats.maxUpdateMs) stats.maxUpdateMs = updateMs;
		stats.updateMsSum += updateMs;
		stats.updateSamples++;

		if (drawMs < stats.minDrawMs) stats.minDrawMs = drawMs;
		if (drawMs > stats.maxDrawMs) stats.maxDrawMs = drawMs;
		stats.drawMsSum += drawMs;
		stats.drawSamples++;

		titleTimer += deltaTime;
		titleFrameCount++;

		titleUpdateMsSum += updateMs;
		titleDrawMsSum += drawMs;
		titleSamples++;

		if (frameWorkMs < stats.minFrameWorkMs)
			stats.minFrameWorkMs = frameWorkMs;

		if (frameWorkMs > stats.maxFrameWorkMs)
			stats.maxFrameWorkMs = frameWorkMs;

		stats.frameWorkMsSum += frameWorkMs;
		stats.frameWorkSamples++;

		if (titleTimer >= 1.0)
		{
			char title[TITLE_SIZE];

			snprintf(
				title,
				sizeof(title),
				"FPS: %.1f | Update: %.3f ms | Draw: %.3f ms | Work: %.3f ms",
				(double)titleFrameCount / titleTimer,
				titleUpdateMsSum / titleSamples,
				titleDrawMsSum / titleSamples,
				stats.frameWorkMsSum / stats.frameWorkSamples
			);

			SDL_SetWindowTitle(app.window, title);

			titleTimer = 0.0;
			titleFrameCount = 0;
			titleUpdateMsSum = 0.0;
			titleDrawMsSum = 0.0;
			titleSamples = 0;
		}
	}

}



int main(int argc, char* argv[])
{
	memset(&app, 0, sizeof(App));
	srand(RNG_SEED);
	initSDL();

	atexit(printStats);

	Uint64 lastCounter = SDL_GetPerformanceCounter();

	double fpsTimer = 0.0;
	int frameCount = 0;

	Uint64 updateStart = 0;
	Uint64 updateEnd = 0;
	Uint64 drawEnd = 0;

	double statsTimer = 0.0;


	// Statistics
	stats.runTime = 0.0;

	stats.minFps = DBL_MAX;
	stats.maxFps = 0.0;
	stats.fpsSum = 0.0;
	stats.fpsSamples = 0;

	stats.minUpdateMs = DBL_MAX;
	stats.maxUpdateMs = 0.0;
	stats.updateMsSum = 0.0;
	stats.updateSamples = 0;

	stats.minDrawMs = DBL_MAX;
	stats.maxDrawMs = 0.0;
	stats.drawMsSum = 0.0;
	stats.drawSamples = 0;
	stats.totalFrames = 0;

	stats.minFrameWorkMs = DBL_MAX;
	stats.maxFrameWorkMs = 0.0;
	stats.frameWorkMsSum = 0.0;
	stats.frameWorkSamples = 0;

	while (1)
	{
		Uint64 frameStart = SDL_GetPerformanceCounter();

		double deltaTime =
			(double)(frameStart - lastCounter) /
			SDL_GetPerformanceFrequency();

		real deltaTimeReal = (real)deltaTime;

		lastCounter = frameStart;

		prepareScene();
		doInput();

		Uint64 updateStart = SDL_GetPerformanceCounter();

		// UPDATE LOGIC HERE
		
		Uint64 updateEnd = SDL_GetPerformanceCounter();

		// DRAW LOGIC HERE
		
		Uint64 drawEnd = SDL_GetPerformanceCounter();

		presentScene();

		Uint64 frameEnd = SDL_GetPerformanceCounter();

		Uint64 performanceFreq = SDL_GetPerformanceFrequency();


		UpdateStats(deltaTime, updateStart, updateEnd, drawEnd, frameStart, frameEnd, performanceFreq);


		double frameSeconds =
			(double)(frameEnd - frameStart) / performanceFreq;

		double targetFrameTime = 1.0 / 60.0;

		stats.totalFrames++;

		if (frameSeconds < targetFrameTime)
		{
			SDL_Delay((Uint32)((targetFrameTime - frameSeconds) * MS_PER_SECOND));
		}
	}

	return 0;
}
