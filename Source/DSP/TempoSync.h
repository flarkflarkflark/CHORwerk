#pragma once
#include <juce_audio_processors/juce_audio_processors.h>

/**
 * Converts time unit + value to Hz, matching the original's extensive tempo sync options.
 * Units: ms, 10ms, sec, 32nds, 16ths, 8ths, quarters,
 *        plus triplet (2/3) and quintuplet (4/5) variants.
 */
namespace TempoSync
{
    enum class Unit
    {
        Milliseconds = 0,   // Direct ms
        TenMs,              // × 10ms
        Seconds,            // Direct seconds
        ThirtySeconds,      // 1/32 note
        Sixteenths,         // 1/16 note
        Eighths,            // 1/8 note
        Quarters,           // 1/4 note
        TripletThirtySeconds,  // 2/3 of 1/32
        TripletSixteenths,     // 2/3 of 1/16
        TripletEighths,        // 2/3 of 1/8
        TripletQuarters,       // 2/3 of 1/4
        QuintupletThirtySeconds, // 4/5 of 1/32
        QuintupletSixteenths,    // 4/5 of 1/16
        QuintupletEighths,       // 4/5 of 1/8
        QuintupletQuarters       // 4/5 of 1/4
    };

    /**
     * Convert a unit+value pair to a period in milliseconds.
     * For musical units, bpm is required.
     * @param unit  The time unit
     * @param value The parameter value (in those units)
     * @param bpm   Current tempo (only used for musical units)
     * @return Period in milliseconds
     */
    inline float toPeriodMs (Unit unit, float value, float bpm = 120.0f)
    {
        // Base quarter note duration
        float quarterMs = 60000.0f / std::max (bpm, 20.0f);

        switch (unit)
        {
            case Unit::Milliseconds:           return value;
            case Unit::TenMs:                  return value * 10.0f;
            case Unit::Seconds:                return value * 1000.0f;

            case Unit::ThirtySeconds:          return value * quarterMs * 0.125f;
            case Unit::Sixteenths:             return value * quarterMs * 0.25f;
            case Unit::Eighths:                return value * quarterMs * 0.5f;
            case Unit::Quarters:               return value * quarterMs;

            case Unit::TripletThirtySeconds:   return value * quarterMs * 0.125f * (2.0f / 3.0f);
            case Unit::TripletSixteenths:      return value * quarterMs * 0.25f  * (2.0f / 3.0f);
            case Unit::TripletEighths:         return value * quarterMs * 0.5f   * (2.0f / 3.0f);
            case Unit::TripletQuarters:        return value * quarterMs          * (2.0f / 3.0f);

            case Unit::QuintupletThirtySeconds: return value * quarterMs * 0.125f * (4.0f / 5.0f);
            case Unit::QuintupletSixteenths:    return value * quarterMs * 0.25f  * (4.0f / 5.0f);
            case Unit::QuintupletEighths:       return value * quarterMs * 0.5f   * (4.0f / 5.0f);
            case Unit::QuintupletQuarters:      return value * quarterMs          * (4.0f / 5.0f);

            default: return value;
        }
    }

    /** Convert period in ms to frequency in Hz. */
    inline float periodMsToHz (float periodMs)
    {
        return (periodMs > 0.001f) ? (1000.0f / periodMs) : 1000.0f;
    }

    /** Convenience: directly get Hz from unit + value + bpm. */
    inline float toHz (Unit unit, float value, float bpm = 120.0f)
    {
        return periodMsToHz (toPeriodMs (unit, value, bpm));
    }

    /** Quantize a phase value to the nearest grid position (if quantize enabled). */
    inline float quantizePhase (float phase, float gridSize)
    {
        if (gridSize <= 0.0f) return phase;
        return std::round (phase / gridSize) * gridSize;
    }
}
