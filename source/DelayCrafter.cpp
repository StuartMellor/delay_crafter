#include "DelayCrafter.h"

DelayCrafter::~DelayCrafter()
{
}

void DelayCrafter::setDelayTime(float delayTimeMs) {
    m_delayTimeMs = delayTimeMs;
}

void DelayCrafter::setFeedback(float feedbackAmount) {
    m_feedbackAmount = feedbackAmount;
}

void DelayCrafter::setDryWetMix(float dryWetRatio) {
    m_dryWetRatio = dryWetRatio;
}

float DelayCrafter::getDelayTime() const {
    return m_delayTimeMs;
}

float DelayCrafter::getFeedback() const { 
    return m_feedbackAmount;
}

float DelayCrafter::getDryWetMix() const {
    return m_dryWetRatio;
}

void DelayCrafter::processAudio(float* inputBuffer, float* outputBuffer, int numSamples, int channel) {
    auto* delayBufferData = m_delayBuffers[static_cast<size_t>(channel)].getWritePointer(0);
    int& writePos = m_writePositions[static_cast<size_t>(channel)];
    int& readPos = m_readPositions[static_cast<size_t>(channel)];

    for(int i = 0; i < numSamples; ++i) {
        // read into output buffer
        outputBuffer[i] = delayBufferData[readPos];
        ++readPos;
        
        // write into the buffer
        if(writePos >= (m_delayTimeMs / 1000.0f) * m_sampleRate) {
            writePos = 0;
        }
        delayBufferData[writePos] = inputBuffer[i];
        ++writePos;
    }
}
