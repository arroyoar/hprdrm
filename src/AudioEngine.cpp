#define MINIAUDIO_IMPLEMENTATION
#include "AudioEngine.h"
#include <iostream>
#include <cmath>
#include <algorithm>

AudioEngine::AudioEngine()
    : m_initialized(false), 
      m_subBass(0.0f), m_bass(0.0f), m_lowMid(0.0f), m_mid(0.0f), m_highMid(0.0f), m_presence(0.0f), m_treble(0.0f),
      m_subBassEnergy(0.0f), m_bassEnergy(0.0f), m_lowMidEnergy(0.0f), m_midEnergy(0.0f), m_highMidEnergy(0.0f), m_presenceEnergy(0.0f), m_trebleEnergy(0.0f),
      m_subBassMaxDuration(0.0f), m_bassMaxDuration(0.0f), m_lowMidMaxDuration(0.0f), m_midMaxDuration(0.0f), m_highMidMaxDuration(0.0f), m_presenceMaxDuration(0.0f), m_trebleMaxDuration(0.0f) {
    m_fftConfig = kiss_fftr_alloc(FFT_SIZE, 0, nullptr, nullptr);
    m_audioBuffer.resize(FFT_SIZE, 0.0f);
    m_fftOutput.resize(FFT_SIZE / 2 + 1);
}

// ... skipping unchanged destructor and init code ...

AudioEngine::~AudioEngine() {
    if (m_initialized) {
        ma_device_uninit(&m_device);
        ma_decoder_uninit(&m_decoder);
    }
    kiss_fftr_free(m_fftConfig);
}

bool AudioEngine::initialize(const std::string& filePath) {
    ma_result result;

    result = ma_decoder_init_file(filePath.c_str(), nullptr, &m_decoder);
    if (result != MA_SUCCESS) {
        std::cerr << "Failed to load audio file: " << filePath << std::endl;
        return false;
    }

    ma_device_config deviceConfig = ma_device_config_init(ma_device_type_playback);
    deviceConfig.playback.format   = m_decoder.outputFormat;
    deviceConfig.playback.channels = m_decoder.outputChannels;
    deviceConfig.sampleRate        = m_decoder.outputSampleRate;
    deviceConfig.dataCallback      = dataCallback;
    deviceConfig.pUserData         = this;

    result = ma_device_init(nullptr, &deviceConfig, &m_device);
    if (result != MA_SUCCESS) {
        std::cerr << "Failed to initialize playback device." << std::endl;
        ma_decoder_uninit(&m_decoder);
        return false;
    }

    m_initialized = true;
    return true;
}

void AudioEngine::play() {
    if (m_initialized) {
        ma_device_start(&m_device);
    }
}

void AudioEngine::dataCallback(ma_device* pDevice, void* pOutput, const void* pInput, ma_uint32 frameCount) {
    AudioEngine* engine = static_cast<AudioEngine*>(pDevice->pUserData);
    ma_decoder* pDecoder = &engine->m_decoder;
    
    // Read audio data into the output buffer
    ma_uint64 framesRead = 0;
    ma_decoder_read_pcm_frames(pDecoder, pOutput, frameCount, &framesRead);

    // Provide the data to our FFT processor (using the first channel if stereo)
    if (framesRead > 0 && pDevice->playback.format == ma_format_f32) {
        engine->processAudioData(static_cast<const float*>(pOutput), static_cast<ma_uint32>(framesRead));
    }

    (void)pInput; // Unused
}

void AudioEngine::processAudioData(const float* pFrames, ma_uint32 frameCount) {
    std::lock_guard<std::mutex> lock(m_audioMutex);

    int channels = m_device.playback.channels;
    
    // Shift buffer left and add new samples (very basic windowing)
    int samplesToCopy = std::min(static_cast<int>(frameCount), FFT_SIZE);
    int shift = FFT_SIZE - samplesToCopy;
    
    if (shift > 0) {
        std::memmove(m_audioBuffer.data(), m_audioBuffer.data() + samplesToCopy, shift * sizeof(float));
    }
    
    for (int i = 0; i < samplesToCopy; ++i) {
        m_audioBuffer[shift + i] = pFrames[i * channels]; // Just taking the first channel
    }
}

