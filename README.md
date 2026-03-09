# CHORwerk v1.0.0

**CHORwerk** is a professional multi-voice ensemble and chorus effect, inspired by the legendary "Buzz" modulation architecture. It provides lush, jittery, and wide stereo textures suitable for everything from subtle doubling to massive ambient pads.

## Features

### Legendary "Buzz" Modulation
- **Linear Velocity Wanderer:** Unlike standard sine-based LFOs, CHORwerk's voices move at randomized speeds and "bounce" at delay boundaries for a more organic, complex ensemble sound.
- **Up to 8 Voices:** Individually randomized and panned for maximum width.
- **Collision Detection:** Re-randomizes voice timing when delays get too close to prevent phase cancellation.

### Professional Signal Path
- **Stereo Feedback Rotation:** Feedback is interpreted as a 2D vector and rotated by a user-definable angle, creating complex cross-channel feedback loops.
- **Dual Filter Topology:** 3-iteration oversampled State Variable Filters (LP/HP) with independent resonance control.
- **Dynamic Envelope Follower:** Modulate filters and delay times based on input level. "Follow Level" ties the output gain to the input envelope for expressive control.

### Advanced Modern UI
- **Fluid & Scalable:** Supports high-resolution displays with scaling from 25% to 400%.
- **Precise A/B Comparison:** Two independent buffers to compare and fine-tune your settings.
- **Tempo Sync:** Fully syncable LFO, Step Sequencer, and Rate Updates with project BPM.
- **Dynamic Labels:** Unit-aware labels show exact musical values (e.g., "1 8ths") when sync is active.

## Installation

### Windows
- **VST3:** Copy the `.vst3` folder to `C:\Program Files\Common Files\VST3`
- **CLAP:** Copy the `.clap` file to `C:\Program Files\Common Files\CLAP`

### macOS
- **Audio Unit:** Copy the `.component` folder to `/Library/Audio/Plug-Ins/Components/`
- **VST3:** Copy the `.vst3` folder to `/Library/Audio/Plug-Ins/VST3/`
- **CLAP:** Copy the `.clap` file to `/Library/Audio/Plug-Ins/CLAP/`

### Linux
- **VST3:** Copy to `~/.vst3/`
- **CLAP:** Copy to `~/.clap/`

## Requirements
- **Host:** Any DAW supporting VST3, CLAP, or AU (macOS only).
- **Architecture:** 64-bit (x86_64).

---
© 2026 flarkAUDIO. All rights reserved.
