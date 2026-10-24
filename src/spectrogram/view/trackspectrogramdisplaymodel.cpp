/*
 * Audacity: A Digital Audio Editor
 */
#include "trackspectrogramdisplaymodel.h"

namespace au::spectrogram {
TrackSpectrogramDisplayModel::TrackSpectrogramDisplayModel(QObject* parent)
    : QObject(parent), muse::Contextable(muse::iocCtxForQmlObject(this))
{
}

void TrackSpectrogramDisplayModel::componentComplete()
{
    spectrogramService()->trackSpectrogramConfigurationChanged().onReceive(this, [this](int changedTrackId) {
        if (changedTrackId == m_trackId) {
            emit stereoCombinedChanged();
        }
    });
}

void TrackSpectrogramDisplayModel::setTrackId(int value)
{
    if (m_trackId == value) {
        return;
    }
    m_trackId = value;
    emit trackIdChanged();
    emit stereoCombinedChanged();
}

bool TrackSpectrogramDisplayModel::stereoCombined() const
{
    const auto config = spectrogramService()->trackSpectrogramConfiguration(m_trackId);
    return config && config->stereoCombined()
           && config->algorithm() == SpectrogramAlgorithm::Frequencies;
}
} // namespace au::spectrogram
