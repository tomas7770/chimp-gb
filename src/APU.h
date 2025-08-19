#pragma once

#include <cstdint>

class APU
{
public:
    APU();

    void doTCycle();

    float getLeftAudioSample() const;
    float getRightAudioSample() const;

    void writeNRx1(int channel, uint8_t value);
    void writeNRx4(int channel, uint8_t value);

    uint8_t NR10 = 0x80;

    uint8_t NRx1[4];
    uint8_t NRx2[4] = {0xF3, 0x00};
    uint8_t NRx3[4] = {0xFF, 0xFF};
    uint8_t NRx4[4];

private:
    float getAudioSample() const;
    void decrementLengthCounters();
    void decrementVolumeEnvelopes();
    void reloadFrequencyTimer(int channel);
    void triggerChannel(int channel);

    int mFrameSequencerStep = 0;
    int mFrameSequencerTimer = FRAME_SEQUENCER_PERIOD;

    // TODO master controls

    int mSquareWaveCounter[2] = {0, 0};
    int mSquareFrequencyTimer[2];
    int mSquareLengthCounter[2];
    int mSquareVolume[2];
    int mSquareEnvDir[2];
    int mSquareEnvPace[2];
    int mSquareEnvCounter[2];
    bool mSquareEnabled[2] = {false, false};
    // TODO square 1 frequency sweep

    static constexpr int CLOCK_RATE = 4194304;
    static constexpr int FRAME_SEQUENCER_PERIOD = CLOCK_RATE / 512;

    static constexpr int MAX_VOL = 15;

    static constexpr uint8_t INITIAL_LENGTH_TIMER_BITMASK = 0b111111;
    static constexpr uint8_t WAVE_DUTY_BIT = 6;
    static constexpr uint8_t INITIAL_VOLUME_BIT = 4;
    static constexpr uint8_t ENV_DIR_BITMASK = (1 << 3);
    static constexpr uint8_t ENV_PACE_BITMASK = 0b111;
    static constexpr uint8_t PERIOD_HIGH_BITMASK = 0b111;
    static constexpr uint8_t LENGTH_ENABLE_BITMASK = (1 << 6);
    static constexpr uint8_t TRIGGER_BIT = 7;

    static constexpr int SQUARE_DUTY_WAVES[4][8] = {
        {0, 0, 0, 0, 0, 0, 0, 1},
        {1, 0, 0, 0, 0, 0, 0, 1},
        {1, 0, 0, 0, 0, 1, 1, 1},
        {0, 1, 1, 1, 1, 1, 1, 0}};
};
