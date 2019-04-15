#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QList>
#include <QMediaPlayer>
#include <QMediaPlaylist>
#include <QProcess>

#include <radiodefinitions.h>
class TrackIO;
#include "trackio.h"
class EventHandler;
#include "eventhandler.h"
class PlaylistGenerator;
#include "playlistgenerator.h"

using namespace Radio;

namespace Ui {
class MainWindow;
}



class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

    Ui::MainWindow *ui;

    //QListWidget *listWidget_PSA, *listWidget_Song, *listWidget_ID;
    TrackIO *tIO;
    EventHandler *eventHandler;
    PlaylistGenerator *playlistGenerator;

    //Create listwidgets to go in appropriate scroll areas and pass them through to the TrackIO object.
    QListWidget *listWidget_ID = new QListWidget;
    QListWidget *listWidget_PSA = new QListWidget;
    QListWidget *listWidget_Song = new QListWidget;


    //Player Stuff
//    QMediaPlayer *player = new QMediaPlayer;
//    QMediaPlaylist *playlist;
    QMediaPlaylist *nextPlaylist;

    void implement();

private:
    qint64 position = 0;
    shared_ptr<Track> eventToAdd_Track = nullptr;
    std::unique_ptr<QList<RadioEvent_Instance>> testEventList;

    QList<RadioEvent_Rule_OneShot> oneShots;
    QList<RadioEvent_Rule> repeaters;


private slots:
    void handleButton_BrowseForEventFile();

    void handleButton_PSA_Add();
    void handleButton_ID_Add();
    void handleButton_Song_Add();

    void handleButton_PSA_Delete();
    void handleButton_ID_Delete();
    void handleButton_Song_Delete();

    void handleButton_Event_Add();
    void handleSelected_Event_Repeater(int currentRow);
    void handleSelected_Event_Oneshot(int currentRow);

    void handleCheckbox_Event_Repeat_Daily(int state);
    void handleCheckbox_Event_Repeat_Weekly(int state);

    //void handle_SongChange(int);
    void handle_DurationChanged(qint64 duration);
    void handle_PositionChanged(qint64 pos);

    void handleButton_TestEventListGeneration();
    void handleButton_TestPlaylistGeneration();

signals:

};

#endif // MAINWINDOW_H
