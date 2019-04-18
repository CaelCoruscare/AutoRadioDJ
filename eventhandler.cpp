#include "eventhandler.h"

#include <QDebug>
#include <algorithm>

EventHandler::EventHandler(QListWidget *lWR, QListWidget *lWO)
{
    listWidgetRepeating = lWR;
    listWidgetOneshot = lWO;
}

std::unique_ptr<QList<RadioEvent_Instance>> EventHandler::generate_DailyEventSchedule(QDate dayToGenerateFor, QTime generateAfter)
{
    auto eventList = std::make_unique<QList<RadioEvent_Instance>>();
    //generate a list of all the events within the next week, sorted by when they start.
    int dayOfWeek = dayToGenerateFor.dayOfWeek();

    //QDate.dayOfWeek() returns 1-7, or a zero if it failed.
    if (dayOfWeek == 0)
    {
        qInfo() << "Reading Day Of Week Errored!";
    }
    else {
        dayOfWeek -= 1;
    }

    //Search the list of repeating event rules and create event_instances for each viable event rule
    for(auto &event: eventList_Repeating)
    {
        //If an event happens on the given day of the week, and does not end before the given day:
        if ((event.daysOfTheWeek[dayOfWeek]) && (event.lastDate >= dayToGenerateFor))
        {
            //Create an Event Instance for each time within the event rule.
            for(auto &time: event.times)
            {
                RadioEvent_Instance instance(event.track, time, event.priority);
                eventList->append(instance);
            }
        }
    }

    //Search the list of one-time event rules and create an event_instance for each viable event rule
    for(auto &event: eventList_OneShots)
    {
        if(event.dateTime.date() == dayToGenerateFor)
        {
            RadioEvent_Instance instance(event);
            eventList->append(instance);
        }

    }

    //Sort the event list from earliest to latest.
    std::sort(eventList->begin(), eventList->end(), [](const RadioEvent_Instance &r0, const RadioEvent_Instance &r1) {
        return (r0.time < r1.time);
    });

    //Remove all the events that take place before the generateAfter variable.
    for (int i = 0; i < eventList->length(); i++)
    {
        if (eventList->at(i).time < generateAfter)
        {
            if (i > 0)
            {
                eventList->erase(eventList->begin(), eventList->begin()+i-1);
            }
            break;
        }
    }

    //TODO: Remove all conflicts between events, or else do that in the import.

    for (int i = 0; i < eventList->length(); i++)
    {
        qInfo() << "Time: " << eventList->at(i).time << "Track:" << eventList->at(i).track->path;
    }

    return eventList;
}





void EventHandler::addEventRule(shared_ptr<Track> track, QDateTime dateTime)
{
    eventList_OneShots.append(RadioEvent_Rule_OneShot(track, dateTime));
    listWidgetOneshot->addItem(track->path.fileName());

}

void EventHandler::addEventRule(shared_ptr<Track> track, QList<QTime> times, QDate firstDate, QDate lastDate, QBitArray daysOfTheWeek)
{
    eventList_Repeating.append(RadioEvent_Rule(track, times, firstDate, lastDate, daysOfTheWeek));
    listWidgetRepeating->addItem(track->path.fileName());
}

void EventHandler::checkForEventConflicts()
{

}
