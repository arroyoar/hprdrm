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

    // Get the accumulated "energy" for each band
    float getSubBass() const { return m_subBassEnergy; }
    float getBass() const { return m_bassEnergy; }
    float getLowMid() const { return m_lowMidEnergy; }
    float getMid() const { return m_midEnergy; }
    float getHighMid() const { return m_highMidEnergy; }
    float getPresence() const { return m_presenceEnergy; }
    float getTreble() const { return m_trebleEnergy; }

    // Get the duration each band has been held at max energy
    float getSubBassMaxDuration() const { return m_subBassMaxDuration; }
    float getBassMaxDuration() const { return m_bassMaxDuration; }
    float getLowMidMaxDuration() const { return m_lowMidMaxDuration; }
    float getMidMaxDuration() const { return m_midMaxDuration; }
    float getHighMidMaxDuration() const { return m_highMidMaxDuration; }
    float getPresenceMaxDuration() const { return m_presenceMaxDuration; }
    float getTrebleMaxDuration() const { return m_trebleMaxDuration; }

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

    // Instantaneous, smoothed peak values
    float m_subBass;
    float m_bass;
    float m_lowMid;
    float m_mid;
    float m_highMid;
    float m_presence;
    float m_treble;
    
    // "Energy" integrators that build up over time when a frequency is sustained
    float m_subBassEnergy;
    float m_bassEnergy;
    float m_lowMidEnergy;
    float m_midEnergy;
    float m_highMidEnergy;
    float m_presenceEnergy;
    float m_trebleEnergy;

    // Track how long (in frames/ticks) a band has been held at maximum energy
    float m_subBassMaxDuration;
    float m_bassMaxDuration;
    float m_lowMidMaxDuration;
    float m_midMaxDuration;
    float m_highMidMaxDuration;
    float m_presenceMaxDuration;
    float m_trebleMaxDuration;
};
