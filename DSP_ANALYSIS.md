# Charsiesis v2 — DSP Architecture Analysis

## Reverse-Engineered from Charsiesis.dll (2006, Jakob Katz)

### Overview
Charsiesis is a **multi-voice chorus/ensemble** effect with per-voice randomized
delay modulation, state-variable filters, envelope following, step sequencer
modulation, and LFO modulation. 32-bit VST2, single export `main()`.

---

## 1. Parameters (46 total)

### Voice Engine
| ID              | Display                    | Range/Notes                    |
|-----------------|----------------------------|--------------------------------|
| Voices          | Voices: %i                 | Integer count                  |
| MinRate         | Minimum rate: %.2f ms/s    | Rate of delay modulation       |
| RateRange       | Rate range: %.2f ms/s      | Random spread of rates         |
| RateUpdate      | Rate update: every X units | How often voices re-randomize  |
| MinDelay        | Minimum delay: %.2f ms     | Base delay offset              |
| DelayRange      | Delay range: %.2f ms       | Random spread of delays        |
| StereoMode      | free/slave/antislave/half  | Stereo voice distribution      |

### Mix & Feedback
| ID              | Display                    | Range/Notes                    |
|-----------------|----------------------------|--------------------------------|
| mixDryWet       | Mix: %.2f %%               | Dry/wet blend                  |
| Feedback        | Feedback: %.3f %%          | Delay feedback amount          |
| Rotation        | Rotation: %.3f pi          | Stereo rotation (radians/π)    |
| Pregain         | Pregain: %.2f dB           | Input gain                     |
| Postgain        | Postgain: %.2f dB          | Output gain                    |

### Filters (State Variable)
| ID              | Display                    | Range/Notes                    |
|-----------------|----------------------------|--------------------------------|
| Lowpass         | Lowpass: %.3f %%           | LP cutoff (normalized)         |
| LPRes           | LP resonance: %.3f %%      | LP resonance/Q                 |
| Highpass        | Highpass: %.3f %%          | HP cutoff (normalized)         |
| HPRes           | HP resonance: %.3f %%      | HP resonance/Q                 |

### Envelope Follower
| ID              | Display                       | Range/Notes                 |
|-----------------|-------------------------------|-----------------------------|
| FollowLevel     | no / slow / normal / fast     | 4 modes of envelope decay   |
| EnvLP           | Level → LP cutoff: %.3f oct   | Env mod depth to LP         |
| EnvHP           | Level → HP cutoff: %.3f oct   | Env mod depth to HP         |
| EnvDelay        | Level → delay: %.2f ms        | Env mod depth to delay      |

### LFO
| ID              | Display                    | Range/Notes                    |
|-----------------|----------------------------|--------------------------------|
| LFOShape        | sine/ramp/triangle/square/stepping random/smooth random/user | 7 shapes |
| LFORate         | LFO rate: X units           | Rate with tempo sync          |
| LFOUnit         | ms/10ms/sec/32nds/16ths/8ths/quarters + triplet/quintuplet variants |
| LFOQuant        | Quantize LFO period: yes/no | Snap to grid                  |
| LFOLP           | LFO → LP cutoff: %.3f oct   | LFO mod depth to LP          |
| LFOHP           | LFO → HP cutoff: %.3f oct   | LFO mod depth to HP          |
| LFODelay        | LFO → delay: %.2f ms        | LFO mod depth to delay       |

### Step Sequencer (8 steps)
| ID              | Display                    | Range/Notes                    |
|-----------------|----------------------------|--------------------------------|
| Seq0–Seq7       | Seq #%i: %.2f %%           | 8 step values (0-100%)        |
| SeqStep         | Seq step: X units           | Step rate with tempo sync     |
| SeqUnit         | Same unit options as LFO    | Tempo-synced units            |
| SeqQuant        | Quantize seq step: yes/no   | Snap to grid                  |
| SeqLP           | Seq → LP cutoff: %.3f oct   | Seq mod depth to LP          |
| SeqHP           | Seq → HP cutoff: %.3f oct   | Seq mod depth to HP          |
| SeqDelay        | Seq → delay: %.2f ms        | Seq mod depth to delay       |

