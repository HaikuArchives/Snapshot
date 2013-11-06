#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <time.h>
#include <string.h>

#include <Application.h>
#include <String.h>
#include <Path.h>
#include <Entry.h>
#include <Directory.h>
#include <FindDirectory.h>

#include "Snapshot.h"
#include "Defs.h"

#define DEBUG 1
#include <Debug.h>

status_t	
find_and_create_directory(directory_which a_which, BVolume * a_volume = NULL, const char * a_relative_path = NULL, BPath * a_full_path = NULL);

Snapshot::Snapshot	(void)
 :	BApplication			(SNAPSHOT_APP_SIG),
	m_snapshot_dir_path		(),
 	m_settings_file			(),
 	m_settings_message		(new BMessage)
{
	status_t	status	=	B_OK;

	status	=	MakeDefaultSettings();		// setup defaults
	if (status != B_OK)	
	{
		ErrorMessage("MakeDefaultSettings()", status);
		exit (-1);
	}

	status	=	MakeSettingsFile();			// create settings file, empty if missing
	if (status != B_OK)
		ErrorMessage("MakeSettingsFile()", status);
	
	status	=	ReadSettings();				// if missing/empty we've still got the defaults
	if (status != B_OK)
		ErrorMessage("ReadSettings()", status);
	
	status	=	MakeSnapshotsFolder(); 		// create folder for snapshots, if missing
	if (status != B_OK)
	{
		ErrorMessage("MakeSnapshotsFolder()", status);
		exit (-1);
	}
}

Snapshot::~Snapshot	(void)	
{
	// void
}

void
Snapshot::ReadyToRun	(void)
{
	be_app->PostMessage(B_QUIT_REQUESTED);
}

void 
Snapshot::ArgvReceived	(int32 argc, char ** argv)
{
	// re-package args -> B_REFS_RECEIVED message
	
	// ( if no refs, or --help, print message and return ) 
	
	// then
	// ProcessRefs(a_message);
}

void 
Snapshot::RefsReceived	(BMessage * a_message)
{
	ProcessRefs(a_message);
}

void
Snapshot::ErrorMessage	(const char * a_text, int32 a_status)
{
	PRINT(("%s: %s\n", a_text, strerror(a_status)));
}

void
Snapshot::MakeShellSafe	(BString * a_string)
{
	a_string->CharacterEscape("\"$`", '\\');
	a_string->Prepend("\""); 
	a_string->Append("\""); 
}

status_t	
Snapshot::MakeSnapshotsFolder	(void)
{
	status_t	status	=	B_OK;
	mode_t		mask	=	umask (0);
	umask (mask);
	
	status	=	create_directory(m_snapshot_dir_path.Path(), mask);
	
	return status;
}

status_t	
Snapshot::MakeDefaultSettings	(void)
{
	// snapshot folder
	status_t	status	=	B_OK;

	status	=	find_directory(B_USER_DIRECTORY, & m_snapshot_dir_path, true);
	
	if (status != B_OK)
		return status;
	
	status	=	m_snapshot_dir_path.Append("Snapshots");	
		if (status != B_OK)
		return status;
		
	// Other defaults
	// ...
		
	return B_OK;
}

status_t	
Snapshot::MakeSettingsFile	(void)
{
	status_t	status	=	B_OK;
	BPath		settings_file_path;

	status	=	find_and_create_directory (B_USER_SETTINGS_DIRECTORY, NULL, "Kirilla/Snapshot", & settings_file_path);
	if (status != B_OK)
		return status;
		
	status	=	settings_file_path.Append ("settings");
	if (status != B_OK)
		return status;
	
	status	=	m_settings_file.SetTo (settings_file_path.Path(), B_READ_WRITE | B_CREATE_FILE);
	if (status != B_OK)
		return status;
		
	return B_OK;
}

