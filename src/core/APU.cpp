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

float convertVolume(int volume)
{
    // Map 0:15 to 0.0:1.0
    float fVolume = volume; // 0.0:15.0
    fVolume /= 15.0F;       // 0.0:1.0
    return fVolume;
}

float convertToAnalog(int signal, int volCode)
{
    // Map 0:x to -1.0:1.0
    if (volCode == 0)
    {
        return 0.0F;
    }
    float fSignal = signal;
    float halfRange = APU::VOL_CODE_HALF_RANGES[volCode];
    fSignal -= halfRange;
    fSignal /= 7.5F;
    return fSignal;
}

void APU::computeAudioSamples()
{
    leftAudioSample = 0.0F;
    rightAudioSample = 0.0F;
    if (!mAPUEnabled)
    {
        return;
    }

    float sum = 0.0F;
    for (int i = 0; i < 2; i++)
    {
        if (mChannelEnabled[i] && mDAC[i])
        {
            sum = (SQUARE_DUTY_WAVES[NRx1[i] >> WAVE_DUTY_BIT][mSquareWaveCounter[i]] ? 1.0F : -1.0F) * convertVolume(mChannelVolume[i]);
            if (NR51 & (1 << (i + LEFT_VOL_BIT)))
            {
                leftAudioSample += sum;
            }
            if (NR51 & (1 << i))
            {
                rightAudioSample += sum;
            }
        }
    }
    if (mChannelEnabled[2] && mDAC[2])
    {
        int volCode = (NRx2[2] >> WAVE_CHANNEL_VOLUME_BIT) & 0b11;
        sum = convertToAnalog(mWaveSampleBuffer >> ((volCode + 4) % 5), volCode);
        if (NR51 & (1 << (2 + LEFT_VOL_BIT)))
        {
            leftAudioSample += sum;
        }
        if (NR51 & (1 << 2))
        {
            rightAudioSample += sum;
        }
    }
    if (mChannelEnabled[3] && mDAC[3])
    {
        // Waveform output is bit 0 of the LFSR, INVERTED.
        sum = ((mLFSR & 1) ? -1.0F : 1.0F) * convertVolume(mChannelVolume[3]);
        if (NR51 & (1 << (3 + LEFT_VOL_BIT)))
        {
            leftAudioSample += sum;
        }
        if (NR51 & (1 << 3))
        {
            rightAudioSample += sum;
        }
    }
    // Apply volume and reduce audio amplitude from -4.0:4.0 (4 channels sum) to -1.0:1.0
    leftAudioSample *= 0.25F * float(((NR50 >> LEFT_VOL_BIT) & VOL_BITMASK) + 1) / 8.0F;
    rightAudioSample *= 0.25F * float((NR50 & VOL_BITMASK) + 1) / 8.0F;
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

void APU::clockSweep()
{
    if (mSquare1SweepEnabled && ((NR10 >> SWEEP_PACE_BIT) & SWEEP_PACE_BITMASK))
    {
        mSquare1SweepTimer--;
        if (mSquare1SweepTimer == 0)
        {
            mSquare1SweepTimer = ((NR10 >> SWEEP_PACE_BIT) & SWEEP_PACE_BITMASK);
            sweepFreqCalcAndOverflowCheck(true);
        }
    }
}

int APU::calcFrequency(int channel)
{
    return ((NRx4[channel] & PERIOD_HIGH_BITMASK) << 8) | NRx3[channel];
}

void APU::reloadFrequencyTimer(int channel)
{
    int freq = calcFrequency(channel);

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

void APU::sweepFreqCalcAndOverflowCheck(bool writePeriodAndRepeat)
{
    int calcFreq = mSquare1FreqSweepShadow >> (NR10 & SWEEP_STEP_BITMASK);
    if (NR10 & SWEEP_DIRECTION_BITMASK)
    {
        calcFreq = -calcFreq;
    }
    calcFreq += mSquare1FreqSweepShadow;
    if (calcFreq > 2047)
    {
        mChannelEnabled[0] = false;
    }
    else if (writePeriodAndRepeat && (NR10 & SWEEP_STEP_BITMASK))
    {
        mSquare1FreqSweepShadow = calcFreq;
        NRx3[0] = mSquare1FreqSweepShadow & 0xFF;
        NRx4[0] &= ~PERIOD_HIGH_BITMASK;
        NRx4[0] |= (mSquare1FreqSweepShadow >> 8) & PERIOD_HIGH_BITMASK;
        sweepFreqCalcAndOverflowCheck();
    }
}

void APU::triggerChannel(int channel)
{
    mChannelEnabled[channel] = mDAC[channel];
    if (mChannelLengthCounter[channel] == 0)
    {
        mChannelLengthCounter[channel] = channel == 2 ? 256 : 64;
    }
    reloadFrequencyTimer(channel);
    if (channel == 3)
    {
        mLFSR = 0x7FFF;
    }
    switch (channel)
    {
    case 0:
        mSquare1FreqSweepShadow = calcFrequency(channel);
        mSquare1SweepTimer = ((NR10 >> SWEEP_PACE_BIT) & SWEEP_PACE_BITMASK);
        mSquare1SweepEnabled = (NR10 & SWEEP_STEP_BITMASK) || ((NR10 >> SWEEP_PACE_BIT) & SWEEP_PACE_BITMASK);
        if (NR10 & SWEEP_STEP_BITMASK)
        {
            sweepFreqCalcAndOverflowCheck();
        }
    case 1:
    case 3:
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

uint8_t APU::readNR52()
{
    uint8_t NR52 = 0x70;
    if (mAPUEnabled)
    {
        NR52 += AUDIO_ON_OFF_BITMASK;
    }
    for (int i = 0; i < 4; i++)
    {
        if (mChannelEnabled[i])
        {
            NR52 += (1 << i);
        }
    }
    return NR52;
}

void APU::writeNR52(uint8_t value)
{
    bool wasOff = !mAPUEnabled;
    mAPUEnabled = (value & AUDIO_ON_OFF_BITMASK) ? true : false;
    if (wasOff && mAPUEnabled)
    {
        mFrameSequencerStep = 0;
        mSquareWaveCounter[0] = 0;
        mSquareWaveCounter[1] = 0;
        mWaveSampleBuffer = 0;
    }
}
