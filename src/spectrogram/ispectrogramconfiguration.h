/*
 * Audacity: A Digital Audio Editor
 */
#pragma once

#include "spectrogramtypes.h"

namespace au::spectrogram {
class ISpectrogramConfiguration
{
public:
    virtual ~ISpectrogramConfiguration() = default;

    virtual double minFreq() = 0;
    virtual void setMinFreq(double value) = 0;

    virtual double maxFreq() = 0;
    virtual void setMaxFreq(double value) = 0;

    virtual SpectrogramColorScheme colorScheme() = 0;
    virtual void setColorScheme(SpectrogramColorScheme value) = 0;

    // A unified stereo display uses brightness for mean power and hue for the
    // left/right ratio.  It is deliberately opt-in so existing spectrograms
    // keep their ordinary palette and per-channel layout.
    virtual bool stereoCombined() = 0;
    virtual void setStereoCombined(bool value) = 0;

    virtual double stereoBalanceGain() = 0;
    virtual void setStereoBalanceGain(double value) = 0;

    virtual int colorGainDb() = 0;
    virtual void setColorGainDb(int value) = 0;

    virtual int colorRangeDb() = 0;
    virtual void setColorRangeDb(int value) = 0;

    /**
     * @brief Color-high boost in dB per decave (portmanteau word for "decade" and "octave")
     */
    virtual int colorHighBoostDbPerDec() = 0;
    virtual void setColorHighBoostDbPerDec(int value) = 0;

    virtual SpectrogramScale scale() = 0;
    virtual void setScale(SpectrogramScale value) = 0;

    virtual SpectrogramAlgorithm algorithm() = 0;
    virtual void setAlgorithm(SpectrogramAlgorithm value) = 0;

    virtual SpectrogramWindowType windowType() = 0;
    virtual void setWindowType(SpectrogramWindowType value) = 0;

    virtual int winSizeLog2() = 0;
    virtual void setWinSizeLog2(int value) = 0;

    virtual int zeroPaddingFactor() = 0;
    virtual void setZeroPaddingFactor(int value) = 0;
};
} // namespace au::spectrogram