status_t
Snapshot::ReadSettings	(void)
{
	status_t	status	=	B_OK;
	status	=	m_settings_message->Unflatten(& m_settings_file);

	BString	snapshots_path;
	if (m_settings_message->FindString("snapshots_path", & snapshots_path) == B_OK)
	{
		status	=	m_snapshot_dir_path.SetTo(snapshots_path.String());
		if (status != B_OK)
			return status;
	}

	/*	
	BPoint loc;
	if (msg.FindPoint("winloc",&loc) == B_OK)		Window()->MoveTo(loc);
	else	printf("Couldn't find winloc point!\n");
	*/
	
	return B_OK;
}

status_t
Snapshot::WriteSettings	(void)
{

	return B_OK;	
}

status_t	
find_and_create_directory(directory_which a_which, BVolume * a_volume, const char * a_relative_path, BPath * a_full_path)
{
	status_t	status	=	B_OK;
	BPath		path;
	mode_t		mask	=	0;
	
	status	=	find_directory (a_which, & path, true, a_volume); // create = true

	if (status != B_OK)
		return status;
	
	if (a_relative_path != NULL)
	{
		path.Append(a_relative_path);
		
		mask =	umask (0);
		umask (mask);
	
		status	=	create_directory (path.Path(), mask);
		if (status != B_OK)
			return status;
	}
	
	if (a_full_path != NULL)
	{
		status	=	a_full_path->SetTo(path.Path());
		if (status != B_OK)
			return status;
	}
	
	return B_OK;
}

void
Snapshot::ProcessRefs	(BMessage * msg)
{
//	init
	BString command			=	"/bin/zip -ry ";
	BString snapshot_name	=	m_snapshot_dir_path.Path();
	BString file_list;
	
	status_t	status	=	B_OK;
	entry_ref	ref;
	entry_ref	last_ref;
	bool		same_folder	=	true;
	
/*
	entry_ref	dir_ref;

	status	=	msg->FindRef("dir_ref", &dir_ref);
	if (status == B_OK)
	{
		BPath path (& dir_ref);		// when used as Tracker addon
		chdir(path.Path());			// telling zip where to work
	}							
*/	
	type_code	ref_type	=	B_REF_TYPE;
	int32		ref_count	=	0;
	
	status	=	msg->GetInfo("refs", & ref_type, & ref_count);
	if (status != B_OK)
		return;


	if (ref_count < 1)
		return;

	for (int index = 0;	index < ref_count ;	index++)
	{
		msg->FindRef("refs", index, & ref);
		
		if (index > 0)
		{
			if (last_ref.directory != ref.directory)
			{
				same_folder	=	false;
				break;
			}
		}
		
		last_ref =	ref;
	}	

	// change dir to avoid full paths in zip archive
	if (same_folder)
	{
		BEntry	entry	(& ref);
		entry.GetParent(& entry);
		BPath	path	(& entry);
		chdir (path.Path());
	}

	// snapshost_name
	if (ref_count == 1)
	{
		snapshot_name	+=	"/";
		snapshot_name	+=	ref.name;
		snapshot_name	+=	".zip";			
	}
	
	if (ref_count > 1)
		snapshot_name	+=	"/multiple_files.zip";	

	// create timestamp
	time_t	current_time	=	time(NULL);
	char * timestamp = new char [100];
	strftime(timestamp, 99, "%Y.%m.%d_%H:%M:%S", localtime(& current_time));

	snapshot_name	+=	" (";
	snapshot_name	+=	timestamp;	
	snapshot_name	+=	")";

	MakeShellSafe(& snapshot_name);

	// files to zip
	for (int index = 0;	index < ref_count ;	index++)
	{
		msg->FindRef("refs", index, & ref);
		// BPath path (& ref);
	
		if (same_folder)				// just the file name
		{
			BString file = ref.name;
			MakeShellSafe (& file);
		
			file_list += " ";
			file_list += file;
		}
		else							// full path
		{
			BString file;
			BPath	path	(& ref);
			file	=	path.Path();
			MakeShellSafe (& file);
		
			file_list += " ";
			file_list += file;
		}
	}
	
	// make command

	command	+=	snapshot_name;
	command +=	" ";
	command	+=	file_list;
	command +=	" &";

	// run command
	printf("command: %s\n", command.String());
	system(command.String());
	
}

