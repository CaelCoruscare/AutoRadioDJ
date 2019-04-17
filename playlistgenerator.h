#ifndef PLAYLISTGENERATOR_H
#define PLAYLISTGENERATOR_H

#include <QMainWindow>
#include <QList>

#include "radiodefinitions.h"
class MainWindow;
#include "mainwindow.h"

using namespace Radio;

class PlaylistGenerator
{
public:
    PlaylistGenerator(QListWidget *lW, QMediaPlaylist *p);
    QTime generateDaySonglist(QDate day, QTime previousDayPlaylist_EndsAt, std::unique_ptr<QList<RadioEvent_Instance> > eventList);
    Track getTrack(int maxLength, TrackType type = SONG);

private:
    QListWidget *listWidget;
    QMediaPlaylist *playlist;

    struct SongOption {
        shared_ptr<Track> track;
        int differenceFromAvg;
        SongOption()
        {}
        SongOption(shared_ptr<Track> track, int differenceFromAvg)
            : track(track)
            , differenceFromAvg(differenceFromAvg)
        {}
    };

public slots:
};

#endif // PLAYLISTGENERATOR_H
