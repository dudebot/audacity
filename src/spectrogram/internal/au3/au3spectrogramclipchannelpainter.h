/*
 * Audacity: A Digital Audio Editor
 */
#pragma once

#include "spectrogramtypes.h"
#include "./au3spectrogramtypes.h"

#include "au3-wave-track/WaveClip.h"

#include <QImage>

namespace au::spectrogram::Au3SpectrogramClipChannelPainter {
void fillImage(QImage&, const ViewInfo&, const SelectionInfo&, const SpectrogramTrackContext&, WaveClipChannel&);
// Renders both raw channel FFT caches into one image.  The callers own the
// decision to use this mode; mono clips continue through fillImage().
void fillStereoCombinedImage(QImage&, const ViewInfo&, const SelectionInfo&, const SpectrogramTrackContext&,
                             WaveClipChannel& left, WaveClipChannel& right);
}
