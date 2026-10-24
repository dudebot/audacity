/*
 * Audacity: A Digital Audio Editor
 */
#include "internal/spectrogramcolors.h"
#include "internal/roseuscolormaps.h"
#include "internal/spectrogramcolordefinitions.h"
#include "internal/spectrogramcolorregister.h"

#include <cmath>
#include <vector>

namespace au::spectrogram {
namespace {
double stereoLinearRgb(double value)
{
    return value <= .04045 ? value / 12.92 : std::pow((value + .055) / 1.055, 2.4);
}

double stereoEncodedRgb(double value)
{
    value = std::clamp(value, 0., 1.);
    return value <= .0031308 ? 12.92 * value : 1.055 * std::pow(value, 1. / 2.4) - .055;
}

// Expand color differences toward the available gamut without a hard cutoff.
// The neutral point has the SAME linear-light luminance as the input color;
// consequently expansion changes color separation, not the intensity ramp.
StereoBalanceColor stereoPaletteEntry(double left, double right, StereoBalancePalette palette, bool selected)
{
    // A shared fixed curve reserves bright blends for two strong channels.
    // Unlike squaring, it retains a nonzero slope at the display floor.
    left = left / (3. - 2. * left);
    right = right / (3. - 2. * right);
    double encoded[3] { left, 0., right };
    switch (palette) {
    case StereoBalancePalette::GreenMagenta:
        encoded[1] = right;
        encoded[2] = left;
        break;
    case StereoBalancePalette::OrangeBlue:
        encoded[1] = .4 * left + .6 * right;
        break;
    default:
        break;
    }
    double linear[3] { stereoLinearRgb(encoded[0]), stereoLinearRgb(encoded[1]), stereoLinearRgb(encoded[2]) };
    const double luminance = .2126 * linear[0] + .7152 * linear[1] + .0722 * linear[2];
    double neutral[3] { luminance, luminance, luminance };
    if (palette == StereoBalancePalette::RedBlue) {
        // Red/blue lives in the G=0 plane: its equal-level neutral is purple.
        neutral[0] = neutral[2] = std::clamp(luminance / (.2126 + .0722), 0., 1.);
        neutral[1] = 0.;
    }
    double radius = 0.;
    for (int channel = 0; channel < 3; ++channel) {
        const double difference = linear[channel] - neutral[channel];
        if (difference > 0. && neutral[channel] < 1.) {
            radius = std::max(radius, difference / (1. - neutral[channel]));
        } else if (difference < 0. && neutral[channel] > 0.) {
            radius = std::max(radius, -difference / neutral[channel]);
        }
    }
    const double expansion = 5. / (1. + 4. * std::clamp(radius, 0., 1.));
    const double scale = 255. * (selected ? .72 : 1.);
    StereoBalanceColor result;
    std::uint8_t* channels[] { &result.red, &result.green, &result.blue };
    for (int channel = 0; channel < 3; ++channel) {
        const double value = neutral[channel] + expansion * (linear[channel] - neutral[channel]);
        *channels[channel] = static_cast<std::uint8_t>(std::lround(scale * stereoEncodedRgb(value)));
    }
    return result;
}
}

// Shared dB normalization preserves the quieter channel. A bounded lookup table
// keeps gamma conversion and gamut calculations out of the pixel loop. No FFT
// or source audio data is transformed when switching palettes.
StereoBalanceColor stereoBalanceColor(float leftDb, float rightDb,
                                             int gainDb, int rangeDb,
                                             StereoBalancePalette palette,
                                             bool selected)
{
    if (!std::isfinite(leftDb) || !std::isfinite(rightDb) || rangeDb <= 0) {
        return {};
    }
    constexpr int steps = 256;
    constexpr int paletteCount = static_cast<int>(StereoBalancePalette::_count);
    static const auto colors = [] {
        // Heap storage avoids a large temporary on the GUI thread's stack.
        std::vector<StereoBalanceColor> table(paletteCount * steps * steps * 2);
        for (int p = 0; p < paletteCount; ++p) {
            for (int l = 0; l < steps; ++l) {
                for (int r = 0; r < steps; ++r) {
                    for (int s = 0; s < 2; ++s) {
                        table[((p * steps + l) * steps + r) * 2 + s] = stereoPaletteEntry(
                            l / 255., r / 255., static_cast<StereoBalancePalette>(p), s != 0);
                    }
                }
            }
        }
        return table;
    }();
    const auto position = [gainDb, rangeDb](float db) {
        return 255. * std::clamp((static_cast<double>(db) + rangeDb + gainDb) / rangeDb, 0., 1.);
    };
    const double left = position(leftDb);
    const double right = position(rightDb);
    const int l0 = static_cast<int>(left);
    const int r0 = static_cast<int>(right);
    const int l1 = std::min(l0 + 1, steps - 1);
    const int r1 = std::min(r0 + 1, steps - 1);
    const double leftFraction = left - l0;
    const double rightFraction = right - r0;
    const int p = static_cast<int>(stereoBalancePaletteFromInt(static_cast<int>(palette)));
    const auto entry = [&](int l, int r) -> const StereoBalanceColor& {
        return colors[((p * steps + l) * steps + r) * 2 + (selected ? 1 : 0)];
    };
    const auto& c00 = entry(l0, r0);
    const auto& c10 = entry(l1, r0);
    const auto& c01 = entry(l0, r1);
    const auto& c11 = entry(l1, r1);
    // Interpolate the bounded display table rather than snapping each channel
    // to a cell. This avoids visible steps near the gamut boundary while
    // keeping gamma conversion and gamut expansion outside the paint loop.
    const auto interpolate = [&](int a, int b, int c, int d) {
        const double lower = a + leftFraction * (b - a);
        const double upper = c + leftFraction * (d - c);
        return static_cast<std::uint8_t>(std::lround(lower + rightFraction * (upper - lower)));
    };
    return { interpolate(c00.red, c10.red, c01.red, c11.red),
             interpolate(c00.green, c10.green, c01.green, c11.green),
             interpolate(c00.blue, c10.blue, c01.blue, c11.blue) };
}


unsigned char SpectrogramColors::gradient_pre[ColorGradientTotal][colorSchemes][gradientSteps][3];

bool SpectrogramColors::gradient_inited = 0;

void SpectrogramColors::PreComputeGradient()
{
    if (gradient_inited) {
        return;
    }
    gradient_inited = 1;
    spectrogramColorRegister.EnsureInitialised();

    // Keep in correspondence with enum Au3SpectrogramSettings::ColorScheme

    // colorScheme 0: Color (New)
    std::copy_n(&roseusColormap[0][0], gradientSteps * 3, &gradient_pre[ColorGradientUnselected][0][0][0]);
    std::copy_n(&roseusSelectedColormap[0][0], gradientSteps * 3, &gradient_pre[ColorGradientTimeSelected][0][0][0]);
    std::copy_n(&roseusFrequencySelectedColormap[0][0], gradientSteps * 3, &gradient_pre[ColorGradientTimeAndFrequencySelected][0][0][0]);
    std::fill_n(&gradient_pre[ColorGradientEdge][0][0][0], gradientSteps * 3, 0);

    for (int selected = 0; selected < ColorGradientTotal; selected++) {
        // Get color scheme from SpectrogramColorRegister
        const int gsteps = 4;
        float gradient[gsteps + 1][3];
        spectrogramColorRegister.Colour(clrSpectro1) = spectrogramColorRegister.Colour(clrUnselected);
        spectrogramColorRegister.Colour(clrSpectro1Sel) = spectrogramColorRegister.Colour(clrSelected);
        int clrFirst = (selected == ColorGradientUnselected) ? clrSpectro1 : clrSpectro1Sel;
        for (int j=0; j < (gsteps + 1); j++) {
            QColor c = spectrogramColorRegister.Colour(clrFirst + j);
            gradient[ j] [0] = c.red() / 255.0;
            gradient[ j] [1] = c.green() / 255.0;
            gradient[ j] [2] = c.blue() / 255.0;
        }

        // colorScheme 1: Color (from theme)
        for (int i = 0; i < gradientSteps; i++) {
            float r, g, b;
            float value = float(i) / gradientSteps;

            int left = (int)(value * gsteps);
            int right = (left == gsteps ? gsteps : left + 1);

            float rweight = (value * gsteps) - left;
            float lweight = 1.0 - rweight;

            r = (gradient[left][0] * lweight) + (gradient[right][0] * rweight);
            g = (gradient[left][1] * lweight) + (gradient[right][1] * rweight);
            b = (gradient[left][2] * lweight) + (gradient[right][2] * rweight);

            switch (selected) {
            case ColorGradientUnselected:
                // not dimmed
                break;

            case ColorGradientTimeAndFrequencySelected:
                float temp;
                temp = r;
                r = g;
                g = b;
                b = temp;
                break;

            case ColorGradientTimeSelected:
                // partly dimmed
                r *= 0.75f;
                g *= 0.75f;
                b *= 0.75f;
                break;

            // For now edge colour is just black (or white if grey-scale)
            // Later we might invert or something else funky.
            case ColorGradientEdge:
                // fully dimmed
                r = 0;
                g = 0;
                b = 0;
                break;
            }
            gradient_pre[selected][1][i][0] = (unsigned char)(255 * r);
            gradient_pre[selected][1][i][1] = (unsigned char)(255 * g);
            gradient_pre[selected][1][i][2] = (unsigned char)(255 * b);
        }

        // colorScheme 3: Inverse Grayscale
        for (int i = 0; i < gradientSteps; i++) {
            float r, g, b;
            float value = float(i) / gradientSteps;

            r = g = b = value;

            switch (selected) {
            case ColorGradientUnselected:
                // not dimmed
                break;

            case ColorGradientTimeAndFrequencySelected:
            // else fall through to SAME grayscale colour as normal selection.
            // The white lines show it up clearly enough.

            case ColorGradientTimeSelected:
                // partly dimmed
                r = r * 0.75f + 0.25f;
                g = g * 0.75f + 0.25f;
                b = b * 0.75f + 0.25f;
                break;

            case ColorGradientEdge:
                r = 1.0f;
                g = 1.0f;
                b = 1.0f;
                break;
            }
            gradient_pre[selected][3][i][0] = (unsigned char)(255 * r);
            gradient_pre[selected][3][i][1] = (unsigned char)(255 * g);
            gradient_pre[selected][3][i][2] = (unsigned char)(255 * b);
        }

        // colorScheme 2: Grayscale (=Old grayscale)
        for (int i = 0; i < gradientSteps; i++) {
            float r, g, b;
            float value = float(i) / gradientSteps;

            r = g = b = 0.84 - 0.84 * value;

            switch (selected) {
            case ColorGradientUnselected:
                // not dimmed
                break;

            case ColorGradientTimeAndFrequencySelected:
            // else fall through to SAME grayscale colour as normal selection.
            // The white lines show it up clearly enough.

            case ColorGradientTimeSelected:
                // partly dimmed
                r *= 0.75f;
                g *= 0.75f;
                b *= 0.75f;
                break;

            // For now edge colour is just black (or white if grey-scale)
            // Later we might invert or something else funky.
            case ColorGradientEdge:
                // fully dimmed
                r = 1.0f;
                g = 1.0f;
                b = 1.0f;
                break;
            }
            gradient_pre[selected][2][i][0] = (unsigned char)(255 * r);
            gradient_pre[selected][2][i][1] = (unsigned char)(255 * g);
            gradient_pre[selected][2][i][2] = (unsigned char)(255 * b);
        }
    }
}
}
