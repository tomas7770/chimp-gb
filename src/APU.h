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
    uint8_t NR30 = 0x7F;

    uint8_t NRx1[4];
    uint8_t NRx2[4] = {0xF3, 0x00, 0x9F, 0x00};
    uint8_t NRx3[4] = {0xFF, 0xFF, 0xFF, 0x00};
    uint8_t NRx4[4];

    static constexpr int waveRamSize = 16;
    uint8_t waveRam[waveRamSize];

private:
    float getAudioSample() const;
    void decrementLengthCounters();
    void decrementVolumeEnvelopes();
    void reloadFrequencyTimer(int channel);
    void triggerChannel(int channel);

    int mFrameSequencerStep = 0;
    int mFrameSequencerTimer = FRAME_SEQUENCER_PERIOD;

    // TODO master controls and DAC on/off

    int mChannelLengthCounter[4];
    bool mChannelEnabled[4] = {false, false, false, false};
    int mChannelFrequencyTimer[4];
    // Wave channel (i == 2) does not have these, but dummy values exist in the arrays for simplicity's sake
    int mChannelVolume[4];
    int mChannelEnvDir[4];
    int mChannelEnvPace[4];
    int mChannelEnvCounter[4];

    int mSquareWaveCounter[2] = {0, 0};
    // TODO square 1 frequency sweep

    int mWaveSampleBuffer = 0;
    int mWavePositionCounter;

    int mLFSR;

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
    static constexpr uint8_t TRIGGER_BITMASK = (1 << 7);
    static constexpr uint8_t WAVE_CHANNEL_VOLUME_BIT = 5;
    static constexpr uint8_t NOISE_CLOCK_DIVIDER_BITMASK = 0b111;
    static constexpr uint8_t NOISE_CLOCK_SHIFT_BIT = 4;
    static constexpr uint8_t NOISE_LFSR_WIDTH_BITMASK = (1 << 3);

    static constexpr int SQUARE_DUTY_WAVES[4][8] = {
        {0, 0, 0, 0, 0, 0, 0, 1},
        {1, 0, 0, 0, 0, 0, 0, 1},
        {1, 0, 0, 0, 0, 1, 1, 1},
        {0, 1, 1, 1, 1, 1, 1, 0}};

    static constexpr int NOISE_FREQ_TIMER_VALUES[] = {8, 16, 32, 48, 64, 80, 96, 112};
};
