#pragma once

#include "juce_audio_basics/juce_audio_basics.h"
#include "juce_core/juce_core.h"
#include "Detector.h"
#include "Utility/MidiRingBuffer.h"

class VibratoDetector : public Detector {
public:
    ///
    /// \param initialBufferSize the number of buffers that is stored internally for calculating averages on
    explicit VibratoDetector(int initialBufferSize)
            : sr(48000),spb(256), ringBuffer((float)initialBufferSize, sr, spb) {
    }
    ~VibratoDetector() override = default;

    void processMidi(juce::MidiBuffer &midiMessages, int numSamples) override {
        amplitude.skip(numSamples);
        rate.skip(numSamples);
        juce::MidiBuffer passthrough;
        juce::MidiBuffer vibratoData;

        for (const auto metadata: midiMessages) {
            auto message = metadata.getMessage();
            const auto time = metadata.samplePosition;

            if (message.isControllerOfType(inputController)) {
                vibratoData.addEvent(message, time);
            } else {
                passthrough.addEvent(message, time);
            }
        }

        ringBuffer.push(vibratoData);

        passthrough.addEvent(
                juce::MidiMessage::controllerEvent(1, ampController, getRms()),
                1);

        passthrough.addEvent(
                juce::MidiMessage::controllerEvent(1, rateController, getFrequency()),
                2);

        midiMessages.swapWith(passthrough);
    }

    [[nodiscard]] int getRms() override {
        return ringBuffer.getRms();
    }

    //Should return rate mapped to the correct values...
    [[nodiscard]] int getFrequency() override {
        return ringBuffer.getFrequency(minRate, maxRate);
    }

    [[nodiscard]] float getTargetRate() const {
        return rate.getTargetValue();
    }

    void setInputController(int newCC) override {
        inputController = clampCCs(newCC);
    }

    void setRmsController(int newCC) override {
        ampController = clampCCs(newCC);
    }

    void setFrequencyController(int newCC) override {
        rateController = clampCCs(newCC);
    }

    static int clampCCs(int newCC) { return std::clamp(newCC, 1, 127); }

    void resetValues(double sampleRate, int blockSize) override {
        sr = sampleRate;
        spb = blockSize;
        amplitude.reset(sr, rampLengthInSeconds);
        rate.reset(sr, rampLengthInSeconds);
        amplitude.setCurrentAndTargetValue(0.f);
        rate.setCurrentAndTargetValue(0.f);
        ringBuffer.reset(sr, spb);
    }

    void setAmpScaling(float newScale) override {
        ampScaling = newScale;
    }

    void setNumSecondsToHold(float numSeconds) override {
        ringBuffer.setSecondsToHold(numSeconds);
    }

    /// Sets the new min and max rates for the vibrato. These values should correspond to the min/max
    /// of whatever instrument you are playing, so that the output rateCC correctly maps from 0-127
    void setMinMaxRate(float newMinRate, float newMaxRate) override {
        minRate = newMinRate;
        maxRate = newMaxRate;
    }

private:

    int inputController = 2;
    int ampController = 21;
    int rateController = 20;

    double rampLengthInSeconds = 0.5;

    //These are the min-max Hz rates in Aaron Venture...
    float minRate = 1.68f, maxRate = 7.33f;
    float ampScaling = 1.f;
    double sr = 48000; //Sample rate

    int spb = 256; //Samples per buffer
    Utility::MidiRingBuffer ringBuffer;

    juce::LinearSmoothedValue<float> amplitude, rate;
};