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
QVector<QVector<Track>> Radio::sorted_IDs(25, QVector<Track>());

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


    //Create listwidgets to go in appropriate scroll areas and pass them through to the TrackIO object.
    QListWidget *listWidget_ID = new QListWidget;
    QListWidget *listWidget_PSA = new QListWidget;
    QListWidget *listWidget_Song = new QListWidget;

    ui->scrollArea_IDs->setWidget(listWidget_ID);
    ui->scrollArea_PSAs->setWidget(listWidget_PSA);
    ui->scrollArea_Songs->setWidget(listWidget_Song);

    //Create listWidget to display Events and pass it through to the EventHandler object.
    QListWidget *listWidget_Event = new QListWidget;

    ui->scrollArea_Events->setWidget(listWidget_Event);

    //
    QListWidget *listWidget_Upcoming = new QListWidget;

    ui->scrollArea_Upcoming->setWidget(listWidget_Upcoming);

    //Create the TrackIO, EventHandler, and PlaylistGenerator objects, and hand them their listwidgets.
    tIO = new TrackIO(this, listWidget_PSA, listWidget_Song, listWidget_ID, listWidget_Event);
    eventHandler = new EventHandler(listWidget_Event);
    playlistGenerator = new PlaylistGenerator(listWidget_Upcoming);

    //playlist = new QMediaPlaylist();
    //connect(playlist, SIGNAL(currentIndexChanged(int)), this, SLOT(handle_SongChange(int)));
    //Testing button clicks
    connect(ui->pushButton_PSA_Add, SIGNAL(released()), this, SLOT(handleButton_PSA_Add()));
    connect(ui->pushButton_ID_Add, SIGNAL(released()), this, SLOT(handleButton_ID_Add()));
    connect(ui->pushButton_Song_Add, SIGNAL(released()), this, SLOT(handleButton_Song_Add()));

    connect(player, SIGNAL(durationChanged(qint64)), this, SLOT(handle_DurationChanged(qint64)));
    connect(player, SIGNAL(positionChanged(qint64)), this, SLOT(handle_PositionChanged(qint64)));

    connect(ui->pushButton_EventTest, SIGNAL(released()), this, SLOT(handleButton_TestEventListGeneration()));
    connect(ui->pushButton_PlaylistTest, SIGNAL(released()), this, SLOT(handleButton_TestPlaylistGeneration()));

    connect(ui->pushButton_Event_FileBrowse, SIGNAL(released()), this, SLOT(handleButton_BrowseForEventFile()));
    connect(ui->pushButton_AddEvent, SIGNAL(released()), this, SLOT(handleButton_Event_Add()));

    connect(ui->checkBox_RepeatWeekly, SIGNAL(stateChanged(int)), this, SLOT(handleCheckbox_Event_Repeat_Weekly(int)));
    connect(ui->checkBox_RepeatDaily, SIGNAL(stateChanged(int)), this, SLOT(handleCheckbox_Event_Repeat_Daily(int)));


    QImage albumPlaceholder;
    albumPlaceholder.load(":/rec/Resources/Images/album_art.jpeg");
}

void MainWindow::implement(){
    //Deprecated
}

