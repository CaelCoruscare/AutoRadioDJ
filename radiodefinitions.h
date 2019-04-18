#ifndef RADIODEFINITIONS_H
#define RADIODEFINITIONS_H

#include <QUrl>
#include <QDateTime>
#include <memory>
#include <QMediaPlayer>
#include <QBitArray>

using std::shared_ptr;
using std::unique_ptr;

namespace Radio {
    //toDo & Event stuff
    struct toDo {
      QUrl path;
      int length;
      QDateTime playOn();
    };

    //Track Stuff
    enum TrackType { PSA, ID, SONG, EVENT };

    struct Track {
      QUrl path;
      int length; //In Milliseconds
      Track()
      {}
      Track(QUrl path, int length)
          : path(path)
          , length(length)
      {}
    };

    extern int priority;

    struct RadioEvent_Rule {
        shared_ptr<Track> track;
        QList<QTime> times;
        QDate firstDate;
        QDate lastDate;
        QBitArray daysOfTheWeek{7}; //Starts on Sunday
        int priority = 0;
        RadioEvent_Rule()
        {}
        RadioEvent_Rule(shared_ptr<Track> track, QList<QTime> times, QDate firstDate, QDate lastDate, QBitArray daysOfTheWeek)
            : track(track)
            , times(times)
            , firstDate(firstDate)
            , lastDate(lastDate)
            , daysOfTheWeek(daysOfTheWeek) //Starts on Sunday
            , priority(Radio::priority++)
        {}
    };

    struct RadioEvent_Rule_OneShot {
        shared_ptr<Track> track;
        QDateTime dateTime;
        int priority;
        RadioEvent_Rule_OneShot()
        {}
        RadioEvent_Rule_OneShot(shared_ptr<Track> track, QDateTime dateTime)
            : track(track)
            , dateTime(dateTime)
            , priority(Radio::priority++)
        {}
    };

    struct RadioEvent_Instance {
        shared_ptr<Track> track;
        QTime time;
        int priority;
        RadioEvent_Instance(shared_ptr<Track> track, QTime time, int priority)
            : track(track)
            , time(time)
            , priority(priority)
        {}
        RadioEvent_Instance(RadioEvent_Rule& rule)
        {
            track = rule.track;
            time = rule.times.first();
            priority = rule.priority;
            for (int i = 1; i < rule.times.length(); i++)
            {
                RadioEvent_Instance(rule.track, rule.times[i], rule.priority);
            }
        }
        RadioEvent_Instance(RadioEvent_Rule_OneShot& rule)
            : track(rule.track)
            , time(rule.dateTime.time())
            , priority(rule.priority)
        {}
    };

    struct RadioEvent_Info {

    };

    extern QVector< QVector< Track> > sorted_Songs;
    extern QVector< QVector< Track> > sorted_PSAs;
    extern QVector< QVector< Track> > sorted_IDs; //This one is sorted by hour, not by most played.

    extern QVector<RadioEvent_Rule> eventList_Repeating;
    extern QVector<RadioEvent_Rule_OneShot> eventList_OneShots;
}

#endif // RADIODEFINITIONS_H
