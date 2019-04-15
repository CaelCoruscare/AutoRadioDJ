#ifndef EVENTHANDLER_H
#define EVENTHANDLER_H

#include <QMainWindow>
#include <QObject>
#include <QListWidget>

#include <radiodefinitions.h>

using namespace Radio;

class EventHandler
{
public:
    EventHandler(QListWidget *lWR, QListWidget *lWO);

    void addEvent();
    void checkForEventConflicts();
    std::unique_ptr<QList<RadioEvent_Instance> > generate_DailyEventSchedule(QDate dayToGenerateFor, QTime generateAfter = QTime(0,0));

    void addEventRule(shared_ptr<Track> track, QDateTime dateTime);
    void addEventRule(shared_ptr<Track> track, QList<QTime> times, QDate firstDate, QDate lastDate, QBitArray daysOfTheWeek);
public slots:

private:
    QListWidget *listWidgetRepeating;
    QListWidget *listWidgetOneshot;
    //std::map<int, bool> eventPriority;
};

#endif // EVENTHANDLER_H
