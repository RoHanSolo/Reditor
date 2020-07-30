# Reditor
Linux based command-line file editor and explorer similar to vi editor

Filer takes 1 argument, which is the absolute or relative path of the folder to be opened.
Reditor takes 1 argument, which is the name of file to be opened (if it exists) or to be created. 
Reditor also accepts no arguments, and the user will be then asked to enter file name while saving the file and the file will be created. If a file with same name already exists, old file will be overwritten. 

Program provides output on alternate screen in the same terminal if the terminal has such a capability. System-command mode will run on the main screen and you will be reverted to the alternate screen after completion of the system command execution. 

Note: If program prematurely terminates, the screen will not be reverted. Enter command "tput rmcup" in such a case. 

Special keys are not to be used in insert mode. 

The second last terminal row provides the status bar and the last row is used for providing important notifications and to take input in command mode. 

Up-down movement keys take you to first letter of next/previous paragraph respectively. Right-left movement keys take you to next or previous letter of the same paragraph respectively. (Arrow keys can also be used along with hjkl) 
