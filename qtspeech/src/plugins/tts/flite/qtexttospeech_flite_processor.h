// Copyright (C) 2022 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only

#ifndef QTEXTTOSPEECHPROCESSOR_FLITE_H
#define QTEXTTOSPEECHPROCESSOR_FLITE_H

#include "qtexttospeechengine.h"
#include "qvoice.h"

#include <QtCore/QList>
#include <QtCore/QMutex>
#include <QtCore/QThread>
#include <QtCore/QSharedPointer>
#include <QtCore/QLibrary>
#include <QtCore/QString>
#include <QtCore/QBasicTimer>
#include <QtCore/QTimerEvent>
#include <QtCore/QAbstractEventDispatcher>
#include <QtCore/QProcessEnvironment>
#include <QtCore/QDateTime>
#include <QtMultimedia/QAudioSink>
#include <QtMultimedia/QMediaDevices>

#include <flite/flite.h>

QT_BEGIN_NAMESPACE

class QTextToSpeechProcessorFlite : public QObject
{
    Q_OBJECT

public:
    QTextToSpeechProcessorFlite(const QAudioDevice &audioDevice);
    ~QTextToSpeechProcessorFlite();

    struct VoiceInfo
    {
        int id;
        cst_voice *vox;
        void (*unregister_func)(cst_voice *vox);
        QString name;
        QString locale;
        QVoice::Gender gender;
        QVoice::Age age;
    };

    Q_INVOKABLE void say(const QString &text, int voiceId, double pitch, double rate, double volume);
    Q_INVOKABLE void pause();
    Q_INVOKABLE void resume();
    Q_INVOKABLE void stop();

    const QList<QTextToSpeechProcessorFlite::VoiceInfo> &voices() const;
    static constexpr QTextToSpeech::State audioStateToTts(QAudio::State audioState);

private:
    // Process a single text
    void processText(const QString &text, int voiceId, double pitch, double rate);

    // Flite callbacks
    static int fliteOutputCb(const cst_wave *w, int start, int size,
                            int last, cst_audio_streaming_info *asi);
    int fliteOutput(const cst_wave *w, int start, int size,
                    int last, cst_audio_streaming_info *asi);

    bool audioOutput(const char *data, qint64 dataSize, QString &errorString);
    void setRateForVoice(cst_voice *voice, float rate);
    void setPitchForVoice(cst_voice *voice, float pitch);

    bool init();
    bool initAudio(double rate, int channelCount);
    void deinitAudio();
    bool checkFormat(const QAudioFormat &format);
    bool checkVoice(int voiceId);
    void deleteSink();
    void createSink();
    QAudio::State audioSinkState() const;
    void setError(QTextToSpeech::ErrorReason err, const QString &errorString = QString());

    // Read available flite voices
    QStringList fliteAvailableVoices(const QString &libPrefix, const QString &langCode) const;

private slots:
    void changeState(QAudio::State newState);

Q_SIGNALS:
    void errorOccurred(QTextToSpeech::ErrorReason error, const QString &errorString);
    void stateChanged(QTextToSpeech::State);

private:
    QAudioSink *m_audioSink = nullptr;
    QAudio::State m_state = QAudio::IdleState;
    QIODevice *m_audioBuffer = nullptr;

    QAudioDevice m_audioDevice;
    QAudioFormat m_format;
    double m_volume = 1;

    QList<VoiceInfo> m_voices;

    // Statistics for debugging
    qint64 numberChunks = 0;
    qint64 totalBytes = 0;
};

QT_END_NAMESPACE

#endif
