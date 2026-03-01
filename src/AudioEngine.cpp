#define MINIAUDIO_IMPLEMENTATION
#include "AudioEngine.h"
#include <iostream>
#include <cmath>
#include <algorithm>

AudioEngine::AudioEngine()
    : m_initialized(false), m_bass(0.0f), m_mid(0.0f), m_treble(0.0f) {
    m_fftConfig = kiss_fftr_alloc(FFT_SIZE, 0, nullptr, nullptr);
    m_audioBuffer.resize(FFT_SIZE, 0.0f);
    m_fftOutput.resize(FFT_SIZE / 2 + 1);
}

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
    float bassSum = 0.0f, midSum = 0.0f, trebleSum = 0.0f;
    int bassCount = 0, midCount = 0, trebleCount = 0;

    int numBins = FFT_SIZE / 2;
    for (int i = 1; i < numBins; ++i) { // Skip DC offset (i=0)
        float magnitude = std::sqrt(m_fftOutput[i].r * m_fftOutput[i].r + m_fftOutput[i].i * m_fftOutput[i].i);
        
        // Very basic band distribution based on bin index
        if (i < 10) { // Bass (low frequency bins)
            bassSum += magnitude;
            bassCount++;
        } else if (i < 100) { // Mids
            midSum += magnitude;
            midCount++;
        } else { // Treble
            trebleSum += magnitude;
            trebleCount++;
        }
    }

    // A simple lerp for smooth visual transitions
    auto smooth = [](float current, float target, float dt) {
        return current + (target - current) * dt;
    };
    
    float dt = 0.1f; // Lerp factor
    m_bass = smooth(m_bass, (bassCount > 0) ? (bassSum / bassCount) : 0.0f, dt);
    m_mid = smooth(m_mid, (midCount > 0) ? (midSum / midCount) : 0.0f, dt);
    m_treble = smooth(m_treble, (trebleCount > 0) ? (trebleSum / trebleCount) : 0.0f, dt);
}