### Update Control
| ID                | Display                    | Range/Notes                  |
|-------------------|----------------------------|------------------------------|
| UpdateUnit        | Same unit options as LFO   | Rate update timing           |
| UpdateQuantize    | Quantize: yes/no           | Snap update to grid          |
| UpdateOnCollision | On collision: yes/no       | Re-trigger on voice collision|

---

## 2. DSP Architecture

### Signal Flow
```
Input → Pregain → [Per-Voice Processing] → Sum → Postgain → Mix(Dry,Wet) → Output

Per-Voice:
  Voice → Delay Line (modulated) → SVF (LP+HP with resonance) → Feedback → Gain
                                                                    ↑
  Modulation Sources: Random Rate + LFO + Sequencer + Envelope ────┘
```

### Voice Engine
- Variable voice count (likely 1-8 based on gain distribution refs)
- Each voice has independent random delay modulation
- Rate of modulation: MinRate ± RateRange (ms/s)
- Delay time: MinDelay ± DelayRange (ms)
- Voices periodically re-randomize their parameters (RateUpdate)
- "Collision" detection: when two voices' delays are close, optional re-trigger
- Gain distribution curve applied across voices (editable via skin)

### Stereo Modes
- **Free**: Each channel's voices modulate independently
- **Slave**: Right channel copies left channel voice parameters
- **Antislave**: Right channel mirrors/inverts left channel parameters
- **Half**: Right channel uses offset (half-cycle) of left parameters

### Delay Line
- Base delay ~20ms max (constant found at 0x244cc)
- Linear or cubic interpolation (FPU pattern suggests at least linear)
- Modulated by: voice random rate + LFO + sequencer + envelope
- Feedback path with rotation (stereo cross-feedback at angle π*Rotation)

### State Variable Filter
- Classic Hal Chamberlin SVF topology (confirmed by FPU instruction pattern)
- Cascaded stages: the triple fmul/fsub/fadd pattern repeats 3× at 0x4EC3-0x4F41
- This is a **2-pole SVF with oversampled coefficient calculation**
- Both LP and HP outputs used simultaneously
- Resonance control on both LP and HP independently
- Filter cutoff modulated by: LFO, Sequencer, Envelope

### Envelope Follower
- Input level tracking with 3 decay rates: slow, normal, fast
- Modulates: LP cutoff (octaves), HP cutoff (octaves), delay time (ms)
- Bipolar modulation depth

### LFO
- 7 waveforms: sine, ramp (saw), triangle, square, stepping random, smooth random, user-defined
- User LFO: 16-point waveform (confirmed by "%.2f/16" format string)
- Tempo-syncable with extensive subdivision options (including triplets and quintuplets)
- Optional period quantization
- Modulates: LP cutoff, HP cutoff, delay time

### Step Sequencer
- 8 steps (Seq0-Seq7 parameter IDs confirmed)
- Tempo-syncable step rate
- Optional step quantization
- Modulates: LP cutoff, HP cutoff, delay time

---

## 3. Key Constants Found

| Offset   | Value        | Likely Purpose                    |
|----------|-------------|-----------------------------------|
| 0x1188   | 44100.0     | Default sample rate               |
| 0x244CC  | 20.0        | Max base delay (ms)               |
| 0x24454  | π (3.14159) | Filter/rotation calculations      |
| 0x24568  | 100.0       | Percentage scaling                |
| 0x24908  | 5.0         | Rate range default?               |
| 0x24910  | 10.0        | Delay range default?              |
| 0x25158  | √2 (1.414)  | Butterworth Q coefficient         |

---

## 4. Memory Layout (Per-Voice State at ESI+0x300xxx)

~25 floats per voice state covering:
- Delay line position and interpolation state
- SVF state (bandpass, lowpass, highpass accumulators)
- LFO phase accumulator
- Sequencer position
- Envelope follower state
- Current rate and delay targets
- Feedback accumulator
