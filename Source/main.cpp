// "Snapshot", Tracker add-on,
// © jonas.sundstrom@kirilla.com, www.kirilla.com

#include <TrackerAddOn.h>
#include <Application.h>
#include <Roster.h>

#include "Snapshot.h" 
#include "Defs.h"

//#define DEBUG 0
//#include <Debug.h>

extern "C" void 
process_refs(entry_ref dir_ref, BMessage *msg, void *)
{
	msg->AddRef("dir_ref", & dir_ref); 
	be_roster->Launch (SNAPSHOT_APP_SIG, msg );
}

int main()
{
	Snapshot app;
	app.Run();

	return (0);
}


