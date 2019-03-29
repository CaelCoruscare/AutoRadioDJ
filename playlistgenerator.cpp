#include "playlistgenerator.h"

PlaylistGenerator::PlaylistGenerator()
{

}

QTime PlaylistGenerator::generateDaySonglist(QDate day, QTime startPlaylistAt, std::unique_ptr<QList<RadioEvent_Instance>> eventList)
{
    //Check for invalid sorted Track lists.
    if(!sorted_PSAs.first().length() || !sorted_Songs.first().length())
    {
        qInfo() << "Oops, sorted_PSAs or sorted_IDs is empty or had an error causing the first QVector<Track> to be empty. They are QVector<QVector<Track>> and the fist QVector<Track> should contain the least played Tracks";
        return QTime();
    }

    for(int i = 0; i < 24; i++)
    {
        if(!sorted_IDs.at(i).length())
        {
            qInfo() << "Oops, sorted_IDs is lacking an ID for the hour: " << i;
            return QTime();
        }
    }

    //Declare important Variables.
    QTime playlistEndsAt = startPlaylistAt;
    QTime time_NextID_OrPSA = QTime(startPlaylistAt.hour(), (startPlaylistAt.minute() - 1) + 15 - ((startPlaylistAt.minute() - 1) % 15)); //Midnight station ID and PSA will be handled by exception case below.
    bool doSpecialMidnightCase = false;
    bool keepGenerating = true;

    while(keepGenerating)
    {
        QTime nextTarget_TargetTime;
        Track nextTarget;
        bool isEvent;

        //If an event should happen before or at the same time as the next scheduled PSA or ID: Event will be the next target.
        if (!eventList->empty() && (eventList->first().time <= time_NextID_OrPSA))
        {
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
            nextTarget = sorted_PSAs.first().first();
            isEvent = false;
                    //sorted_IDs.at(time_NextID_OrPSA.hour()).at(qrand() % sorted_IDs.at(time_NextID_OrPSA.hour()).size()); //Fetches a random ID from the vector holding IDs for the current hour.
        }

        //Define the target window.
        auto targetWindow_Start = nextTarget_TargetTime.addSecs(-60);

        //Build until target window is reached.
        int timeUntilWindowEnd = nextTarget_TargetTime.msecsSinceStartOfDay() - playlistEndsAt.msecsSinceStartOfDay() + 30000;
        while (playlistEndsAt < targetWindow_Start)
        {
            //Get a Song that won't end after the target window.
            timeUntilWindowEnd = nextTarget_TargetTime.msecsSinceStartOfDay() - playlistEndsAt.msecsSinceStartOfDay() + 30000;
            Track songToAdd = getTrack(timeUntilWindowEnd);

            //Add the Song to the playlist and update the counter for when the playlist will end.
            playlist->addMedia(songToAdd.path);
            playlistEndsAt = playlistEndsAt.addMSecs(songToAdd.length);
        }

        //Make sure that the playlist ends on the next day, to make things easier for next day's generation.
        if (doSpecialMidnightCase)
        {
            Track iDToAdd = sorted_IDs.at(time_NextID_OrPSA.hour()).at(qrand() % sorted_IDs.at(time_NextID_OrPSA.hour()).size()); //Fetches a random ID from the vector holding IDs for the current hour.
            playlist->addMedia(iDToAdd.path);
            playlistEndsAt = playlistEndsAt.addMSecs(iDToAdd.length);

            int msecsOfExtraPSAs = 0;
            while(msecsOfExtraPSAs < 60000)
            {
                //Get the PSA.
                Track pSAToAdd = getTrack(90000, PSA);

                //Add the PSA to the playlist and update the counter for when the playlist will end.
                playlist->addMedia(pSAToAdd.path);
                playlistEndsAt = playlistEndsAt.addMSecs(pSAToAdd.length);
                msecsOfExtraPSAs += pSAToAdd.length;
            }

        }

        //If there is more than 30 seconds until the event, PSA, or ID, play a tiny PSA.
        while (timeUntilWindowEnd > 60000)
        {
            //Get the PSA.
            Track pSAToAdd = getTrack(60000, PSA);

            //Add the PSA to the playlist and update the counter for when the playlist will end.
            playlist->addMedia(pSAToAdd.path);
            playlistEndsAt = playlistEndsAt.addMSecs(pSAToAdd.length);

            //Update so the while loop can exit.
            timeUntilWindowEnd = nextTarget_TargetTime.msecsSinceStartOfDay() - playlistEndsAt.msecsSinceStartOfDay() + 30000;
        }

        //Cleanup
        if (!isEvent)
        {
            if (nextTarget_TargetTime.minute() == 0 || nextTarget_TargetTime.minute() == 59)
            {
                Track iDToAdd = sorted_IDs.at(time_NextID_OrPSA.hour()).at(qrand() % sorted_IDs.at(time_NextID_OrPSA.hour()).size()); //Fetches a random ID from the vector holding IDs for the current hour.
                playlist->addMedia(iDToAdd.path);
                playlistEndsAt = playlistEndsAt.addMSecs(iDToAdd.length);
            }

            //Dealing with the midnight case.
            if (doSpecialMidnightCase)
            {
                keepGenerating = false;
            }

            time_NextID_OrPSA = time_NextID_OrPSA.addSecs(900);

            if (time_NextID_OrPSA.msecsSinceStartOfDay() == 0)
                doSpecialMidnightCase = true;
        }

        //Add the target track to the playlist.
        playlist->addMedia(nextTarget.path);
        playlistEndsAt = playlistEndsAt.addMSecs(nextTarget.length);
    }

    //Special midnight case


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


/*void PlaylistGenerator::generateHourSonglist(int hour)
{
    //Deprecated.

    //In milliseconds.
    long int playlistDuration = 0;
    long int oneHour = 3600000;
    long int fifteenMinutes = 900000;
    long int twoMinutes = 120000;


    //Generate a list of all IDs for the current hour.
    QList<track> listIDMatchedToHour = QList<track>();
    for (auto &idTrack : fullList_ID)
    {
        if (idTrack.path.fileName().midRef(0, 2).toInt() == hour)
        {
            listIDMatchedToHour.append(idTrack);
        }
    }
    //Randomly choose one of the correct IDs, create a new playlist, and add the ID to the start of the playlist.
    track id = listIDMatchedToHour.at(qrand() % listIDMatchedToHour.size());

    nextPlaylist = new QMediaPlaylist();
    nextPlaylist->addMedia(id.path);
    ui->listWidget_SongPlaylist->addItem(id.path.fileName());
    playlistDuration += id.length;

    //std::shuffle(fullList_PSA.begin(), fullList_PSA.end(), qrand());
    //std::shuffle(fullList_Song.begin(), fullList_Song.end(), qrand());

    nextPlaylist->addMedia(fullList_PSA[0].path);

    int indexPSA = 1;
    int indexSong = 0;
    int countPSA = 1;
    while(1)
    {
        //Handle edge case of PSA or Song being fully iterated through. Should never happen.
        if (indexPSA == fullList_PSA.length())
        {
            indexPSA = 0;
        }
        if (indexSong == fullList_Song.length())
        {
            indexSong = 0;
        }

        //If adding the song would make the playlist long enough, add it and stop building the playlist.
        if((playlistDuration + fullList_Song[indexSong].length) > oneHour)
        {
            nextPlaylist->addMedia(fullList_Song[indexSong].path);
            ui->listWidget_SongPlaylist->addItem(fullList_Song[indexSong].path.fileName());
            break;
        }

        //If adding the song would make it more than about 17 minutes since the last PSA, look for a different song.
        if((playlistDuration + fullList_Song[indexSong].length) > ((fifteenMinutes * countPSA) + twoMinutes))
        {
            indexSong++;
            continue;
        }

        //If the song would make it more than 15 minutes (but less than 17 b/c of previous if statement) since the last PSA, add this song and a PSA.
        if((playlistDuration + fullList_Song[indexSong].length) > (fifteenMinutes * countPSA))
        {
            nextPlaylist->addMedia(fullList_Song[indexSong].path);
            nextPlaylist->addMedia(fullList_PSA[indexPSA].path);
            ui->listWidget_SongPlaylist->addItem(fullList_Song[indexSong].path.fileName());
            ui->listWidget_SongPlaylist->addItem(fullList_PSA[indexPSA].path.fileName());
            playlistDuration += fullList_Song[indexSong].length;
            playlistDuration += fullList_PSA[indexPSA].length;
            indexSong++;
            indexPSA++;
            countPSA++;
            continue;
        }

        //Else add a song
        nextPlaylist->addMedia(fullList_Song[indexSong].path);
        ui->listWidget_SongPlaylist->addItem(fullList_Song[indexSong].path.fileName());
        playlistDuration += fullList_Song[indexSong].length;
        indexSong++;
    }
}*/


