#include "DelayCrafter.h"

DelayCrafter::~DelayCrafter()
{
}

void DelayCrafter::setDelayTime (float delayTimeMs)
{
    m_delayTimeMs = delayTimeMs;
}

void DelayCrafter::setFeedback (float feedbackAmount)
{
    m_feedbackAmount = feedbackAmount;
}

void DelayCrafter::setDryWetMix (float dryWetRatio)
{
    m_dryWetRatio = dryWetRatio;
}

float DelayCrafter::getDelayTime() const
{
    return m_delayTimeMs;
}

float DelayCrafter::getFeedback() const
{
    return m_feedbackAmount;
}

float DelayCrafter::getDryWetMix() const
{
    return m_dryWetRatio;
}

void DelayCrafter::processAudio (float* inputBuffer, float* outputBuffer, int numSamples, int channel)
{
    auto* delayBufferData = m_delayBuffers[static_cast<size_t> (channel)].getWritePointer (0);
    int delayBufferSize = m_delayBuffers[static_cast<size_t> (channel)].getNumSamples();
    int& writePos = m_writePositions[static_cast<size_t> (channel)];

    int delaySamples = static_cast<int> ((m_delayTimeMs / 1000.0f) * m_sampleRate);
    delaySamples = juce::jlimit (1, delayBufferSize - 1, delaySamples);

    for (int i = 0; i < numSamples; ++i)
    {
        // Calculate read position
        int readPos = writePos - delaySamples;
        if (readPos < 0)
            readPos += delayBufferSize;

        // Read delayed signal
        float delayedSample = delayBufferData[readPos];

        // Write to delay buffer (input + feedback)
        float inputSample = inputBuffer[i];
        delayBufferData[writePos] = inputSample + (delayedSample * m_feedbackAmount);

        // Mix dry and wet signals
        outputBuffer[i] = inputSample * (1.0f - m_dryWetRatio) + delayedSample * m_dryWetRatio;

        // Advance write position
        ++writePos;
        if (writePos >= delayBufferSize)
            writePos = 0;
    }
}
