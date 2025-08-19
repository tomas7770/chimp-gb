#include "APU.h"

APU::APU()
{
    writeNRx1(0, 0xBF);
    writeNRx4(0, 0xBF);

    writeNRx1(1, 0x3F);
    writeNRx4(1, 0xBF);

    writeNRx1(2, 0xFF);
    writeNRx4(2, 0xBF);

    writeNRx1(3, 0xFF);
    writeNRx4(3, 0xBF);
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

    for (int i = 0; i < 4; i++)
    {
        mChannelFrequencyTimer[i]--;
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
                    mWaveSampleBuffer = waveRam[mWavePositionCounter / 2] >> 4;
                }
                else
                {
                    mWaveSampleBuffer = waveRam[mWavePositionCounter / 2] & 0xF;
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

float convertVolume(int volume)
{
    // Map 0:15 to 0.0:1.0
    float fVolume = volume; // 0.0:15.0
    fVolume /= 15.0F;       // 0.0:1.0
    return fVolume;
}

float convertToAnalog(int signal)
{
    // Map 0:15 to -1.0:1.0
    float fSignal = signal; // 0.0:15.0
    fSignal -= 7.5F;        // -7.5:7.5
    fSignal /= 7.5F;        // -1.0:1.0
    return fSignal;
}

float APU::getAudioSample() const
{
    // TODO
    float sum = 0.0F;
    for (int i = 0; i < 2; i++)
    {
        if (mChannelEnabled[i])
        {
            sum += (SQUARE_DUTY_WAVES[NRx1[i] >> WAVE_DUTY_BIT][mSquareWaveCounter[i]] ? 1.0F : -1.0F) * convertVolume(mChannelVolume[i]);
        }
    }
    if (mChannelEnabled[2])
    {
        int volCode = (NRx2[2] >> WAVE_CHANNEL_VOLUME_BIT) & 0b11;
        sum += convertToAnalog(mWaveSampleBuffer >> ((volCode + 4) % 5));
    }
    if (mChannelEnabled[3])
    {
        // Waveform output is bit 0 of the LFSR, INVERTED.
        sum += ((mLFSR & 1) ? -1.0F : 1.0F) * convertVolume(mChannelVolume[3]);
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
    if (channel == 2)
    {
        mChannelLengthCounter[channel] = 256 - value;
    }
    else
    {
        mChannelLengthCounter[channel] = 64 - (value & INITIAL_LENGTH_TIMER_BITMASK);
    }
}

void APU::writeNRx4(int channel, uint8_t value)
{
    NRx4[channel] = value;
    if (NRx4[channel] & TRIGGER_BITMASK)
    {
        triggerChannel(channel);
    }
}

void APU::decrementLengthCounters()
{
    for (int i = 0; i < 4; i++)
    {
        if ((NRx4[i] & LENGTH_ENABLE_BITMASK) && mChannelLengthCounter[i] > 0)
        {
            mChannelLengthCounter[i]--;
            if (mChannelLengthCounter[i] == 0)
            {
                mChannelEnabled[i] = false;
            }
        }
    }
}

void APU::decrementVolumeEnvelopes()
{
    for (int i = 0; i < 4; i++)
    {
        if (i == 2)
        {
            continue;
        }
        if (mChannelEnvCounter[i] > 0)
        {
            mChannelEnvCounter[i]--;
            if (mChannelEnvCounter[i] == 0)
            {
                switch (mChannelEnvDir[i])
                {
                case 0:
                    mChannelVolume[i]--;
                    break;

                case 1:
                    mChannelVolume[i]++;
                    break;

                default:
                    break;
                }

                if (mChannelVolume[i] <= 0)
                {
                    mChannelVolume[i] = 0;
                }
                else if (mChannelVolume[i] >= MAX_VOL)
                {
                    mChannelVolume[i] = MAX_VOL;
                }
                else
                {
                    mChannelEnvCounter[i] = mChannelEnvPace[i];
                }
            }
        }
    }
}

void APU::reloadFrequencyTimer(int channel)
{
    int freq = ((NRx4[channel] & PERIOD_HIGH_BITMASK) << 8) | NRx3[channel];

    switch (channel)
    {
    case 0:
    case 1:
        mChannelFrequencyTimer[channel] = (2048 - freq) * 4;
        break;

    case 2:
        mChannelFrequencyTimer[channel] = (2048 - freq) * 2;
        break;

    case 3:
        mChannelFrequencyTimer[channel] = NOISE_FREQ_TIMER_VALUES[NRx3[channel] & NOISE_CLOCK_DIVIDER_BITMASK]
                                          << (NRx3[channel] >> NOISE_CLOCK_SHIFT_BIT);
        break;

    default:
        break;
    }
}

void APU::triggerChannel(int channel)
{
    mChannelEnabled[channel] = true;
    if (mChannelLengthCounter[channel] == 0)
    {
        mChannelLengthCounter[channel] = channel == 2 ? 256 : 64;
    }
    reloadFrequencyTimer(channel);
    switch (channel)
    {
    case 3:
        mLFSR = 0x7FFF;
    case 0:
    case 1:
        mChannelVolume[channel] = NRx2[channel] >> INITIAL_VOLUME_BIT;
        mChannelEnvDir[channel] = (NRx2[channel] & ENV_DIR_BITMASK) ? 1 : 0;
        mChannelEnvPace[channel] = NRx2[channel] & ENV_PACE_BITMASK;
        mChannelEnvCounter[channel] = mChannelEnvPace[channel];
        break;

    case 2:
        mWavePositionCounter = 0;
        break;

    default:
        break;
    }
}