void AudioEngine::update() {
    if (!m_initialized) return;

    std::vector<float> currentBuffer;
    {
        std::lock_guard<std::mutex> lock(m_audioMutex);
        currentBuffer = m_audioBuffer;
    }

    // Apply Hanning Window to reduce spectral leakage
    for (int i = 0; i < FFT_SIZE; ++i) {
        float multiplier = 0.5f * (1.0f - cos(2.0f * 3.14159265358979323846f * i / (FFT_SIZE - 1)));
        currentBuffer[i] *= multiplier;
    }

    // Execute FFT
    kiss_fftr(m_fftConfig, currentBuffer.data(), m_fftOutput.data());

    // Calculate magnitudes and group into bands
    float subBassSum = 0.0f, bassSum = 0.0f, lowMidSum = 0.0f, midSum = 0.0f, highMidSum = 0.0f, presenceSum = 0.0f, trebleSum = 0.0f;
    int subBassCount = 0, bassCount = 0, lowMidCount = 0, midCount = 0, highMidCount = 0, presenceCount = 0, trebleCount = 0;

    int numBins = FFT_SIZE / 2;
    // We apply a noise threshold so low-volume noise doesn't trigger massive spikes
    // Increased significantly to kill the constant red/orange 'hum' floor
    const float noiseThreshold = 3.0f; 

    for (int i = 1; i < numBins; ++i) { // Skip DC offset (i=0)
        float magnitude = std::sqrt(m_fftOutput[i].r * m_fftOutput[i].r + m_fftOutput[i].i * m_fftOutput[i].i);
        
        // Subtract noise floor and clamp to 0
        magnitude = std::max(0.0f, magnitude - noiseThreshold);

        // 7-Band distribution based on bin index
        if (i < 4) { // Sub-Bass
            subBassSum += magnitude;
            subBassCount++;
        } else if (i < 12) { // Bass
            bassSum += magnitude;
            bassCount++;
        } else if (i < 24) { // Low Mids
            lowMidSum += magnitude;
            lowMidCount++;
        } else if (i < 48) { // Mids
            midSum += magnitude;
            midCount++;
        } else if (i < 96) { // High Mids
            highMidSum += magnitude;
            highMidCount++;
        } else if (i < 150) { // Presence
            presenceSum += magnitude;
            presenceCount++;
        } else { // Treble
            trebleSum += magnitude;
            trebleCount++;
        }
    }

    // Asymmetric smoothing: Fast attack (rises quickly to the beat) and slow release (falls slowly)
    auto smooth = [](float current, float target, float attackDt, float releaseDt) {
        if (target > current) {
            return current + (target - current) * attackDt;
        } else {
            return current + (target - current) * releaseDt;
        }
    };
    
    // The "base" peak value moves up instantly when the beat hits, but falls somewhat quickly
    float attackDt = 0.5f;   
    float releaseDt = 0.05f; 
    
    float targetSubBass = (subBassCount > 0) ? (subBassSum / subBassCount) : 0.0f;
    float targetBass = (bassCount > 0) ? (bassSum / bassCount) : 0.0f;
    float targetLowMid = (lowMidCount > 0) ? (lowMidSum / lowMidCount) : 0.0f;
    float targetMid = (midCount > 0) ? (midSum / midCount) : 0.0f;
    float targetHighMid = (highMidCount > 0) ? (highMidSum / highMidCount) : 0.0f;
    float targetPresence = (presenceCount > 0) ? (presenceSum / presenceCount) : 0.0f;
    float targetTreble = (trebleCount > 0) ? (trebleSum / trebleCount) : 0.0f;

    m_subBass = smooth(m_subBass, targetSubBass, attackDt, releaseDt);
    m_bass = smooth(m_bass, targetBass, attackDt, releaseDt);
    m_lowMid = smooth(m_lowMid, targetLowMid, attackDt, releaseDt);
    m_mid = smooth(m_mid, targetMid, attackDt, releaseDt);
    m_highMid = smooth(m_highMid, targetHighMid, attackDt, releaseDt);
    m_presence = smooth(m_presence, targetPresence, attackDt, releaseDt);
    m_treble = smooth(m_treble, targetTreble, attackDt, releaseDt);

    // Energy Integrator: 
    // We add the current smoothed value to the "Energy" pool.
    // If a note is sustained, it piles up and grows taller over time.
    // We also constantly bleed off energy (decay) so it falls back down slowly.
    
    auto integrateEnergy = [](float energy, float input, float maxEnergy, float& maxDurationTracker) {
        // Only build up energy if the input is significantly above a tiny threshold
        if (input > 0.1f) {
            energy += input * 0.08f; // Greatly reduced from 0.15f to prevent instant maxing
        }
        
        // Decay/bleed energy back down. 
        // We subtract a constant amount AND multiply to ensure it reaches zero quickly when quiet
        energy = (energy * 0.90f) - 0.1f; // Reduced multiplier and added linear falloff
        
        // Strict clamp
        energy = std::max(0.0f, std::min(energy, maxEnergy)); 
        
        // Track how long we are pinned at the ceiling
        if (energy >= maxEnergy - 0.5f) {
            maxDurationTracker += 1.0f; // Accumulate frames spent at max
        } else {
            maxDurationTracker = std::max(0.0f, maxDurationTracker - 2.0f); // Bleed off duration rapidly when we fall
        }

        return energy;
    };

    // The maximum "Energy" limit before we forcibly cap it to avoid overflow
    float maxE = 15.0f; // Lowered from 20.0f
    
    m_subBassEnergy = integrateEnergy(m_subBassEnergy, m_subBass, maxE, m_subBassMaxDuration);
    m_bassEnergy = integrateEnergy(m_bassEnergy, m_bass, maxE, m_bassMaxDuration);
    m_lowMidEnergy = integrateEnergy(m_lowMidEnergy, m_lowMid, maxE, m_lowMidMaxDuration);
    m_midEnergy = integrateEnergy(m_midEnergy, m_mid, maxE, m_midMaxDuration);
    m_highMidEnergy = integrateEnergy(m_highMidEnergy, m_highMid, maxE, m_highMidMaxDuration);
    m_presenceEnergy = integrateEnergy(m_presenceEnergy, m_presence, maxE, m_presenceMaxDuration);
    m_trebleEnergy = integrateEnergy(m_trebleEnergy, m_treble, maxE, m_trebleMaxDuration);
}
