#include "APU.h"

APU::APU()
{
    writeNRx1(0, 0xBF);
    writeNRx4(0, 0xBF);

    writeNRx1(1, 0x3F);
    writeNRx4(1, 0xBF);
}

void APU::doTCycle()
{
    mFrameSequencerTimer--;
    if (mFrameSequencerTimer == 0)
    {
        mFrameSequencerTimer = FRAME_SEQUENCER_PERIOD;
        mFrameSequencerStep = (mFrameSequencerStep + 1) % 8;
        switch (mFrameSequencerStep)
        {
        case 0:
            decrementLengthCounters();
            break;

        case 2:
            decrementLengthCounters();
            // Sweep
            break;

        case 4:
            decrementLengthCounters();
            break;

        case 6:
            decrementLengthCounters();
            // Sweep
            break;

        case 7:
            decrementVolumeEnvelopes();
            break;

        default:
            break;
        }
    }

    for (int i = 0; i < 2; i++)
    {
        mSquareFrequencyTimer[i]--;
        if (mSquareFrequencyTimer[i] == 0)
        {
            reloadFrequencyTimer(i);
            mSquareWaveCounter[i] = (mSquareWaveCounter[i] + 1) % 8;
        }
    }
}

float convertVolume(int volume)
{
    // Map 0:15 to 0.0:1.0
    float fVolume = volume; // 0.0:15.0
    fVolume /= 15.0F;       // 0.0:1.0
    return fVolume;
}

float APU::getAudioSample() const
{
    // TODO
    float sum = 0.0F;
    for (int i = 0; i < 2; i++)
    {
        if (mSquareEnabled[i])
        {
            sum += (SQUARE_DUTY_WAVES[NRx1[i] >> WAVE_DUTY_BIT][mSquareWaveCounter[i]] ? 1.0F : -1.0F) * convertVolume(mSquareVolume[i]);
        }
    }
    return sum * 0.25F; // reduce audio amplitude from -4.0:4.0 (4 channels sum) to -1.0:1.0
}

float APU::getLeftAudioSample() const
{
    // TODO
    return getAudioSample();
}

float APU::getRightAudioSample() const
{
    // TODO
    return getAudioSample();
}

void APU::writeNRx1(int channel, uint8_t value)
{
    NRx1[channel] = value;
    mSquareLengthCounter[channel] = 64 - (value & INITIAL_LENGTH_TIMER_BITMASK);
}

void APU::writeNRx4(int channel, uint8_t value)
{
    NRx4[channel] = value;
    if (NRx4[channel] >> TRIGGER_BIT)
    {
        triggerChannel(channel);
    }
}

void APU::decrementLengthCounters()
{
    for (int i = 0; i < 2; i++)
    {
        if ((NRx4[i] & LENGTH_ENABLE_BITMASK) && mSquareLengthCounter[i] > 0)
        {
            mSquareLengthCounter[i]--;
            if (mSquareLengthCounter[i] == 0)
            {
                mSquareEnabled[i] = false;
            }
        }
    }
}

void APU::decrementVolumeEnvelopes()
{
    for (int i = 0; i < 2; i++)
    {
        if (mSquareEnvCounter[i] > 0)
        {
            mSquareEnvCounter[i]--;
            if (mSquareEnvCounter[i] == 0)
            {
                switch (mSquareEnvDir[i])
                {
                case 0:
                    mSquareVolume[i]--;
                    break;

                case 1:
                    mSquareVolume[i]++;
                    break;

                default:
                    break;
                }

                if (mSquareVolume[i] <= 0)
                {
                    mSquareVolume[i] = 0;
                }
                else if (mSquareVolume[i] >= MAX_VOL)
                {
                    mSquareVolume[i] = MAX_VOL;
                }
                else
                {
                    mSquareEnvCounter[i] = mSquareEnvPace[i];
                }
            }
        }
    }
}

void APU::reloadFrequencyTimer(int channel)
{
    int freq;

    switch (channel)
    {
    case 0:
    case 1:
        freq = ((NRx4[channel] & PERIOD_HIGH_BITMASK) << 8) | NRx3[channel];
        mSquareFrequencyTimer[channel] = (2048 - freq) * 4;
        break;

    default:
        break;
    }
}

void APU::triggerChannel(int channel)
{
    mSquareEnabled[channel] = true;
    if (mSquareLengthCounter[channel] == 0)
    {
        mSquareLengthCounter[channel] = 64;
    }
    reloadFrequencyTimer(channel);
    mSquareVolume[channel] = NRx2[channel] >> INITIAL_VOLUME_BIT;
    mSquareEnvDir[channel] = (NRx2[channel] & ENV_DIR_BITMASK) ? 1 : 0;
    mSquareEnvPace[channel] = NRx2[channel] & ENV_PACE_BITMASK;
    mSquareEnvCounter[channel] = mSquareEnvPace[channel];
}
