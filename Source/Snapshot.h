#include <Application.h>
#include <Messenger.h>
#include <Entry.h>
#include <Path.h>
#include <File.h>

#include "FindDirectory.h"

#ifndef _SNAPSHOT_APP_

class Snapshot : public BApplication
{
public:
	Snapshot (void);
	~Snapshot (void);

	virtual void	ReadyToRun		(void);
	virtual void	ArgvReceived	(int32 argc, char ** argv);
	virtual void	RefsReceived	(BMessage	*	a_message);

	void			ProcessRefs			(BMessage *	msg);

private:

	void			MakeShellSafe		(BString	*	a_string);
	void			ErrorMessage		(const char	*	a_text, int32 a_status);
	
	status_t		MakeDefaultSettings	(void);
	status_t		MakeSettingsFile	(void);
	status_t		ReadSettings		(void);
	status_t		WriteSettings		(void);

	status_t		MakeSnapshotsFolder	(void);

	BPath			m_snapshot_dir_path;	// where the snapshots go
	BFile			m_settings_file;		// settings-file
	BMessage	*	m_settings_message;
	
};

#define _SNAPSHOT_APP_
#endif
