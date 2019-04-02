#ifndef TRACKIO_H
#define TRACKIO_H

#include <QMainWindow>
#include <QListWidget>

#include "radiodefinitions.h"
class MainWindow;
#include "mainwindow.h"


using namespace Radio;



class TrackIO
{
public:
    QListWidget *listWidget_PSA, *listWidget_Song, *listWidget_ID;
    TrackIO(MainWindow *mW, QListWidget *lW_PSA, QListWidget *lW_Song, QListWidget *lW_ID, QListWidget *lW_Event);
    MainWindow *mainWindow;
    QList<QUrl> open();
    void addToList(TrackType type, QUrl url);
    void addToList(TrackType type, const QList<QUrl> &urls);
    void fillLists();
    void saveToFile();
    void loadFromFile();

    //When loading an ID, reject if it is more than 59 seconds.
    //When loading a PSA, reject if it is more than 10 minutes.
    //When loading song, reject if it is more than 15 minutes.

private:
};

#endif // TRACKIO_H
