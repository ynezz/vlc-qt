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

#ifndef VLCQT_QMLVIDEOPLAYER_H_
#define VLCQT_QMLVIDEOPLAYER_H_

#include <QtCore/QMutex>
#include <QtCore/QTimer>
#include <QtDeclarative/QDeclarativeItem>

#include "Enums.h"
#include "SharedExport.h"

class VlcAudio;
class VlcInstance;
class VlcMedia;
class VlcMediaPlayer;

/* necessary workaround for bug? in moc */
using namespace Vlc;

class VLCQT_EXPORT VlcQMLVideoPlayer : public QDeclarativeItem
{
Q_OBJECT
public:
	explicit VlcQMLVideoPlayer(QDeclarativeItem *parent = 0);
	~VlcQMLVideoPlayer();

	Q_ENUMS(State);

	Q_PROPERTY(State state READ state NOTIFY stateChanged);
	Q_PROPERTY(QString source READ source WRITE setSource NOTIFY sourceChanged);
	Q_PROPERTY(bool playing READ playing WRITE setPlaying NOTIFY playingChanged);
	Q_PROPERTY(bool paused READ paused WRITE setPaused NOTIFY pausedChanged);

	QImage *_frame;
	QMutex _mutex;

	State state() const;
	QString source() const;
	bool playing() const;
	bool paused() const;

protected:
	void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget = 0);
	void geometryChanged(const QRectF &newGeometry, const QRectF &oldGeometry);

signals:
	void stateChanged();
	void sourceChanged();
	void playingChanged();
	void pausedChanged();

public slots:
	void setSource(const QString &source);
	void setPlaying(const bool &p) { p ? play() : stop(); }
	void setPaused(bool p) { p ? pause() : resume(); }
	void pause();
	void resume();
	void play();
	void stop();

private slots:
	void updateFrame();
	void playerStateChanged();

private:
	void openInternal();

	VlcInstance *_instance;
	VlcMediaPlayer *_player;
	VlcMedia *_media;

	VlcAudio *_audioManager;

	bool _playing;
	bool _paused;
	bool _hasMedia;
	int _currentTime;
	QTimer *_timer;
};

#endif // VLCQT_QMLVIDEOPLAYER_H_
