/*
 * Audacity: A Digital Audio Editor
 */
#pragma once

#include "spectrogramtypes.h"

#include <algorithm>
#include <cmath>
#include <cstdint>

namespace au::spectrogram {
struct StereoBalanceColor {
    std::uint8_t red = 0;
    std::uint8_t green = 0;
    std::uint8_t blue = 0;
};

// Convert *unclipped dB power* from a pair of FFTs into the combined-stereo
// palette.  Brightness is the mean linear power; hue is the signed L/R power
// ratio.  Keeping this here (rather than operating on SpecPxCache values) is
// deliberate: those values have already been clipped and cannot express a
// meaningful channel ratio.
inline StereoBalanceColor stereoBalanceColor(float leftDb, float rightDb,
                                             int gainDb, int rangeDb,
                                             double balanceGain,
                                             bool selected = false)
{
    if (!std::isfinite(leftDb) || !std::isfinite(rightDb) || rangeDb <= 0
        || !std::isfinite(balanceGain) || balanceGain <= 0.) {
        return {};
    }

    // log10((10^(L/10) + 10^(R/10)) / 2), calculated relative to the
    // stronger channel so normal spectrogram values do not overflow.
    const double stronger = std::max(leftDb, rightDb);
    const double meanDb = stronger + 10. * std::log10(
        (std::pow(10., (leftDb - stronger) / 10.)
        + std::pow(10., (rightDb - stronger) / 10.)) / 2.);
    const double brightness = std::clamp((meanDb + rangeDb + gainDb) / rangeDb, 0., 1.);

    // A gain of 1 saturates at +/-12 dB, matching the documented HypnoCLI
    // convention.  The default 2.5 therefore saturates at +/-4.8 dB.
    const double balance = std::clamp(balanceGain * (leftDb - rightDb) / 12., -1., 1.);
    const double dim = selected ? .72 : 1.; // Never permute channels for selection.
    return {
        static_cast<std::uint8_t>(std::lround(127.5 * brightness * dim * (1. + balance))),
        0,
        static_cast<std::uint8_t>(std::lround(127.5 * brightness * dim * (1. - balance)))
    };
}

class SpectrogramColors
{
public:

    enum ColorGradientChoice {
        ColorGradientUnselected = 0,
        ColorGradientTimeSelected,
        ColorGradientTimeAndFrequencySelected,
        ColorGradientEdge,

        ColorGradientTotal // keep me last
    };

    static void PreComputeGradient();

    static bool gradient_inited;
    static const int colorSchemes = static_cast<int>(SpectrogramColorScheme::_count);
    static const int gradientSteps = 256;
    static unsigned char gradient_pre[ColorGradientTotal][colorSchemes][gradientSteps][3];
};

inline void GetColorGradient(float value,
                             SpectrogramColors::ColorGradientChoice selected,
                             SpectrogramColorScheme colorScheme,
                             unsigned char* __restrict red,
                             unsigned char* __restrict green,
                             unsigned char* __restrict blue)
{
    int idx = value * (SpectrogramColors::gradientSteps - 1);

    const auto c = static_cast<int>(colorScheme);
    *red = SpectrogramColors::gradient_pre[selected][c][idx][0];
    *green = SpectrogramColors::gradient_pre[selected][c][idx][1];
    *blue = SpectrogramColors::gradient_pre[selected][c][idx][2];
}
}
