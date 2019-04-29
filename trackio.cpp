#include "trackio.h"
#include "mainwindow.h"
#include <QCoreApplication>
#include <QFileDialog>
#include <QStandardPaths>
#include "radiodefinitions.h"
#include <memory>
#include <QMessageBox>

using namespace std;

TrackIO::TrackIO(MainWindow *mW, QListWidget *lW_PSA, QListWidget *lW_Song, QListWidget *lW_ID, QMediaPlayer *p, QListWidget *lW_EO, QListWidget *lW_ER){

    mainWindow = mW;
    listWidget_PSA = lW_PSA;
    listWidget_Song = lW_Song;
    listWidget_ID = lW_ID;
    listWidget_EventOneshot = lW_EO;
    listWidget_EventRepeating = lW_ER;
    player = p;

    qRegisterMetaTypeStreamOperators<Track>("Track");
    qRegisterMetaTypeStreamOperators<RadioEvent_Rule>("RadioEvent_Rule");
}

//Track Serialization
QDataStream &operator<<(QDataStream& out, const Track& track){

    return out << track.path << track.length;
}

QDataStream &operator>>(QDataStream& in, Track& track){

    return in >> track.path >> track.length;
}

//RadioEventRule Serialization
QDataStream &operator<<(QDataStream& stream, const RadioEvent_Rule& rule){

    return stream << rule.times << *rule.track << rule.lastDate << rule.firstDate << rule.priority << rule.daysOfTheWeek;
}

QDataStream &operator>>(QDataStream& stream, RadioEvent_Rule& rule){

    Track t;
    stream >> rule.times >> t >> rule.lastDate >> rule.firstDate >> rule.priority >> rule.daysOfTheWeek;
    rule.track = make_shared<Track>(t);
    return stream;
}

//RadioEventRule_OneShot Serialization
QDataStream &operator<<(QDataStream& stream, const RadioEvent_Rule_OneShot& rule){

    return stream << rule.dateTime << *rule.track << rule.priority;
}

QDataStream &operator>>(QDataStream& stream, RadioEvent_Rule_OneShot& rule){

    Track t;
    stream >> rule.dateTime >> t >> rule.priority;
    rule.track = make_shared<Track>(t);
    return stream;
}

void TrackIO::saveToFile(){

    //Create/open file.dat
    QFile file(QCoreApplication::applicationDirPath() + "/data/file.dat");
    if (!file.exists())
    {
        QDir dir = QCoreApplication::applicationDirPath();
        dir.mkdir("data");
    }
    file.open(QIODevice::WriteOnly);
    qInfo() << file.exists();
    // we will serialize the data into the file
    QDataStream out(&file);

    out << sorted_IDs;
    out << sorted_PSAs;
    out << sorted_Songs;
    //out << eventList_OneShots;
    //out << eventList_Repeating;

}

void TrackIO::loadFromFile(){

    QFile file(QCoreApplication::applicationDirPath() + "/data/file.dat");

    if (file.exists())
    {
        file.open(QIODevice::ReadOnly);
        QDataStream in(&file);

        QVector< QVector< Track> > test;

        in >> sorted_IDs;
        in >> sorted_PSAs;
        in >> sorted_Songs;

        if (sorted_IDs.count() != 25)
        {
            qInfo() << "ERROR: sorted_IDs was loaded with a number of lists that does not equal 25. Something went terribly wrong!";
            return;
        }

        fillList(sorted_IDs, *listWidget_ID);
        fillList(sorted_PSAs, *listWidget_PSA);
        fillList(sorted_Songs, *listWidget_Song);
    }


}

void TrackIO::fillList(QVector< QVector< Track> > &sorted, QListWidget &widget){

    for(auto &list: sorted)
    {
        for(auto &t: list)
        {
            widget.addItem(t.path.fileName());
        }
    }
}

