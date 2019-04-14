#include "trackio.h"
#include "mainwindow.h"
#include <QFileDialog>
#include <QStandardPaths>
#include "radiodefinitions.h"
#include <memory>

using namespace std;

TrackIO::TrackIO(MainWindow *mW, QListWidget *lW_PSA, QListWidget *lW_Song, QListWidget *lW_ID, QListWidget *lW_Event)
{
    mainWindow = mW;
    listWidget_PSA = lW_PSA;
    listWidget_Song = lW_Song;
    listWidget_ID = lW_ID;

    qRegisterMetaTypeStreamOperators<Track>("Track");
    qRegisterMetaTypeStreamOperators<RadioEvent_Rule>("RadioEvent_Rule");
}

//Track Serialization
QDataStream &operator<<(QDataStream& stream, const Track& track){
    return stream << track.path << track.length;
}

QDataStream &operator>>(QDataStream& stream, Track& track){
    return stream >> track.path >> track.length;
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

QList<QUrl> TrackIO::open()
{

    QFileDialog fileDialog(mainWindow);
    fileDialog.setFileMode(QFileDialog::ExistingFiles);
    fileDialog.setAcceptMode(QFileDialog::AcceptOpen);
    fileDialog.setWindowTitle("Open Files");
    QStringList supportedMimeTypes = Radio::player->supportedMimeTypes();
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

void TrackIO::addToList(TrackType type, const QList<QUrl> &urls)
{
    qInfo() << "hi " << urls[0].url();
    //Setup for adding the track to the proper track list and listwidget.
    QListWidget *listWidget;
    QVector<Track> *list;

    //Set-up for fetching the length of each song.
    QStringList arguments;
    arguments << "--Inform=Audio;%Duration%";
    QProcess lengthGetter;

    qInfo() << "SortedPSAs Length: " << sorted_PSAs.length();

    //More setup for adding the track to the proper track list and listwidget.
    switch(type)
    {
        case PSA  : listWidget = listWidget_PSA;
                    list = &sorted_PSAs.first();
                    break;

        case ID  : listWidget = listWidget_ID;
                   list = &sorted_IDs.last(); //The last list of IDs holds only IDs that aren't specific to a time.
                   break;

        case SONG  : listWidget = listWidget_Song;
                     list = &sorted_Songs.first();
                     break;
    }

    for(auto &url: urls)
    {


        //Add the filename to the listWidget.
        listWidget->addItem(url.fileName());

        //Set up to read length
        arguments << url.path();
        qInfo() << "arguments: " << arguments;
        qInfo() << "path: " << url.path();
        //Read the length of the music file
        lengthGetter.start("mediainfo", arguments);
        lengthGetter.waitForFinished(); // sets current thread to sleep and waits for pingProcess end
        QString output(lengthGetter.readAllStandardOutput());
        qInfo() << "Length: " << output << " | " << url.path();
        //Clear out the filename argument so arguments can be used again in the next loop.
        arguments.pop_back();

        //Fill the track struct and append it to the proper track list.
        auto t = Track(url, output.toLongLong());
        qInfo() << "t: " << t.path << " " << t.length;

        //Handle ID Special Case
        if (type == ID)
        {
            bool isNumber;
            int hour = url.fileName().mid(0,2).toInt(&isNumber);
            if (!isNumber)
            {
                qInfo() << "No number at the front of this ID: " << url.fileName();
                list->append(t);
            }
            else if (hour > 24 || hour < 0)
            {
                qInfo() << "Wrong number at the front of this ID!: " << url.fileName();
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

    }
}












//Unimplmeneted example code.
/*void MainWindow::saveToFile()
{
    QString fileName = QFileDialog::getSaveFileName(this,
        tr("Save Address Book"), "",
        tr("Address Book (*.abk);;All Files (*)"));

    if (fileName.isEmpty())
        return;
    else {
        QFile file(fileName);
        if (!file.open(QIODevice::WriteOnly)) {
            QMessageBox::information(this, tr("Unable to open file"),
                file.errorString());
            return;
        }

        QDataStream out(&file);
        out.setVersion(QDataStream::Qt_4_5);
        out << fullList_PSA;
    }
}


//Unimplemeneted example code.
void MainWindow::loadFromFile()
{
    QString fileName = QFileDialog::getOpenFileName(this,
        tr("Open Address Book"), "",
        tr("Address Book (*.abk);;All Files (*)"));

    if (fileName.isEmpty())
        return;
    else {

        QFile file(fileName);

        if (!file.open(QIODevice::ReadOnly)) {
            QMessageBox::information(this, tr("Unable to open file"),
                file.errorString());
            return;
        }

        QDataStream in(&file);
        in.setVersion(QDataStream::Qt_4_5);
        fullList_PSA.clear();   // clear existing contacts
        in >> fullList_PSA;

        if (fullList_PSA.isEmpty()) {
            QMessageBox::information(this, tr("No contacts in file"),
                tr("The file you are attempting to open contains no contacts."));
        } else {
            QMap<QString, QString>::iterator i = fullList_PSA.begin();
            nameLine->setText(i.key());
            addressText->setText(i.value());
        }
    }

    //updateInterface(NavigationMode);
}*/
