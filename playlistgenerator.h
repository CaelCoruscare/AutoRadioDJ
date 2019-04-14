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
    QListWidget *listWidget;

    PlaylistGenerator(QListWidget *lW);
    QTime generateDaySonglist(QDate day, QTime previousDayPlaylist_EndsAt, std::unique_ptr<QList<RadioEvent_Instance> > eventList);
    Track getTrack(int maxLength, TrackType type = SONG);
public slots:
};

#endif // PLAYLISTGENERATOR_H