QList<QUrl> TrackIO::open(){

    QFileDialog fileDialog(mainWindow);
    fileDialog.setFileMode(QFileDialog::ExistingFiles);
    fileDialog.setAcceptMode(QFileDialog::AcceptOpen);
    fileDialog.setWindowTitle("Open Files");
    QStringList supportedMimeTypes = player->supportedMimeTypes();
    if (!supportedMimeTypes.isEmpty()) {
        supportedMimeTypes.append("audio/x-m3u"); // MP3 playlists
        fileDialog.setMimeTypeFilters(supportedMimeTypes);
    }
    fileDialog.setDirectory(QStandardPaths::standardLocations(QStandardPaths::MusicLocation).value(0, QDir::homePath()));
    if (fileDialog.exec() == QDialog::Accepted)
        return fileDialog.selectedUrls();
    else {
        return QList<QUrl>();
    }

}

void TrackIO::addToList(TrackType type, const QList<QUrl> &urls){

    //Cancel if no file was selected.
    if (urls.empty())
        return;

    //Setup for adding the track to the proper track list and listwidget.
    QListWidget *listWidget;
    QVector<Track> *list;

    //Set-up for fetching the length of each song.
    QStringList arguments;
    arguments << "--Inform=Audio;%Duration%";
    QProcess lengthGetter;

    //More setup for adding the track to the proper track list and listwidget.
    switch(type)
    {
        case PSA  : listWidget = listWidget_PSA;
                    if(sorted_PSAs.empty())
                        sorted_PSAs.append(QVector<Track>());
                    list = &sorted_PSAs.first();
                    break;

        case ID  :  listWidget = listWidget_ID;
                    list = &sorted_IDs.last(); //The last list of IDs holds only IDs that aren't specific to a time.
                    break;

        case SONG  :    listWidget = listWidget_Song;
                        if(sorted_Songs.empty())
                            sorted_Songs.append(QVector<Track>());
                        list = &sorted_Songs.first();
                        break;

        case EVENT : qInfo() << "Event should not be pased through here";
                    return;
    }

    for(auto &url: urls)
    {

        //Set up to read length
        arguments << url.path();
        //Read the length of the music file
        lengthGetter.start("mediainfo", arguments);
        lengthGetter.waitForFinished(); // sets current thread to sleep and waits for pingProcess end
        QString output(lengthGetter.readAllStandardOutput());

        //Clear out the filename argument so arguments can be used again in the next loop.
        arguments.pop_back();

        //If length 0 or > 15.30 don't import
        auto length =  output.toInt();
        if(length == 0)
        {
            qInfo() << "For song " << url.fileName() << ", duration is zero. Track will be rejected. Are you sure you have mediainfo installed?";
            QMessageBox msgBox;
            msgBox.setText("For song " + url.fileName() + ", duration is zero. Track will be rejected. Are you sure you have mediainfo installed?");
            msgBox.exec();
            continue;
        }
        if(length > 930000)
        {
            qInfo() << "For song " << url.fileName() <<  ", duration is more than 15.5 minutes. The song will be rejected to maintain viability of having a PSA every 15 minutes.";
            QMessageBox msgBox;
            msgBox.setText("For song " + url.fileName() + ", duration is more than 15.5 minutes. The song will be rejected to maintain viability of having a PSA every 15 minutes.");
            msgBox.exec();
            continue;
        }


        //Fill the track struct and append it to the proper track list.
        auto t = Track(url, length);

        //Handle ID Special Case
        if (type == ID)
        {
            bool isNumber;
            int hour = url.fileName().mid(0,2).toInt(&isNumber);
            if (!isNumber)
            {
                list->append(t);
            }
            else if (hour > 24 || hour < 0)
            {
                list->append(t);
            }
            else {
                sorted_IDs[hour].append(t);
            }
        }
        //Handle Song/PSA
        else {
            list->append(t);
        }

        //Add the filename to the listWidget.
        listWidget->addItem(url.fileName());
    }
}
