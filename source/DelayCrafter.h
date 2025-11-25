#include "juce_audio_basics/juce_audio_basics.h"
#include <vector>

class DelayCrafter
{
public:
    DelayCrafter (int m_sampleRate, int m_channels)
        : m_delayTimeMs (500.0f), m_feedbackAmount (0.5f), m_dryWetRatio (0.5f), m_channels (m_channels), m_sampleRate (m_sampleRate)
    {
        int delayBufferSize = static_cast<int> (m_sampleRate * (MAX_DELAY_TIME_MS / 1000.0f));

        m_delayBuffers.resize (static_cast<size_t> (m_channels));
        m_writePositions.resize (static_cast<size_t> (m_channels), 0);

        for (int ch = 0; ch < m_channels; ++ch)
        {
            m_delayBuffers[static_cast<size_t> (ch)].setSize (1, delayBufferSize);
            m_delayBuffers[static_cast<size_t> (ch)].clear();
        }
    }
    ~DelayCrafter();

    void processAudio (float* inputBuffer, float* outputBuffer, int numSamples, int channel);
    void setDelayTime (float delayTimeMs);
    void setFeedback (float feedbackAmount);
    void setDryWetMix (float dryWetRatio);

    float getDelayTime() const;
    float getFeedback() const;
    float getDryWetMix() const;
    const float MAX_DELAY_TIME_MS = 2000.0f;

private:
    float m_delayTimeMs;
    float m_feedbackAmount;
    float m_dryWetRatio;
    int m_channels;
    int m_sampleRate;

    std::vector<juce::AudioBuffer<float>> m_delayBuffers;
    std::vector<int> m_writePositions;
};