#include "AudioPlayer.h"
#include <QDebug>

AudioPlayer::AudioPlayer(QObject *parent)
    : QObject(parent),
      mediaPlayer(new QMediaPlayer(this)),
      audioOutput(new QAudioOutput(this))
{
    // Set up audio output with default device
    mediaPlayer->setAudioOutput(audioOutput);
    
    connect(mediaPlayer, &QMediaPlayer::playbackStateChanged,
            this, &AudioPlayer::handlePlaybackStateChanged);
    connect(mediaPlayer, &QMediaPlayer::mediaStatusChanged,
            this, &AudioPlayer::handleMediaStatusChanged);
}

AudioPlayer::~AudioPlayer()
{
    stop();
}

void AudioPlayer::play(const QString &filePath)
{
    if (isPlaying()) {
        stop();
    }
    
    mediaPlayer->setSource(QUrl::fromLocalFile(filePath));
    mediaPlayer->play();
}

void AudioPlayer::stop()
{
    mediaPlayer->stop();
}

bool AudioPlayer::isPlaying() const
{
    return mediaPlayer->playbackState() == QMediaPlayer::PlayingState;
}

int AudioPlayer::duration() const
{
    return mediaPlayer->duration();
}

int AudioPlayer::position() const
{
    return mediaPlayer->position();
}

void AudioPlayer::handlePlaybackStateChanged(QMediaPlayer::PlaybackState state)
{
    if (state == QMediaPlayer::PlayingState) {
        emit playbackStarted();
    } else if (state == QMediaPlayer::StoppedState || 
               state == QMediaPlayer::PausedState) {
        // Check if we reached the end
        if (mediaPlayer->position() >= mediaPlayer->duration() - 100) {
            emit playbackFinished();
        }
    }
}

void AudioPlayer::handleMediaStatusChanged(QMediaPlayer::MediaStatus status)
{
    if (status == QMediaPlayer::EndOfMedia) {
        emit playbackFinished();
    } else if (status == QMediaPlayer::LoadedMedia || 
               status == QMediaPlayer::BufferedMedia) {
        // Media loaded successfully
    } else if (status == QMediaPlayer::InvalidMedia) {
        emit playbackError(mediaPlayer->errorString());
    }
}
