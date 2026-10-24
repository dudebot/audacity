/*
 * Audacity: A Digital Audio Editor
 */
#include "spectrogram/internal/spectrogramcolors.h"

#include <gtest/gtest.h>

#include <array>
#include <cmath>
#include <limits>

using namespace au::spectrogram;

namespace {
constexpr std::array palettes {
    StereoBalancePalette::RedBlue,
    StereoBalancePalette::GreenMagenta,
    StereoBalancePalette::OrangeBlue
};

void expectColor(StereoBalanceColor actual, int red, int green, int blue, int tolerance = 0)
{
    EXPECT_NEAR(actual.red, red, tolerance);
    EXPECT_NEAR(actual.green, green, tolerance);
    EXPECT_NEAR(actual.blue, blue, tolerance);
}

bool sameColor(StereoBalanceColor a, StereoBalanceColor b)
{
    return a.red == b.red && a.green == b.green && a.blue == b.blue;
}
}

TEST(StereoPalette, ChannelEndpointsAndEqualPowerHaveDocumentedColors)
{
    // A shared -75..0 dB scale; the floor contributes no channel color.
    expectColor(stereoBalanceColor(0, -75, 0, 75, StereoBalancePalette::RedBlue), 255, 0, 0);
    expectColor(stereoBalanceColor(-75, 0, 0, 75, StereoBalancePalette::RedBlue), 0, 0, 255);
    expectColor(stereoBalanceColor(0, 0, 0, 75, StereoBalancePalette::RedBlue), 255, 0, 255);

    expectColor(stereoBalanceColor(0, -75, 0, 75, StereoBalancePalette::GreenMagenta), 255, 0, 255);
    expectColor(stereoBalanceColor(-75, 0, 0, 75, StereoBalancePalette::GreenMagenta), 0, 255, 0);
    expectColor(stereoBalanceColor(0, 0, 0, 75, StereoBalancePalette::GreenMagenta), 255, 255, 255);

    expectColor(stereoBalanceColor(0, -75, 0, 75, StereoBalancePalette::OrangeBlue), 255, 102, 0);
    expectColor(stereoBalanceColor(-75, 0, 0, 75, StereoBalancePalette::OrangeBlue), 0, 153, 255);
    expectColor(stereoBalanceColor(0, 0, 0, 75, StereoBalancePalette::OrangeBlue), 255, 255, 255);
}

TEST(StereoPalette, EqualChannelMidpointUsesFixedIntensityCurve)
{
    // u=.5 maps to u/(3-2u)=.25, or about 64 in 8-bit RGB.
    // One code of tolerance allows the 256-step input lookup quantization.
    expectColor(stereoBalanceColor(-37.5, -37.5, 0, 75, StereoBalancePalette::RedBlue), 64, 0, 64, 1);
    for (const auto palette : { StereoBalancePalette::GreenMagenta, StereoBalancePalette::OrangeBlue }) {
        expectColor(stereoBalanceColor(-37.5, -37.5, 0, 75, palette), 64, 64, 64, 1);
    }
}

TEST(StereoPalette, EqualChannelIntensityRisesAcrossTheSharedRange)
{
    for (const auto palette : palettes) {
        auto previous = stereoBalanceColor(-75, -75, 0, 75, palette);
        for (const float db : { -60.f, -45.f, -30.f, -15.f, 0.f }) {
            const auto color = stereoBalanceColor(db, db, 0, 75, palette);
            EXPECT_GT(color.red, previous.red);
            EXPECT_EQ(color.red, color.blue);
            EXPECT_EQ(color.green, palette == StereoBalancePalette::RedBlue ? 0 : color.red);
            previous = color;
        }
    }
}

