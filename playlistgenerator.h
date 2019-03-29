#ifndef PLAYLISTGENERATOR_H
#define PLAYLISTGENERATOR_H

#include <QMainWindow>
#include <QList>
#include "mainwindow.h"

class PlaylistGenerator
{
public:
    PlaylistGenerator();
    //Each QList has a series of QList<track> that are sorted by how many times they have been played.


    //When selecting an ID, simply choose the least played appropriate track.
    //When selecting a song or PSA, choose the least played track that would not end more than a minute after the next scheduled PSA, ID, or Event.

    //When selecting a track, if the remaining time afterwards before an Event would be between 1 minute and negative one minute, put the Event next.
    //When selecting a song, if the remaining time afterwards before an ID, PSA, or Event would be between 1 minute and negative one minute, put the ID, PSA, or Event next.

    //After selecting a track to put in the playlist, move it to the next QList<track>
    //If the track was in the first QList<track, and the first QList<track> is now empty, it should be removed.





    QTime generateDaySonglist(QDate day, QTime previousDayPlaylist_EndsAt, std::unique_ptr<QList<RadioEvent_Instance> > eventList);
    Track getTrack(int maxLength, TrackType type = SONG);
public slots:
};

#endif // PLAYLISTGENERATOR_H
