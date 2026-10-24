/*
 * Audacity: A Digital Audio Editor
 */
#pragma once

#include <QObject>
#include <QQmlParserStatus>

#include "spectrogram/ispectrogramservice.h"
#include "framework/global/async/asyncable.h"
#include "framework/global/modularity/ioc.h"

namespace au::spectrogram {
// Unlike TrackSpectrogramSettingsModel this object has no dialog snapshot.
// It is safe to keep in a clip/ruler QML delegate to observe display state.
class TrackSpectrogramDisplayModel : public QObject, public muse::Contextable, public muse::async::Asyncable, public QQmlParserStatus
{
    Q_OBJECT
    Q_INTERFACES(QQmlParserStatus)
    Q_PROPERTY(int trackId READ trackId WRITE setTrackId NOTIFY trackIdChanged)
    Q_PROPERTY(bool stereoCombined READ stereoCombined NOTIFY stereoCombinedChanged)

    muse::ContextInject<ISpectrogramService> spectrogramService { this };

public:
    explicit TrackSpectrogramDisplayModel(QObject* parent = nullptr);

    int trackId() const { return m_trackId; }
    void setTrackId(int value);
    bool stereoCombined() const;

signals:
    void trackIdChanged();
    void stereoCombinedChanged();

private:
    void classBegin() override {}
    void componentComplete() override;

    int m_trackId = -1;
};
} // namespace au::spectrogram