TEST(StereoPalette, EitherQuieterChannelRemainsVisibleBeyondOldDominanceCutoff)
{
    for (const auto palette : palettes) {
        auto previousLeft = stereoBalanceColor(-75, -10, 0, 75, palette);
        auto previousRight = stereoBalanceColor(-10, -75, 0, 75, palette);
        for (const float quieterDb : { -60.f, -45.f, -30.f, -15.f }) {
            const auto left = stereoBalanceColor(quieterDb, -10, 0, 75, palette);
            const auto right = stereoBalanceColor(-10, quieterDb, 0, 75, palette);
            EXPECT_FALSE(sameColor(left, previousLeft));
            EXPECT_FALSE(sameColor(right, previousRight));
            previousLeft = left;
            previousRight = right;
        }
    }
}

TEST(StereoPalette, FloorAndBelowFloorAreBlack)
{
    for (const auto palette : palettes) {
        expectColor(stereoBalanceColor(-75, -75, 0, 75, palette), 0, 0, 0);
        expectColor(stereoBalanceColor(-160, -160, 0, 75, palette), 0, 0, 0);
    }
}

TEST(StereoPalette, SelectionDimsColorsWithoutReversingChannels)
{
    for (const auto palette : palettes) {
        const auto normal = stereoBalanceColor(-18, -32, 0, 75, palette);
        const auto selected = stereoBalanceColor(-18, -32, 0, 75, palette, true);
        expectColor(selected, static_cast<int>(std::lround(normal.red * .72)),
                    static_cast<int>(std::lround(normal.green * .72)),
                    static_cast<int>(std::lround(normal.blue * .72)), 1);
    }
    expectColor(stereoBalanceColor(0, -75, 0, 75, StereoBalancePalette::RedBlue, true), 184, 0, 0);
    expectColor(stereoBalanceColor(-75, 0, 0, 75, StereoBalancePalette::RedBlue, true), 0, 0, 184);
}

TEST(StereoPalette, InvalidInputDoesNotCreateColoredPixels)
{
    const auto nan = std::numeric_limits<float>::quiet_NaN();
    const auto infinity = std::numeric_limits<float>::infinity();
    for (const auto palette : palettes) {
        expectColor(stereoBalanceColor(0, 0, 0, 0, palette), 0, 0, 0);
        expectColor(stereoBalanceColor(0, 0, 0, -1, palette), 0, 0, 0);
        expectColor(stereoBalanceColor(nan, 0, 0, 75, palette), 0, 0, 0);
        expectColor(stereoBalanceColor(0, infinity, 0, 75, palette), 0, 0, 0);
    }
}

TEST(StereoPalette, ExtremeFiniteLevelsClampToGamutEndpoints)
{
    const auto largest = std::numeric_limits<float>::max();
    for (const auto palette : palettes) {
        EXPECT_TRUE(sameColor(stereoBalanceColor(largest, -largest, 0, 75, palette),
                              stereoBalanceColor(0, -75, 0, 75, palette)));
        EXPECT_TRUE(sameColor(stereoBalanceColor(-largest, largest, 0, 75, palette),
                              stereoBalanceColor(-75, 0, 0, 75, palette)));
        expectColor(stereoBalanceColor(-largest, -largest, 0, 75, palette), 0, 0, 0);
    }
}

TEST(StereoPalette, InvalidStoredPaletteFallsBackToRedBlue)
{
    for (const int invalid : { -1, static_cast<int>(StereoBalancePalette::_count), 999 }) {
        EXPECT_EQ(stereoBalancePaletteFromInt(invalid), StereoBalancePalette::RedBlue);
        EXPECT_TRUE(sameColor(stereoBalanceColor(-15, -35, 0, 75, static_cast<StereoBalancePalette>(invalid)),
                              stereoBalanceColor(-15, -35, 0, 75, StereoBalancePalette::RedBlue)));
    }
    for (const auto palette : palettes) {
        EXPECT_EQ(stereoBalancePaletteFromInt(static_cast<int>(palette)), palette);
    }
}

TEST(StereoPalette, DisplayGainUsesOneScaleForBothChannels)
{
    for (const auto palette : palettes) {
        EXPECT_TRUE(sameColor(stereoBalanceColor(-45, -65, 20, 80, palette),
                              stereoBalanceColor(-25, -45, 0, 80, palette)));
    }
}
