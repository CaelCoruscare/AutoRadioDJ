#ifndef EVENTHANDLER_H
#define EVENTHANDLER_H

#include <QMainWindow>
#include <QObject>

#include <radiodefinitions.h>

using namespace Radio;

class EventHandler
{
public:
    EventHandler();
    void addEvent();
    void addEventRule(shared_ptr<Track> track, QDateTime dateTime);
    void checkForEventConflicts();
    void addEventRule(shared_ptr<Track> track, QList<QTime> times, QDate firstDate, QDate lastDate, QBitArray daysOfTheWeek);
    std::unique_ptr<QList<RadioEvent_Instance> > generate_DailyEventSchedule(QDate dayToGenerateFor, QTime generateAfter = QTime(0,0));
public slots:

private:
    //std::map<int, bool> eventPriority;
};

#endif // EVENTHANDLER_H
