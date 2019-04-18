#include "playlistgenerator.h"

PlaylistGenerator::PlaylistGenerator(QListWidget *lW, QMediaPlaylist *p)
{
    listWidget = lW;
    playlist = p;
}

QTime PlaylistGenerator::generateDaySonglist(QDate day, QTime startPlaylistAt, std::unique_ptr<QList<RadioEvent_Instance>> eventList)
{
    //Check for invalid sorted Track lists.
    if(!sorted_PSAs.first().length() || !sorted_Songs.first().length())
    {
        qInfo() << "Oops, sorted_PSAs or sorted_IDs is empty or had an error causing the first QVector<Track> to be empty. They are QVector<QVector<Track>> and the fist QVector<Track> should contain the least played Tracks";
        return QTime();
    }

    //Declare important Variables.
    QTime playlistEndsAt = startPlaylistAt;

    int nextIDPSA_Minute = (startPlaylistAt.minute() - 1) + 15 - ((startPlaylistAt.minute() - 1) % 15);
    bool addHour = false;
    if (nextIDPSA_Minute == 60)
    {
        nextIDPSA_Minute = 0;
        addHour= true;
    }
    QTime time_NextID_OrPSA = QTime(startPlaylistAt.hour(), nextIDPSA_Minute); //Midnight station ID and PSA will be handled by exception case below.
    if (addHour)
        time_NextID_OrPSA = time_NextID_OrPSA.addSecs(3600);

    bool doSpecialMidnightCase = false;
    bool keepGenerating = true;

    while(keepGenerating)
    {
        qInfo() << "loop start";
        QTime nextTarget_TargetTime;
        Track nextTarget;
        bool isEvent;

        //If an event should happen before or at the same time as the next scheduled PSA or ID: Event will be the next target.
        if (!eventList->empty() && (eventList->first().time <= time_NextID_OrPSA))
        {
            qInfo() << "Found an Event";
            //Calculate when the event will end.
            QTime eventWillEndAt = eventList->first().time.addMSecs(eventList->first().track->length);

            //If the event will end after midnight (detected via rollover), stop generating after this loop.
            if(eventWillEndAt < eventList->first().time)
                keepGenerating = false;

            //If the event will end after any PSA or IDs are qued to start, cancel those PSAs and IDs
            while (keepGenerating && (time_NextID_OrPSA.hour() != 0 || time_NextID_OrPSA.minute() != 0) && eventWillEndAt > time_NextID_OrPSA) //Catches midnight and
                time_NextID_OrPSA = time_NextID_OrPSA.addSecs(900);

            //Set Next Target Variables
            nextTarget_TargetTime = eventList->first().time;
            nextTarget = *eventList->first().track;

            //Remove the event from the list
            eventList->removeFirst();

            isEvent = true;
        }
        //If an ID or PSA is next: Make them the target.
        else
        {
            //Set Next Target Variables
            nextTarget_TargetTime = (!doSpecialMidnightCase) ? time_NextID_OrPSA : QTime(23,59,59,59);
            nextTarget = getTrack(300000, PSA); //Get the least played PSA that is under 5 minutes.
            isEvent = false;
        }

        //Define the target window.
        auto targetWindow_Start = nextTarget_TargetTime.addSecs(-60);

        //Build until target window is reached.
        int timeUntilWindowEnd = nextTarget_TargetTime.msecsSinceStartOfDay() - playlistEndsAt.msecsSinceStartOfDay() + 30000;
        while (playlistEndsAt < targetWindow_Start && (!doSpecialMidnightCase || playlistEndsAt > QTime(5,5)))
        {
            //Get a Song that won't end after the target window.
            Track songToAdd = getTrack(timeUntilWindowEnd);

            qInfo() << "time: " << playlistEndsAt << "File: " << songToAdd.path;
            //Add the Song to the playlist.
            playlist->addMedia(songToAdd.path);
            //Add the song to the listwidget for upcoming songs.
            listWidget->addItem(playlistEndsAt.toString() + " " + songToAdd.path.fileName());

            //Update the counter for when the playlist will end.
            playlistEndsAt = playlistEndsAt.addMSecs(songToAdd.length);
            //Update the time until the window end.
            timeUntilWindowEnd = nextTarget_TargetTime.msecsSinceStartOfDay() - playlistEndsAt.msecsSinceStartOfDay() + 30000;
        }

        //Make sure that the playlist ends on the next day, to make things easier for next day's generation.
        if (doSpecialMidnightCase)
        {
            Track iDToAdd;
            if(!sorted_IDs.at(time_NextID_OrPSA.hour()).length())
            {
                iDToAdd = sorted_IDs.at(24).first();
            } else {
                iDToAdd = sorted_IDs.at(time_NextID_OrPSA.hour()).at(qrand() % sorted_IDs.at(time_NextID_OrPSA.hour()).size()); //Fetches a random ID from the vector holding IDs for the current hour.
            }
            qInfo() << "time: " << playlistEndsAt << "File: " << iDToAdd.path;
            playlist->addMedia(iDToAdd.path);
            listWidget->addItem(playlistEndsAt.toString() + " " + iDToAdd.path.fileName());
            playlistEndsAt = playlistEndsAt.addMSecs(static_cast<int>(iDToAdd.length));

            int msecsOfExtraPSAs = 0;
            while(msecsOfExtraPSAs < 60000)
            {
                //Get the PSA.
                Track pSAToAdd = getTrack(90000, PSA);

                //Add the PSA to the playlist and update the counter for when the playlist will end.
                qInfo() << "time: " << playlistEndsAt << "File: " << pSAToAdd.path;
                playlist->addMedia(pSAToAdd.path);
                listWidget->addItem(playlistEndsAt.toString() + " " + pSAToAdd.path.fileName());
                playlistEndsAt = playlistEndsAt.addMSecs(static_cast<int>(pSAToAdd.length));
                msecsOfExtraPSAs += pSAToAdd.length;
            }

        }

        //If there is more than 30 seconds until the event, PSA, or ID, play a tiny PSA.
        while (!doSpecialMidnightCase && timeUntilWindowEnd > 60000)
        {
            //Get the PSA.
            Track pSAToAdd = getTrack(60000, PSA);

            //Add the PSA to the playlist and update the counter for when the playlist will end.
            qInfo() << "time: " << playlistEndsAt << "Tiny PSA File: " << pSAToAdd.path;
            playlist->addMedia(pSAToAdd.path);
            listWidget->addItem(playlistEndsAt.toString() + " " + pSAToAdd.path.fileName());
            playlistEndsAt = playlistEndsAt.addMSecs(static_cast<int>(pSAToAdd.length));

            //Update so the while loop can exit.
            timeUntilWindowEnd = nextTarget_TargetTime.msecsSinceStartOfDay() - playlistEndsAt.msecsSinceStartOfDay() + 30000;
        }

        //Cleanup
        if (!isEvent)
        {
            //If the next target is top of the hour, including midnight...
            if (nextTarget_TargetTime.minute() == 0)
            {
                //Add an ID for this timeslot if possible, otherwise grab an untimed ID.
                Track iDToAdd;
                if(!sorted_IDs.at(time_NextID_OrPSA.hour()).length())
                {
                    iDToAdd = sorted_IDs.at(24).first();
                } else {
                    iDToAdd = sorted_IDs.at(time_NextID_OrPSA.hour()).at(qrand() % sorted_IDs.at(time_NextID_OrPSA.hour()).size()); //Fetches a random ID from the vector holding IDs for the current hour.
                }
                qInfo() << "time: " << playlistEndsAt << "File: " << iDToAdd.path;
                playlist->addMedia(iDToAdd.path);
                listWidget->addItem(playlistEndsAt.toString() + " " + iDToAdd.path.fileName());
                playlistEndsAt = playlistEndsAt.addMSecs(static_cast<int>(iDToAdd.length));
            }

            //Dealing with the midnight case.
            if (doSpecialMidnightCase)
            {
                keepGenerating = false;
            }

            //Update the time_nextID_OrPSA variable.
            if (!isEvent)
                time_NextID_OrPSA = time_NextID_OrPSA.addSecs(900);

            //Detect whether to do special midnight case for the next round.
            if (time_NextID_OrPSA.msecsSinceStartOfDay() == 0)
                doSpecialMidnightCase = true;
        }

        //Add the target track to the playlist.
        qInfo() << "time: " << playlistEndsAt << "File: " << nextTarget.path.fileName();
        playlist->addMedia(nextTarget.path);
        listWidget->addItem(playlistEndsAt.toString() + " " + nextTarget.path.fileName());
        playlistEndsAt = playlistEndsAt.addMSecs(static_cast<int>(nextTarget.length));
    }


    qInfo() << "Made the playlist!";

    //return the time that the playlist is expected to end.
    return playlistEndsAt;
}

