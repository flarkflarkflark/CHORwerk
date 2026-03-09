#pragma once
#include <vector>
#include <cmath>
#include <algorithm>

/**
 * Fractional delay line with cubic interpolation.
 */
class DelayLine
{
public:
    void prepare (double sampleRate, float maxDelayMs)
    {
        sr = sampleRate;
        maxDelaySamples = static_cast<int> (maxDelayMs * 0.001 * sr) + 4;
        buffer.assign (static_cast<size_t> (maxDelaySamples + 4), 0.0f);
        writePos = 0;
    }

    void reset()
    {
        std::fill (buffer.begin(), buffer.end(), 0.0f);
        writePos = 0;
    }

    void pushSample (float sample)
    {
        buffer[static_cast<size_t> (writePos)] = sample;
        if (++writePos >= static_cast<int> (buffer.size()))
            writePos = 0;
    }

    float readSample (float delayMs) const
    {
        float delaySamples = static_cast<float> (delayMs * 0.001 * sr);
        delaySamples = std::clamp (delaySamples, 0.0f, static_cast<float> (maxDelaySamples - 1));

        int idx = static_cast<int> (delaySamples);
        float frac = delaySamples - static_cast<float> (idx);

        float y0 = readRaw (writePos - idx - 2);
        float y1 = readRaw (writePos - idx - 1);
        float y2 = readRaw (writePos - idx);
        float y3 = readRaw (writePos - idx + 1);

        float c0 = y1;
        float c1 = 0.5f * (y2 - y0);
        float c2 = y0 - 2.5f * y1 + 2.0f * y2 - 0.5f * y3;
        float c3 = 0.5f * (y3 - y0) + 1.5f * (y1 - y2);

        return ((c3 * frac + c2) * frac + c1) * frac + c0;
    }

private:
    float readRaw (int pos) const
    {
        const int size = static_cast<int> (buffer.size());
        pos %= size;
        if (pos < 0) pos += size;
        return buffer[static_cast<size_t> (pos)];
    }

    std::vector<float> buffer;
    int writePos = 0;
    int maxDelaySamples = 0;
    double sr = 44100.0;
};
