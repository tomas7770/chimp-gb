#pragma once

#include <cstdint>

class APU
{
public:
    APU();

    void doCycle()
    {
        // 2 MHz cycle (2 T-cycles)
        // Technically the APU runs at 4 MHz, but in practice important events only happen every 2 cycles,
        // so performance can be optimized by emulating 2 cycles at once.

        mFrameSequencerTimer -= 2;
        if (mFrameSequencerTimer == 0)
        {
            mFrameSequencerTimer = FRAME_SEQUENCER_PERIOD;
            if (!mAPUEnabled)
            {
                return;
            }
            switch (mFrameSequencerStep)
            {
            case 0:
                decrementLengthCounters();
                break;

            case 2:
                decrementLengthCounters();
                clockSweep();
                break;

            case 4:
                decrementLengthCounters();
                break;

            case 6:
                decrementLengthCounters();
                clockSweep();
                break;

            case 7:
                decrementVolumeEnvelopes();
                break;

            default:
                break;
            }
            mFrameSequencerStep = (mFrameSequencerStep + 1) % 8;
        }
        else if (!mAPUEnabled)
        {
            return;
        }

        for (int i = 0; i < 4; i++)
        {
            if (i == 2)
            {
                mDAC[i] = (NR30 >> DAC_BIT_WAVE_CHANNEL) ? true : false;
            }
            else
            {
                mDAC[i] = (NRx2[i] >> DAC_BIT) ? true : false;
            }

            if (!mDAC[i])
            {
                mChannelEnabled[i] = false;
            }

            mChannelFrequencyTimer[i] -= 2;
            if (mChannelFrequencyTimer[i] == 0)
            {
                reloadFrequencyTimer(i);
                int xorValue;
                switch (i)
                {
                case 0:
                case 1:
                    mSquareWaveCounter[i] = (mSquareWaveCounter[i] + 1) % 8;
                    break;

                case 2:
                    mWavePositionCounter = (mWavePositionCounter + 1) % (waveRamSize * 2);
                    if (mWavePositionCounter % 2)
                    {
                        mWaveSampleBuffer = waveRam[mWavePositionCounter / 2] & 0xF;
                    }
                    else
                    {
                        mWaveSampleBuffer = waveRam[mWavePositionCounter / 2] >> 4;
                    }
                    break;

                case 3:
                    xorValue = (mLFSR & 1) ^ ((mLFSR >> 1) & 1);
                    mLFSR >>= 1;
                    mLFSR |= (xorValue << 14);
                    if (NRx3[3] & NOISE_LFSR_WIDTH_BITMASK)
                    {
                        mLFSR &= ~(1 << 6);
                        mLFSR |= (xorValue << 6);
                    }
                    break;

                default:
                    break;
                }
            }
        }
    }

    void computeAudioSamples();

    void writeNRx1(int channel, uint8_t value);
    void writeNRx4(int channel, uint8_t value);

    uint8_t readNR52();
    void writeNR52(uint8_t value);

    uint8_t NR10 = 0x80;
    uint8_t NR30 = 0x7F;

    uint8_t NRx1[4];
    uint8_t NRx2[4] = {0xF3, 0x00, 0x9F, 0x00};
    uint8_t NRx3[4] = {0xFF, 0xFF, 0xFF, 0x00};
    uint8_t NRx4[4];

    uint8_t NR50 = 0x77;
    uint8_t NR51 = 0xF3;

    static constexpr int waveRamSize = 16;
    uint8_t waveRam[waveRamSize];

    float leftAudioSample = 0.0F, rightAudioSample = 0.0F;

    static constexpr float VOL_CODE_HALF_RANGES[] = {0.0F, 7.5F, 3.5F, 1.5F};

private:
    float getAudioSample(bool left) const;
    void decrementLengthCounters();
    void decrementVolumeEnvelopes();
    void clockSweep();
    int calcFrequency(int channel);
    void reloadFrequencyTimer(int channel);
    void sweepFreqCalcAndOverflowCheck(bool writePeriodAndRepeat = false);
    void triggerChannel(int channel);

    int mFrameSequencerStep = 0;
    int mFrameSequencerTimer = FRAME_SEQUENCER_PERIOD;

    bool mAPUEnabled = true;
    bool mDAC[4] = {false, false, false, false};

    int mChannelLengthCounter[4];
    bool mChannelEnabled[4] = {false, false, false, false};
    int mChannelFrequencyTimer[4];
    // Wave channel (i == 2) does not have these, but dummy values exist in the arrays for simplicity's sake
    int mChannelVolume[4];
    int mChannelEnvDir[4];
    int mChannelEnvPace[4];
    int mChannelEnvCounter[4];

    int mSquareWaveCounter[2] = {0, 0};
    bool mSquare1SweepEnabled;
    uint16_t mSquare1FreqSweepShadow;
    int mSquare1SweepTimer;

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
    static constexpr uint8_t SWEEP_STEP_BITMASK = 0b111;
    static constexpr uint8_t SWEEP_PACE_BIT = 4;
    static constexpr uint8_t SWEEP_PACE_BITMASK = 0b111;
    static constexpr uint8_t SWEEP_DIRECTION_BITMASK = (1 << 3);
    static constexpr uint8_t WAVE_CHANNEL_VOLUME_BIT = 5;
    static constexpr uint8_t NOISE_CLOCK_DIVIDER_BITMASK = 0b111;
    static constexpr uint8_t NOISE_CLOCK_SHIFT_BIT = 4;
    static constexpr uint8_t NOISE_LFSR_WIDTH_BITMASK = (1 << 3);
    static constexpr uint8_t DAC_BIT = 3;
    static constexpr uint8_t DAC_BIT_WAVE_CHANNEL = 7;
    static constexpr uint8_t AUDIO_ON_OFF_BITMASK = (1 << 7);
    static constexpr uint8_t VOL_BITMASK = 0b111;
    static constexpr uint8_t LEFT_VOL_BIT = 4;

    static constexpr int SQUARE_DUTY_WAVES[4][8] = {
        {0, 0, 0, 0, 0, 0, 0, 1},
        {1, 0, 0, 0, 0, 0, 0, 1},
        {1, 0, 0, 0, 0, 1, 1, 1},
        {0, 1, 1, 1, 1, 1, 1, 0}};

    static constexpr int NOISE_FREQ_TIMER_VALUES[] = {8, 16, 32, 48, 64, 80, 96, 112};
};
