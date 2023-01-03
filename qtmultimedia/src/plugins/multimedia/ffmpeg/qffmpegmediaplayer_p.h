// Copyright (C) 2021 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

//
//  W A R N I N G
//  -------------
//
// This file is not part of the Qt API.  It exists purely as an
// implementation detail.  This header file may change from version to
// version without notice, or even be removed.
//
// We mean it.
//

#ifndef QFFMPEGMEDIAPLAYER_H
#define QFFMPEGMEDIAPLAYER_H

#include <private/qplatformmediaplayer_p.h>
#include <qmediametadata.h>
#include <qtimer.h>
#include "qffmpeg_p.h"

QT_BEGIN_NAMESPACE

namespace QFFmpeg {
class Decoder;
}
class QPlatformAudioOutput;

class QFFmpegMediaPlayer : public QObject, public QPlatformMediaPlayer
{
    Q_OBJECT
public:
    QFFmpegMediaPlayer(QMediaPlayer *player);
    ~QFFmpegMediaPlayer();

    qint64 duration() const override;

    void setPosition(qint64 position) override;

    float bufferProgress() const override;

    QMediaTimeRange availablePlaybackRanges() const override;

    qreal playbackRate() const override;
    void setPlaybackRate(qreal rate) override;

    QUrl media() const override;
    const QIODevice *mediaStream() const override;
    void setMedia(const QUrl &media, QIODevice *stream) override;

    void play() override;
    void pause() override;
    void stop() override;

//    bool streamPlaybackSupported() const { return false; }

    void setAudioOutput(QPlatformAudioOutput *) override;

    QMediaMetaData metaData() const override;

    void setVideoSink(QVideoSink *sink) override;
    QVideoSink *videoSink() const;

    int trackCount(TrackType) override;
    QMediaMetaData trackMetaData(TrackType type, int streamNumber) override;
    int activeTrack(TrackType) override;
    void setActiveTrack(TrackType, int streamNumber) override;

    Q_INVOKABLE void delayedLoadedStatus() { mediaStatusChanged(QMediaPlayer::LoadedMedia); }

private slots:
    void updatePosition();
    void endOfStream();
    void error(int error, const QString &errorString)
    {
        QPlatformMediaPlayer::error(error, errorString);
    }

private:
    friend class QFFmpeg::Decoder;

    QTimer positionUpdateTimer;

    QFFmpeg::Decoder *decoder = nullptr;
    QPlatformAudioOutput *m_audioOutput = nullptr;
    QVideoSink *m_videoSink = nullptr;

    QUrl m_url;
    QIODevice *m_device = nullptr;
    float m_playbackRate = 1.;
};

QT_END_NAMESPACE


#endif  // QMEDIAPLAYERCONTROL_H