Track PlaylistGenerator::getTrack(int maxLength, TrackType type)
{
    QVector< QVector< Track> > *sorted_List;
    switch(type)
    {
        case PSA  : sorted_List = &sorted_PSAs;
                    break;

        case ID   : qInfo() << "How did 'ID' get passed in? The IDs are not sorted in a way that is compatible with this method.";
                    return Track();

        case SONG  : sorted_List = &sorted_Songs;
                     break;

        case EVENT : qInfo() << "How did 'EVENT' get passed in? The Events are not sorted in a way that is compatible with this method.";
                    return Track();
    }

    //Iterate through the song library and get the track that is less than maxLength, least played, and oldest. Those evaluation criteria are in order of importance.
    for (int listOfSongs_Index = 0; listOfSongs_Index < sorted_List->length(); listOfSongs_Index++)
    {
        for (int song_Index = 0; song_Index < sorted_List->at(listOfSongs_Index).length(); song_Index++)
        {
            int lengthOfSong = sorted_List->at(listOfSongs_Index).at(song_Index).length;
            if (lengthOfSong < maxLength)
            {
                //Copy the song into the return value.
                Track t = sorted_List->at(listOfSongs_Index).at(song_Index);

                //If the song is one of the most played songs, create a new list of songs that have been played even more.
                if ((listOfSongs_Index+1) == sorted_List->length())
                    sorted_List->append(QVector<Track>());

                //Move the song onto the next list of songs, that have been played 1 time more than it.
                (*sorted_List)[listOfSongs_Index+1].append((*sorted_List)[listOfSongs_Index].takeAt(song_Index));

                //If the song was the only one in the list of songs played a certain number of times, delete that list.
                if (sorted_List->at(listOfSongs_Index).empty())
                    sorted_List->removeAt(listOfSongs_Index);

                return t;
            }
        }
    }
    //This is here to catch the case of having no Track in the list that is shorter than maxLength.
    return sorted_List->first().first();
}


