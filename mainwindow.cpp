#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "qimage.h"
#include "trackio.h"
#include "radiodefinitions.h"
#include "deselectableqlistwidget.h"

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

int Radio::priority;

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    implement();

    ui->setupUi(this);
    clearEventOptions();

    player = new QMediaPlayer(this);
    playlist = new QMediaPlaylist(player);

    player->setPlaylist(playlist);
    player->setVolume(70);
    player->setAudioRole(QAudio::MusicRole);
    playlist->setPlaybackMode(QMediaPlaylist::Loop);

    listWidget_ID->setSelectionMode(QAbstractItemView::ExtendedSelection);
    listWidget_PSA->setSelectionMode(QAbstractItemView::ExtendedSelection);
    listWidget_Song->setSelectionMode(QAbstractItemView::ExtendedSelection);

    ui->scrollArea_IDs->setWidget(listWidget_ID);
    ui->scrollArea_PSAs->setWidget(listWidget_PSA);

    ui->scrollArea_Songs->setWidget(listWidget_Song);

    //Create listWidget to display Events and pass it through to the EventHandler object.
    QListWidget *listWidget_Event_Repeating = new DeselectableQListWidget;
    QListWidget *listWidget_Event_Oneshot = new DeselectableQListWidget;

    ui->scrollArea_Events_OneShots->setWidget(listWidget_Event_Oneshot);
    ui->scrollArea_Events_Repeating->setWidget(listWidget_Event_Repeating);

    listWidget_Upcoming = new QListWidget;

    ui->scrollArea_Upcoming->setWidget(listWidget_Upcoming);

    //Create the TrackIO, EventHandler, and PlaylistGenerator objects, and hand them their listwidgets.
    tIO = new TrackIO(this, listWidget_PSA, listWidget_Song, listWidget_ID, player);
    eventHandler = new EventHandler(listWidget_Event_Repeating, listWidget_Event_Oneshot);
    playlistGenerator = new PlaylistGenerator(listWidget_Upcoming, playlist);

    //playlist = new QMediaPlaylist();
    //connect(playlist, SIGNAL(currentIndexChanged(int)), this, SLOT(handle_SongChange(int)));
    //Testing button clicks
    connect(ui->pushButton_PSA_Add, SIGNAL(released()), this, SLOT(handleButton_PSA_Add()));
    connect(ui->pushButton_ID_Add, SIGNAL(released()), this, SLOT(handleButton_ID_Add()));
    connect(ui->pushButton_Song_Add, SIGNAL(released()), this, SLOT(handleButton_Song_Add()));

    connect(ui->pushButton_PSA_Delete, SIGNAL(released()), this, SLOT(handleButton_PSA_Delete()));
    connect(ui->pushButton_ID_Delete, SIGNAL(released()), this, SLOT(handleButton_ID_Delete()));
    connect(ui->pushButton_Song_Delete, SIGNAL(released()), this, SLOT(handleButton_Song_Delete()));

    connect(player, SIGNAL(durationChanged(qint64)), this, SLOT(handle_DurationChanged(qint64)));
    connect(player, SIGNAL(positionChanged(qint64)), this, SLOT(handle_PositionChanged(qint64)));

    connect(ui->pushButton_PlaylistTest, SIGNAL(released()), this, SLOT(handleButton_TestPlaylistGeneration()));

    connect(ui->pushButton_Event_FileBrowse, SIGNAL(released()), this, SLOT(handleButton_BrowseForEventFile()));
    connect(ui->pushButton_AddEvent, SIGNAL(released()), this, SLOT(handleButton_Event_Add()));

    connect(ui->checkBox_RepeatWeekly, SIGNAL(stateChanged(int)), this, SLOT(handleCheckbox_Event_Repeat_Weekly(int)));
    connect(ui->checkBox_RepeatDaily, SIGNAL(stateChanged(int)), this, SLOT(handleCheckbox_Event_Repeat_Daily(int)));

    connect(listWidget_Event_Repeating, SIGNAL(currentRowChanged(int)), this, SLOT(handleSelected_Event_Repeater(int)));
    connect(listWidget_Event_Oneshot, SIGNAL(currentRowChanged(int)), this, SLOT(handleSelected_Event_Oneshot(int)));

    connect(playlist, SIGNAL(currentIndexChanged(int)), this, SLOT(handle_SongChange(int)));
}

void MainWindow::implement(){
    //Deprecated
}

