#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QList>
#include <QMediaPlayer>
#include <QMediaPlaylist>
#include <QProcess>

#include "deselectableqlistwidget.h"

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

    void implement();

public slots:
private:
    TrackIO *tIO;
    EventHandler *eventHandler;
    PlaylistGenerator *playlistGenerator;

    QMediaPlayer* player;
    QMediaPlaylist* playlist;

    qint64 position = 0;
    shared_ptr<Track> eventToAdd_Track = nullptr;
    std::unique_ptr<QList<RadioEvent_Instance>> testEventList;

    QList<RadioEvent_Rule_OneShot> oneShots;
    QList<RadioEvent_Rule> repeaters;

    QTime playlistEndTime;

    //Create listwidgets to go in appropriate scroll areas and pass them through to the TrackIO object.
    QListWidget *listWidget_ID = new DeselectableQListWidget;
    QListWidget *listWidget_PSA = new DeselectableQListWidget;
    QListWidget *listWidget_Song = new DeselectableQListWidget;

    //listWidget_ID


    QListWidget *listWidget_Upcoming;

    QTime durationTime;

    void clearEventOptions();

    void listWidgetSet(QListWidget *l, QPushButton *b);


private slots:
    void handleButton_TestSaving();
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

    void handle_SongChange(int position); //If after 23:30, generate the next day's playlist.

    void handle_DurationChanged(qint64 duration);
    void handle_PositionChanged(qint64 pos);

    void handleButton_TestPlaylistGeneration();

    void handle_PlayerError(QMediaPlayer::Error error);
signals:

};

#endif // MAINWINDOW_H
