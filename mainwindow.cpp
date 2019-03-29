#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "qimage.h"
#include "trackio.h"
#include "radiodefinitions.h"

#include <QMediaPlayer>
#include <QMediaService>
#include <QMediaPlaylist>
#include <QDir>
#include <QAudioDecoder>
#include <QCoreApplication>
#include <QFileDialog>
#include <QStandardPaths>
#include <QStringRef>
#include <QMessageBox>
#include <QSignalMapper>

QVector<QVector<Track>> Radio::sorted_Songs(1, QVector<Track>());
QVector<QVector<Track>> Radio::sorted_PSAs(1, QVector<Track>());
QVector<QVector<Track>> Radio::sorted_IDs(1, QVector<Track>());

QVector<RadioEvent_Rule> Radio::eventList_Repeating;
QVector<RadioEvent_Rule_OneShot> Radio::eventList_OneShots;


QMediaPlayer* Radio::player = new QMediaPlayer;
QMediaPlaylist* Radio::playlist = new QMediaPlaylist;

int Radio::priority;

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    implement();

    ui->setupUi(this);


    QListWidget *listWidget_ID = new QListWidget;
    QListWidget *listWidget_PSA = new QListWidget;
    QListWidget *listWidget_Song = new QListWidget;

    ui->scrollArea_IDs->setWidget(listWidget_ID);
    ui->scrollArea_PSAs->setWidget(listWidget_PSA);
    ui->scrollArea_Songs->setWidget(listWidget_Song);

    tIO = new TrackIO(this, listWidget_PSA, listWidget_Song, listWidget_ID);

    //playlist = new QMediaPlaylist();
    //connect(playlist, SIGNAL(currentIndexChanged(int)), this, SLOT(handle_SongChange(int)));
    //Testing button clicks
    connect(ui->pushButton_PSA_Add, SIGNAL(released()), this, SLOT(handleButton_PSA_Add()));
    connect(ui->pushButton_ID_Add, SIGNAL(released()), this, SLOT(handleButton_ID_Add()));
    connect(ui->pushButton_Song_Add, SIGNAL(released()), this, SLOT(handleButton_Song_Add()));

    connect(player, SIGNAL(durationChanged(qint64)), this, SLOT(handle_DurationChanged(qint64)));
    connect(player, SIGNAL(positionChanged(qint64)), this, SLOT(handle_PositionChanged(qint64)));

    connect(ui->pushButton_TempTest, SIGNAL(released()), this, SLOT(handleButton_TestGeneration()));


    QImage albumPlaceholder;
    albumPlaceholder.load(":/rec/Resources/Images/album_art.jpeg");
}

void MainWindow::implement(){
    //Deprecated
}


void MainWindow::handle_DurationChanged(qint64 duration)
{
    {
        ui->progressBar_Duration->setValue((position/1000));
    }

    ui->progressBar_Duration->setMaximum((duration/1000));
}

void MainWindow::handle_PositionChanged(qint64 pos)
{
    position = pos;
}


void MainWindow::handleButton_TestGeneration(){

}


void MainWindow::handleButton_PSA_Add(){
    /*
    //Store all the IDs from the ID folder in a QStringList
    //TODO: Set up dynamic directory creation/use
    QDir directoryIDs("/home/cael/AutoDJ/Audio/IDs");
    QStringList iDTemp = directoryIDs.entryList(QStringList() << "*.mp3" << "*.MP3", QDir::Files);

    qInfo() << "A2AA";
    QList<QUrl> urls = QList<QUrl>();

    //Move IDs into playlist object and playlist widget
     foreach(QString filename, iDTemp) {
         qInfo() << "Z2Z: " << filename;
         urls.append(QUrl::fromLocalFile(filename));
     }*/



    tIO->open(PSA);
}

void MainWindow::handleButton_Event_Add(){

}

void MainWindow::handleCheckbox_Event_Repeat_Daily(int state){
    //state =
        //0 = unchecked = Qt::Unchecked
        //1 = partially checked and in heirarchy of checkboxes = QT::PartiallyChecked
        //2 = checked = QT::Checked

    //if state = Qt::Unchecked
        //.enabled = true to all the days of the week radioBox options.
        //repeatWeekly checkbox .enabled = true
    //if state = Qt::Checked
        //.enabled = false to all the days of the week radioBox options.
        //repeatWeekly checkbox .enabled = false

    if (state == Qt::Unchecked)
    {

    }


}

void MainWindow::handleCheckbox_Event_Repeat_Weekly(int state){
    //state =
        //0 = unchecked = Qt::Unchecked
        //1 = partially checked and in heirarchy of checkboxes = QT::PartiallyChecked
        //2 = checked = QT::Checked

    //if state = Qt::Unchecked
        //.enabled = false to all the days of the week radioBox options.
        //repeatDaily checkbox .enabled = true
    //if state = Qt::Checked
        //.enabled = true to all the days of the week radioBox options.
        //repeatDaily checkbox .enabled = false
}

void MainWindow::handleButton_ID_Add(){
    tIO->open(ID);
}

void MainWindow::handleButton_Song_Add(){
    tIO->open(SONG);
}

/*void MainWindow::fillLists()
{

}*/

/*void MainWindow::addToList(TrackType type, QUrl url)
{
    QList<QUrl> temp = QList<QUrl>() << url;
    addToList(type, temp);
}*/





MainWindow::~MainWindow(){
    delete ui;
}