void MainWindow::handleButton_Event_Add(){

    QDate lastDate = QDate(3000,1,1);

    if(ui->checkBox_RepeatDaily->checkState() != Qt::Checked && ui->checkBox_RepeatWeekly->checkState() != Qt::Checked)
    {
        //Get the datetime from the date and time edits.
        QDateTime dateTime = ui->dateEdit_Event->dateTime();
        dateTime.setTime(ui->timeEdit_Event->time());

        //Create the Event
        eventHandler->addEventRule(eventToAdd_Track, dateTime);
        oneShots.append(eventList_OneShots.last());
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
            if(!ui->radioButton_Mon->isChecked())
                daysOfWeek.clearBit(1);
            if(!ui->radioButton_Tue->isChecked())
                daysOfWeek.clearBit(2);
            if(!ui->radioButton_Wed->isChecked())
                daysOfWeek.clearBit(3);
            if(!ui->radioButton_Thu->isChecked())
                daysOfWeek.clearBit(4);
            if(!ui->radioButton_Fri->isChecked())
                daysOfWeek.clearBit(5);
            if(!ui->radioButton_Sat->isChecked())
                daysOfWeek.clearBit(6);
        }

        //Create the event
        eventHandler->addEventRule(eventToAdd_Track, *times, firstDate, lastDate, daysOfWeek);
        repeaters.append(eventList_Repeating.last());
    }

    //addEventRule(shared_ptr<Track> track, QDateTime dateTime)
    //addEventRule(shared_ptr<Track> track, QList<QTime> times, QDate firstDate, QDate lastDate, QBitArray daysOfTheWeek)
}

void MainWindow::handle_DurationChanged(qint64 duration){

    ui->progressBar_Duration->setMaximum(static_cast<int>(duration));
    durationTime = QTime(0,0).addMSecs(static_cast<int>(duration));
}

void MainWindow::handle_PositionChanged(qint64 pos){

    ui->progressBar_Duration->setValue(static_cast<int>(pos));
    //auto prog = QTime(0,0,0,pos);
    ui->progressBar_Duration->setFormat(QTime(0,0).addMSecs(static_cast<int>(pos)).toString("m:ss") + " / " + durationTime.toString("m:ss"));
}

void MainWindow::handleButton_TestPlaylistGeneration(){

    qInfo() << "Yes Sir! player is: " << player;
    playlistEndTime = playlistGenerator->generateDaySonglist(QDate::currentDate(), QDateTime::currentDateTime().time(), eventHandler->generate_DailyEventSchedule(QDate::currentDate()));
    player->play();    
}

