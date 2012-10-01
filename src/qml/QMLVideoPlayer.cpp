/****************************************************************************
* VLC-Qt - Qt and libvlc connector library
* Copyright (C) 2012 Tadej Novak <tadej@tano.si>
*
* This library is free software: you can redistribute it and/or modify
* it under the terms of the GNU Lesser General Public License as published
* by the Free Software Foundation, either version 3 of the License, or
* (at your option) any later version.
*
* This library is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
* GNU Lesser General Public License for more details.
*
* You should have received a copy of the GNU Lesser General Public License
* along with this library. If not, see <http://www.gnu.org/licenses/>.
*****************************************************************************/

#include <QtCore/QDebug>
#include <QtGui/QKeyEvent>
#include <QtGui/QPainter>

#include <vlc/vlc.h>

#include "core/Audio.h"
#include "core/Common.h"
#include "core/Instance.h"
#include "core/Media.h"
#include "core/MediaPlayer.h"
#include "qml/QMLVideoPlayer.h"

// VLC display functions
static void display(void *core, void *picture)
{
    (void) core;

    Q_ASSERT(picture == NULL);
}

static void *lock(void *core, void **planes)
{
    VlcQMLVideoPlayer *player = (VlcQMLVideoPlayer *)core;
    player->_mutex.lock();
    *planes = player->_frame->bits();
    return NULL;
}

static void unlock(void *core, void *picture, void *const *planes)
{
    Q_UNUSED(planes)

    VlcQMLVideoPlayer *player = (VlcQMLVideoPlayer *)core;
    player->_mutex.unlock();

    Q_ASSERT(picture == NULL);
}

VlcQMLVideoPlayer::VlcQMLVideoPlayer(QDeclarativeItem *parent)
    : QDeclarativeItem(parent),
      _hasMedia(false),
      _playing(false),
      _paused(false),
      _position(0),
      _duration(0)
{
    setFlag(QGraphicsItem::ItemHasNoContents,false);
    setFlag(QGraphicsItem::ItemIsFocusable,true);

    _instance = new VlcInstance(VlcCommon::args(), this);
    _player = new VlcMediaPlayer(_instance);
    _audioManager = new VlcAudio(_player);

    _frame = new QImage(800, 600, QImage::Format_ARGB32_Premultiplied);

    _timer = new QTimer(this);
    connect(_timer,SIGNAL(timeout()),this,SLOT(updateFrame()));
    _timer->start(40); // FPS: 25

    _updateTimer = new QTimer(this);
    connect(_updateTimer,SIGNAL(timeout()),this,SLOT(updateTimerFired()));
    _updateTimer->start(100);

    connect(_player,SIGNAL(stateChanged()),this,SLOT(playerStateChanged()));
}

VlcQMLVideoPlayer::~VlcQMLVideoPlayer()
{
    _player->stop();

    delete _frame;
    delete _timer;

    delete _audioManager;
    delete _media;
    delete _player;
    delete _instance;
}

void VlcQMLVideoPlayer::geometryChanged(const QRectF &newGeometry, const QRectF &oldGeometry)
{
    if (newGeometry == oldGeometry)    {
        QDeclarativeItem::geometryChanged(newGeometry, oldGeometry);
        return;
    }

    _mutex.lock();

    delete _frame;
    _frame = new QImage(newGeometry.width(), newGeometry.height(), QImage::Format_ARGB32_Premultiplied);

    if (_hasMedia) {
        libvlc_video_set_callbacks(_player->core(), lock, unlock, display, this);
        libvlc_video_set_format(_player->core(), "RV32", _frame->width(), _frame->height(), _frame->width() * 4);
    }

    _mutex.unlock();

    QDeclarativeItem::geometryChanged(newGeometry, oldGeometry);

}

void VlcQMLVideoPlayer::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    Q_UNUSED(option)
    Q_UNUSED(widget)

    _mutex.lock();
    painter->drawImage(0, 0, *_frame);
    _mutex.unlock();
}

void VlcQMLVideoPlayer::setSource(const QString &source)
{
    if (_media) {
        if (_media->currentLocation() == source)
            return;

        delete _media;
    }

    if (source.contains(QString("://"))) {
        _media = new VlcMedia(source, false, _instance);
    } else {
        _media = new VlcMedia(source, true, _instance);
    }

    emit sourceChanged();
    openInternal();
}

void VlcQMLVideoPlayer::openInternal()
{
    _player->openOnly(_media);

    libvlc_video_set_callbacks(_player->core(), lock, unlock, display, this);
    libvlc_video_set_format(_player->core(), "RV32", _frame->width(), _frame->height(), _frame->width() * 4);

    _hasMedia = true;
}

void VlcQMLVideoPlayer::pause()
{
    _player->pause();
}

void VlcQMLVideoPlayer::play()
{
    _player->play();
}

void VlcQMLVideoPlayer::stop()
{
    _player->stop();
}

void VlcQMLVideoPlayer::resume()
{
    _playing = true;
    _paused = false;
    _player->resume();
}

void VlcQMLVideoPlayer::updateFrame()
{
    //Fix for cpu usage of calling update inside unlock.
    update(boundingRect());
}

VlcQMLVideoPlayer::State VlcQMLVideoPlayer::state() const
{
    return (State) _player->state();
}

void VlcQMLVideoPlayer::playerStateChanged()
{
     emit stateChanged();

     switch (state()) {
         case Vlc::Playing:
             if (!_playing) {
                 _playing = true;
                 _paused = false;
                 emit playingChanged();
             }
         break;
         case Vlc::Stopped:
             if (_playing) {
                 _playing = false;
                 emit playingChanged();
             }
         break;
         case Vlc::Paused:
             if (!_paused) {
                 _paused = true;
                 emit pausedChanged();
             }
         break;
     }
}

QString VlcQMLVideoPlayer::source() const
{
    if (!_media)
        return QString();

    return _media->currentLocation();
}

bool VlcQMLVideoPlayer::playing() const
{
    return _playing;
}

bool VlcQMLVideoPlayer::paused() const
{
    return _paused;
}

int VlcQMLVideoPlayer::volume() const
{
    return _player->audio()->volume();
}

int VlcQMLVideoPlayer::duration() const
{
    return _player->length();
}

int VlcQMLVideoPlayer::position() const
{
    return _player->position() * _duration;
}

void VlcQMLVideoPlayer::setVolume(int vol)
{
    if (_volume == vol)
        return;

    _volume = vol;
    _player->audio()->setVolume(vol);
}

void VlcQMLVideoPlayer::setPosition(int pos)
{
    if (!_media)
        return;

    if (_duration == 0)
        return;

    _player->setPosition((qreal)pos/_duration);
}

void VlcQMLVideoPlayer::updateTimerFired()
{
    if (!_media || !_player)
        return;

    int v = _player->audio()->volume();
    if (_volume != v) {
        _volume = v;
        emit volumeChanged();
    }

    float p = _player->position();
    if (_position != p) {
        _position = p;
        emit positionChanged();
    }

    int time = _player->length();
    if (_duration != time) {
        _duration = time;
        emit durationChanged();
    }
}