void MainWindow::handleButton_Event_Add()
{
    QDate lastDate = QDate(3000,1,1);

    if(ui->checkBox_RepeatDaily->checkState() != Qt::Checked && ui->checkBox_RepeatWeekly->checkState() != Qt::Checked)
    {
        //Get the datetime from the date and time edits.
        QDateTime dateTime = ui->dateEdit_Event->dateTime();
        dateTime.setTime(ui->timeEdit_Event->time());

        //Create the Event
        eventHandler->addEventRule(eventToAdd_Track, dateTime);
    } else {
        //First Date
        QDate firstDate = ui->dateEdit_Event->date();
        //Last Date
        if (ui->checkBox_EventEnd->checkState() == Qt::Checked)
            lastDate = ui->dateEdit_EventEnd->date();
        //Start Time
        QList<QTime> *times = new QList<QTime>();
        times->append(QTime(ui->timeEdit_Event->time()));
        //Days of week
        QBitArray daysOfWeek = QBitArray(7, true);
        if (ui->checkBox_RepeatWeekly->checkState() == Qt::Checked)
        {
            if(!ui->radioButton_Sun->isChecked())
                daysOfWeek.clearBit(0);
            if(ui->radioButton_Mon->isChecked())
                daysOfWeek.clearBit(1);
            if(ui->radioButton_Tue->isChecked())
                daysOfWeek.clearBit(2);
            if(ui->radioButton_Wed->isChecked())
                daysOfWeek.clearBit(3);
            if(ui->radioButton_Thu->isChecked())
                daysOfWeek.clearBit(4);
            if(ui->radioButton_Fri->isChecked())
                daysOfWeek.clearBit(5);
            if(ui->radioButton_Sat->isChecked())
                daysOfWeek.clearBit(6);
        }

        //Create the event
        eventHandler->addEventRule(eventToAdd_Track, *times, firstDate, lastDate, daysOfWeek);
    }

    //addEventRule(shared_ptr<Track> track, QDateTime dateTime)
    //addEventRule(shared_ptr<Track> track, QList<QTime> times, QDate firstDate, QDate lastDate, QBitArray daysOfTheWeek)
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

void MainWindow::handleButton_TestEventListGeneration(){
    testEventList = eventHandler->generate_DailyEventSchedule(QDate::currentDate());
}

void MainWindow::handleButton_TestPlaylistGeneration(){
    qInfo() << "Yes Sir!";
    playlistGenerator->generateDaySonglist(QDate::currentDate(), QTime(0,3),eventHandler->generate_DailyEventSchedule(QDate::currentDate()));
}

void MainWindow::handleButton_BrowseForEventFile(){

    QUrl url = tIO->open().first();

    ui->lineEdit_EventFilename->setText(url.fileName());

    //Set-up for fetching the length of each song.
    QStringList arguments;
    arguments << "--Inform=Audio;%Duration%";
    QProcess lengthGetter;

    //Set up to read length
    arguments << url.path();
    qInfo() << "arguments: " << arguments;
    qInfo() << "path: " << url.path();
    //Read the length of the music file
    lengthGetter.start("mediainfo", arguments);
    lengthGetter.waitForFinished(); // sets current thread to sleep and waits for pingProcess end
    QString output(lengthGetter.readAllStandardOutput());
    qInfo() << "Length: " << output << " | " << url.path();


    //Fill the track struct.
    eventToAdd_Track = std::make_shared<Track>(url, output.toLongLong());
    qInfo() << "eventToAdd_Track: " << eventToAdd_Track->path << " " << eventToAdd_Track->length;


}

void MainWindow::handleButton_PSA_Add(){

    tIO->addToList(PSA, tIO->open());
}

void MainWindow::handleButton_ID_Add(){
    tIO->addToList(ID, tIO->open());
}

void MainWindow::handleButton_Song_Add(){
    tIO->addToList(SONG, tIO->open());
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

    if (state == Qt::Checked)
    {
        ui->radioButton_Sun->setEnabled(false);
        ui->radioButton_Mon->setEnabled(false);
        ui->radioButton_Tue->setEnabled(false);
        ui->radioButton_Wed->setEnabled(false);
        ui->radioButton_Thu->setEnabled(false);
        ui->radioButton_Fri->setEnabled(false);
        ui->radioButton_Sat->setEnabled(false);

        ui->checkBox_RepeatWeekly->setChecked(false);
    }


}

void MainWindow::handleCheckbox_Event_Repeat_Weekly(int state){
    //state =
        //0 = unchecked = Qt::Unchecked
        //1 = partially checked and in heirarchy of checkboxes = QT::PartiallyChecked
        //2 = checked = QT::Checked

    if (state == Qt::Checked)
    {
        ui->radioButton_Sun->setEnabled(true);
        ui->radioButton_Mon->setEnabled(true);
        ui->radioButton_Tue->setEnabled(true);
        ui->radioButton_Wed->setEnabled(true);
        ui->radioButton_Thu->setEnabled(true);
        ui->radioButton_Fri->setEnabled(true);
        ui->radioButton_Sat->setEnabled(true);

        ui->checkBox_RepeatDaily->setChecked(false);
    } else {
        ui->radioButton_Sun->setEnabled(false);
        ui->radioButton_Mon->setEnabled(false);
        ui->radioButton_Tue->setEnabled(false);
        ui->radioButton_Wed->setEnabled(false);
        ui->radioButton_Thu->setEnabled(false);
        ui->radioButton_Fri->setEnabled(false);
        ui->radioButton_Sat->setEnabled(false);
    }
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