void MainWindow::handleButton_BrowseForEventFile(){

    //Fetch the file with a file dialogue.
    QList<QUrl> eventFiles = tIO->open();
    //Cancel if no file was selected.
    if (eventFiles.empty())
        return;
    //Ignore all but the first if multiple were selected.
    QUrl url = eventFiles.first();

    ui->lineEdit_EventFilename->setText(url.fileName());

    //Set-up for fetching the length of each song.
    QStringList arguments;
    arguments << "--Inform=Audio;%Duration%";
    QProcess lengthGetter;

    //Set up to read length
    arguments << url.path();
    //Read the length of the music file
    lengthGetter.start("mediainfo", arguments);
    lengthGetter.waitForFinished(); // sets current thread to sleep and waits for pingProcess end
    QString output(lengthGetter.readAllStandardOutput());


    //Fill the track struct.
    eventToAdd_Track = std::make_shared<Track>(url, output.toLongLong());
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

void MainWindow::handleButton_PSA_Delete(){

    QList<QListWidgetItem *> items = listWidget_PSA->selectedItems();

    for (auto item: items)
    {
        //Search sorted_PSAs until a matching file is found.
        bool found = false;
        for (auto &list: sorted_PSAs)
        {
            for (int i = 0; i < list.length(); i++)
            {
                if (!item->text().compare(list.at(i).path.fileName()))
                {
                    list.removeAt(i);
                    found = true;
                    break;
                }
            }

            if (found)
                break;
        }
    }

    qDeleteAll(listWidget_PSA->selectedItems());
}

void MainWindow::handleButton_ID_Delete(){

    QList<QListWidgetItem *> items = listWidget_ID->selectedItems();

    for (auto item: items)
    {
        //Search sorted_IDs until a matching file is found.
        bool found = false;
        for (auto list: sorted_IDs)
        {
            for (int i = 0; i < list.length(); i++)
            {
                if (!item->text().compare(list.at(i).path.fileName()))
                {
                    list.removeAt(i);
                    found = true;
                    break;
                }
            }

            if (found)
                break;
        }
    }

    qDeleteAll(listWidget_ID->selectedItems());
}

void MainWindow::handleButton_Song_Delete(){

    QList<QListWidgetItem *> items = listWidget_Song->selectedItems();

    for (auto item: items)
    {
        //Search sorted_Songs until a matching file is found.
        bool found = false;
        for (auto list: sorted_Songs)
        {
            for (int i = 0; i < list.length(); i++)
            {
                if (!item->text().compare(list.at(i).path.fileName()))
                {
                    list.removeAt(i);
                    found = true;
                    break;
                }
            }

            if (found)
                break;
        }
    }

    qDeleteAll(listWidget_Song->selectedItems());
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

void MainWindow::clearEventOptions(){

    //File
    ui->lineEdit_EventFilename->setText("");

    //First Date
    ui->dateEdit_Event->setDate(QDateTime::currentDateTime().date());

    //Last Date
    ui->checkBox_EventEnd->setCheckState(Qt::Unchecked);
    ui->dateEdit_EventEnd->setDate(QDateTime::currentDateTime().date());

    //Start Time
    ui->timeEdit_Event->setTime(QTime(14, 0));

    //Days of week
    ui->checkBox_RepeatDaily->setCheckState(Qt::Unchecked);
    ui->checkBox_RepeatWeekly->setCheckState(Qt::Unchecked);

    ui->radioButton_Sun->setChecked(false);
    ui->radioButton_Mon->setChecked(false);
    ui->radioButton_Tue->setChecked(false);
    ui->radioButton_Wed->setChecked(false);
    ui->radioButton_Thu->setChecked(false);
    ui->radioButton_Fri->setChecked(false);
    ui->radioButton_Sat->setChecked(false);
}

void MainWindow::handleSelected_Event_Repeater(int currentRow){

    if (currentRow == -1)
        clearEventOptions();
    else {
        //Get the rule for reference.
        RadioEvent_Rule rule = repeaters[currentRow];

        //File
        ui->lineEdit_EventFilename->setText(rule.track->path.fileName());
        eventToAdd_Track = rule.track;

        //First Date
        ui->dateEdit_Event->setDate(rule.firstDate);

        //Last Date
        if(rule.lastDate == QDate(3000,1,1))
        {
            ui->checkBox_EventEnd->setCheckState(Qt::Unchecked);
            ui->dateEdit_EventEnd->setDate(QDateTime::currentDateTime().date());
        }
        else {
            ui->checkBox_EventEnd->setCheckState(Qt::Checked);
            ui->dateEdit_EventEnd->setDate(rule.firstDate);
        }

        //Start Time
        ui->timeEdit_Event->setTime(rule.times.first());

        //Days of week
        if(rule.daysOfTheWeek == QBitArray(7, true))
        {
            ui->checkBox_RepeatDaily->setCheckState(Qt::Checked);
            ui->checkBox_RepeatWeekly->setCheckState(Qt::Unchecked);

            ui->radioButton_Sun->setChecked(false);
            ui->radioButton_Mon->setChecked(false);
            ui->radioButton_Tue->setChecked(false);
            ui->radioButton_Wed->setChecked(false);
            ui->radioButton_Thu->setChecked(false);
            ui->radioButton_Fri->setChecked(false);
            ui->radioButton_Sat->setChecked(false);
        } else {
            ui->checkBox_RepeatDaily->setCheckState(Qt::Unchecked);
            ui->checkBox_RepeatWeekly->setCheckState(Qt::Checked);

            ui->radioButton_Sun->setChecked(rule.daysOfTheWeek.testBit(0));
            ui->radioButton_Mon->setChecked(rule.daysOfTheWeek.testBit(1));
            ui->radioButton_Tue->setChecked(rule.daysOfTheWeek.testBit(2));
            ui->radioButton_Wed->setChecked(rule.daysOfTheWeek.testBit(3));
            ui->radioButton_Thu->setChecked(rule.daysOfTheWeek.testBit(4));
            ui->radioButton_Fri->setChecked(rule.daysOfTheWeek.testBit(5));
            ui->radioButton_Sat->setChecked(rule.daysOfTheWeek.testBit(6));
        }
    }
}

void MainWindow::handleSelected_Event_Oneshot(int currentRow){

    if (currentRow == -1)
        clearEventOptions();
    else{
        //Get the rule for reference.
        RadioEvent_Rule_OneShot rule = oneShots[currentRow];

        //File
        ui->lineEdit_EventFilename->setText(rule.track->path.fileName());
        eventToAdd_Track = rule.track;

        //Date
        ui->dateEdit_Event->setDate(rule.dateTime.date());

        //Time
        ui->timeEdit_Event->setTime(rule.dateTime.time());

        //Clear Repeater stuff
        //Last Date
        ui->checkBox_EventEnd->setCheckState(Qt::Unchecked);
        ui->dateEdit_EventEnd->setDate(QDateTime::currentDateTime().date());

        //Days of week
        ui->checkBox_RepeatDaily->setCheckState(Qt::Unchecked);
        ui->checkBox_RepeatWeekly->setCheckState(Qt::Unchecked);

        ui->radioButton_Sun->setChecked(false);
        ui->radioButton_Mon->setChecked(false);
        ui->radioButton_Tue->setChecked(false);
        ui->radioButton_Wed->setChecked(false);
        ui->radioButton_Thu->setChecked(false);
        ui->radioButton_Fri->setChecked(false);
        ui->radioButton_Sat->setChecked(false);
    }
}

void MainWindow::handle_SongChange(int position){

    qInfo() << "songchange";
    ui->label_TrackTitle->setText("Now Playing:\n" + listWidget_Upcoming->takeItem(0)->text());

    if(QDateTime::currentDateTime().time() > QTime(23,30))
    {
        qInfo() << "Time to generate tomorrow's audio!";
        QDate tomorrow = QDate::currentDate().addDays(1);
        playlistEndTime = playlistGenerator->generateDaySonglist(tomorrow, playlistEndTime, eventHandler->generate_DailyEventSchedule(tomorrow));
    }
}

MainWindow::~MainWindow(){
    delete ui;
}

