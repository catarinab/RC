READ ME

Compiling Commands (Makefile):

	- Compile Directory Server (DS) and User Application (User):
		~$ make

	- Compile Directory Server (DS):
		~$ make servidor

	- Compile User Application (User):
		~$ make cliente


Directory Server (DS)

	To start running the Directory Server (DS) invoke the command:
		~$ ./DS [-p DSport] [-v]

		DSport - Port where the DS server accepts requests, both in UDP and TCP. This is an optional argument. If omitted, it assumes the value 58056.
		-v option is set when invoking the program, it operates in verbose mode, meaning that the DS server outputs to the screen a short description of the received requests (UID, GID) and the IP and port originating those requests.


User Application (User)

	To start running the User Application (User) invoke the command:
		~$ ./user [-n DSIP] [-p DSport]

		DSIP - IP address of the machine where the directory server (DS) runs. This is an optional argument. If this argument is omitted, the DS should be running on the same machine (localhost).
		DSport - Port (TCP and UDP) where the DS server accepts requests. This is an optional argument. If omitted, it assumes the value 58056.



Directories present in the zip file:

	client: Contains auxiliary files for the implementation of the User Application.
	
	server: Contains auxiliary files for the implementation of the Directory Server.

	USERS: Directory Server's user permanent database.

	GROUPS: Directory Server's group permanent database.
	
	RETRIEVED: Directory containing the retrieved files by the User Application from the Directory Server.



User Application (User) Commands:

	- Register a new user ID (UID):
		reg UID pass

		UID - 5-digit identification number
		pass - 8 alphanumerical characters, restricted to letters and numbers


	- Unregister an user with identification UID and password pass:
		unregister UID pass
		unr UID pass

		UID - 5-digit identification number
		pass - 8 alphanumerical characters, restricted to letters and numbers


	- Validate the user credentials and memorizes the user ID (UID) and password (pass) in usage:
		login UID pass
		
		UID - 5-digit identification number
		pass - 8 alphanumerical characters, restricted to letters and numbers


	- Forget the credentials of the previously logged in user:
		logout


	- Display the UID of the user that is logged in:
		showuid
		su


	- Terminate User application:
		exit
	

	- Get the list of available groups. For each group is displayed group ID (GID), group name (GName) and number of the last message available for that group (MID):
		groups
		gl


	The following group management commands can only be issued after a user has logged in.


	- Subscribe a desired group, identified by its group ID (GID) and group name (GName). If the group ID is 0 this corresponds to a request to create and subscribe to a new group named GName:
		subscribe GID GName
		s GID GName
		
		GID - 2-digit group identification number
		GName - 24 alphanumerical characters (plus ‘-‘, and ‘_’)


	- Unsubscribe a desired group, identified by its group ID (GID):
		unsubscribe GID or u GID

		GID - 2-digit group identification number


	- Get the list of groups to which the logged in user has already subscribed. For each group is displayed group ID (GID), group name (GName) and number of the last message available for that group (MID):
		my_groups
		mgl


	- Select a group with group ID (GID) as the active group. Subsequent ulist, post and retrieve commands refer to this active group:
		select GID
		sag GID

		GID - 2-digit group identification number


	- Display the group ID (GID) of the selected group:
		showgid
		sg


	- Get the list of user's IDs (UIDs) that are subscribed to the currently active group. The active group must be selected before issue ulist or ul command:
		ulist
		ul


	These commands can only be issued after a user has logged in and an active group with group ID (GID) has been selected.


	- Post a message on the active group containing text (between "") and possibly a file with name Fname if included in the command:
		post “text” [Fname]
		
		Fname - limited to a maximum of 24 alphanumerical characters (and ‘-‘, ‘_’ and ‘.’), including the separating dot and the 3-letter extension.


	- Receive up to 20 messages, starting with the one with message ID (MID), for the active group. The messages are displayed as a numbered list and for each message is presented its text and, if available, the associated filenames and respective sizes. Only messages that include at least an author (UID) and text are received, any incomplete messages are omitted.
		retrieve MID
		r MID
