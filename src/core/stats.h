// stats.h
#pragma once

typedef struct
{
    double runTime;
    double minFps;
    double maxFps;
    double fpsSum;
    int fpsSamples;

    double minUpdateMs;
    double maxUpdateMs;
    double updateMsSum;
    int updateSamples;

    double minDrawMs;
    double maxDrawMs;
    double drawMsSum;
    int drawSamples;

    int totalFrames;

    double minFrameWorkMs;
    double maxFrameWorkMs;
    double frameWorkMsSum;
    int frameWorkSamples;
} Stats;

extern Stats stats;