#pragma once

#include <string>
#include <vector>
#include <mutex>
#include <miniaudio.h>
#include <kiss_fftr.h>

class AudioEngine {
public:
    AudioEngine();
    ~AudioEngine();

    bool initialize(const std::string& filePath);
    void play();
    void update();

    // Get smoothed frequency band values (0.0 to 1.0 roughly)
    float getBass() const { return m_bass; }
    float getMid() const { return m_mid; }
    float getTreble() const { return m_treble; }

private:
    static void dataCallback(ma_device* pDevice, void* pOutput, const void* pInput, ma_uint32 frameCount);
    void processAudioData(const float* pFrames, ma_uint32 frameCount);

    ma_decoder m_decoder;
    ma_device m_device;
    bool m_initialized;

    // FFT Setup
    static const int FFT_SIZE = 1024;
    kiss_fftr_cfg m_fftConfig;
    std::vector<float> m_audioBuffer;
    std::vector<kiss_fft_cpx> m_fftOutput;
    
    // Concurrency between audio thread and main thread
    std::mutex m_audioMutex;

    float m_bass;
    float m_mid;
    float m_treble;
};
