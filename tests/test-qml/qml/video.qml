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

import QtQuick 1.1
import VLCQt 0.6

Rectangle {
	id: root
	width: 820
	height: 420
	color: "black"

	VlcVideoPlayer {
		id: videowidget
		anchors.fill: parent

		/*
		source: "d:/big_buck_bunny_1080p_surround.avi"
		source: "http://devimages.apple.com/iphone/samples/bipbop/gear1/prog_index.m3u8"
		source: "mms://live1.wm.skynews.servecast.net/skynews_wmlz_live300k"
		*/

		source: "http://content.bitsontherun.com/videos/nfSyO85Q-52qL9xLP.mp4"

		Component.onCompleted: play()
		MouseArea {
			anchors.fill: parent
			onClicked: {
				if (!videowidget.paused) {
					console.log("pause, media: " + videowidget.source)
					videowidget.pause()
				} else {
					console.log("resume, media: " + videowidget.source)
					videowidget.resume()
				}
			}
		}
	}
}
