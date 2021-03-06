//=============================================================================
//
// Adventure Game Studio (AGS)
//
// Copyright (C) 1999-2011 Chris Jones and 2011-20xx others
// The full list of copyright holders can be found in the Copyright.txt
// file, which is part of this source code distribution.
//
// The AGS source code is provided under the Artistic License 2.0.
// A copy of this license can be found in the file License.txt and at
// http://www.opensource.org/licenses/artistic-license-2.0.php
//
//=============================================================================

#include <string.h> // memset
#include <stdlib.h> // free
#include "ac/common.h"
#include "ac/roomstatus.h"
#include "util/datastream.h"

using AGS::Common::DataStream;

void RoomStatus::ReadFromFile(DataStream *in)
{
    beenhere = in->ReadInt32();
    numobj = in->ReadInt32();
    for (int i = 0; i < MAX_INIT_SPR; ++i)
    {
        obj[i].ReadFromFile(in);
    }
    in->ReadArrayOfInt16(flagstates, MAX_FLAGS);
    // might need to skip 2 if MAX_FLAGS is odd
    in->Seek(Common::kSeekCurrent, 2*(MAX_FLAGS%2));
    tsdatasize = in->ReadInt32();
    tsdata = (char *) in->ReadInt32();
    for (int i = 0; i < MAX_HOTSPOTS; ++i)
    {
        intrHotspot[i].ReadFromFile(in);
    }
    for (int i = 0; i < MAX_INIT_SPR; ++i)
    {
        intrObject[i].ReadFromFile(in);
    }
    for (int i = 0; i < MAX_REGIONS; ++i)
    {
        intrRegion[i].ReadFromFile(in);
    }
    intrRoom.ReadFromFile(in);
    in->ReadArrayOfInt8((int8_t*)hotspot_enabled, MAX_HOTSPOTS);
    in->ReadArrayOfInt8((int8_t*)region_enabled, MAX_REGIONS);
    in->ReadArrayOfInt16(walkbehind_base, MAX_OBJ);
    in->Seek(Common::kSeekCurrent, get_padding(MAX_HOTSPOTS+MAX_REGIONS+2*MAX_OBJ));
    in->ReadArrayOfInt32(interactionVariableValues, MAX_GLOBAL_VARIABLES);
}
void RoomStatus::WriteToFile(DataStream *out)
{
    char pad[4];
    out->WriteInt32(beenhere);
    out->WriteInt32(numobj);
    for (int i = 0; i < MAX_INIT_SPR; ++i)
    {
        obj[i].WriteToFile(out);
    }
    out->WriteArrayOfInt16(flagstates, MAX_FLAGS);
    // might need to skip 2 if MAX_FLAGS is odd
    out->Write(pad, 2*(MAX_FLAGS%2));
    out->WriteInt32(tsdatasize);
    out->WriteInt32((int)tsdata);
    for (int i = 0; i < MAX_HOTSPOTS; ++i)
    {
        intrHotspot[i].WriteToFile(out);
    }
    for (int i = 0; i < MAX_INIT_SPR; ++i)
    {
        intrObject[i].WriteToFile(out);
    }
    for (int i = 0; i < MAX_REGIONS; ++i)
    {
        intrRegion[i].WriteToFile(out);
    }
    intrRoom.WriteToFile(out);
    out->Write(hotspot_enabled, MAX_HOTSPOTS);
    out->Write(region_enabled, MAX_REGIONS);
    out->WriteArrayOfInt16(walkbehind_base, MAX_OBJ);
    out->Write(pad, get_padding(MAX_HOTSPOTS+MAX_REGIONS+2*MAX_OBJ));
    out->WriteArrayOfInt32(interactionVariableValues,MAX_GLOBAL_VARIABLES);
}

// JJS: Replacement for the global roomstats array in the original engine.

RoomStatus* room_statuses[MAX_ROOMS];

// Replaces all accesses to the roomstats array
RoomStatus* getRoomStatus(int room)
{
    if (room_statuses[room] == NULL)
    {
        // First access, allocate and initialise the status
        room_statuses[room] = new RoomStatus;
        memset(room_statuses[room], 0, sizeof(RoomStatus));
    }
    return room_statuses[room];
}

// Used in places where it is only important to know whether the player
// had previously entered the room. In this case it is not necessary
// to initialise the status because a player can only have been in
// a room if the status is already initialised.
bool isRoomStatusValid(int room)
{
    return (room_statuses[room] != NULL);
}

void resetRoomStatuses()
{
    for (int i = 0; i < MAX_ROOMS; i++)
    {
        if (room_statuses[i] != NULL)
        {
            if ((room_statuses[i]->tsdata != NULL) && (room_statuses[i]->tsdatasize > 0))
                free(room_statuses[i]->tsdata);

            // Don't delete the status on 2.x. The status struct contains NewInteraction
            // pointer that are also referenced in the current room struct.
            // If they are freed here this will lead to an access violation when the
            // room unloading function tries to frees them.
            if (loaded_game_file_version <= kGameVersion_272)
            {
                room_statuses[i]->tsdatasize = 0;
                room_statuses[i]->tsdata = 0;
                room_statuses[i]->beenhere = 0;
            }
            else
            {
                delete room_statuses[i];
                room_statuses[i] = NULL;
            }
        }
    }
}
