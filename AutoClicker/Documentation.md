# Developer Document - Auto Clicker
</br>
This document will focus on explaining how Auto Clicker works behind the scenes.</br>
</br>
Since this is my first C++ project aswell as first wxWidgets project a lot of thing could've been better here,</br>
but I've decided to release the program and code like this since this is an exercise after all and not a commercial project.</br>
</br>
I've followed [this guide](https://www.youtube.com/watch?v=BjQhp0eHmJw&list=PLFk1_lkqT8MbVOcwEppCPfjGOGhLvcf9G&index=1) to get the projects started.</br>
</br>

## App.cpp </br>
Its main objective is to initialize the program and the program's window. </br>
'App::OnInit()' declares the MainFrame and loads a config file (the user's saved preferences). </br>
Inside you'll also find 'App::IconManager()' function that changes the programs icons. </br>
</br>

## MainFrame.cpp </br>
I will be going function by function to break down how this script works. </br>
But before that lets go over general things: </br>
</br>
the 'wxBEGIN_EVENT_TABLE' binds controls to their functions. </br>
</br>
I've saved a lot of the controls globally since they'll be called from multiple functions: </br>
'AutoClicking'    bool - auto clicking right now or not. </br>
'ChangingBind'    bool - is user changing key bind. </br>
'RandomIntervals'  bool - enable random mouse clicks intervals. </br>
'LimitNumberOfClicks'   bool - limit how many clicks will the program click before stopping the auto click loop. </br>
'LimitMousePosition'    bool - click on a specific place on screen. </br>
'cursorLockPos'    point - the location where to click if locked. </br>
'gen'              this is a seed for a random number generator used to randomize interval between clicks. </br>
'KeyBindConfigMemory'   int - Remembers what bind the user has - used to load user preferences. </br>
</br>
Now for the functions: </br>
'MainFrame' Adds all the controls and sets their values. </br>
Generates a random seed for 'gen'. </br>
Registers the F6 as the default bind to start/stop the autoclicking loop. </br>
</br>
'OnCBClicksClicked' random intervals check box function - enables/disables random intervals. </br>
</br>
'OnCB2ClicksClicked' limit clicks - enables/disables a limit on how many times the loop will click before stopping.</br>
</br>
'OnCB3ClicksClicked' position limit - enables/disables a position lock on where the auto clicker will click.</br>
</br>
'OnButtonStartClicked' calls function 'StartLoop'.</br>
</br>
'OnButtonStopClicked' calls function 'StopLoop'.</br>
</br>
'PressedF6' calls for 'StartLoop' or 'StopLoop' depending on AutoClicking.</br>
'ChangingBind' also causes a return since you don't want the program to start autoclicking when user is changing a key.</br>
</br>
'PressedBindKey' detects what key the user pressed when changing a bind.</br>
Checks that the key is valid and remembers the key (KeyBindConfigMemory).</br>
Unregisters old bind and registers new bind.</br>
Changes text on start and stop buttons.</br>
</br>
'StartLoop' starts the loop.</br>
if 'loopRunning' then stop the loop and start again - this prevents a crash.</br>
disables start button.</br>
if LimitNumberOfClicks - get how many clicks and start counting down each click.</br>
loopRunning - means that the function is autoclicking on the separate thread (loopThread).</br>
if (RandomIntervals) - add random wait time before every click (randomInterval).</br>
'if (clicksLeft <= 1)' when true break out of the loop and disable it (it doesn't stop the loop since that would result in a crash),</br>
so instead we 'disable it' - it still runs but doesn't do anything.</br>
'std::this_thread::sleep_for' stops the loop for milliseconds(msIntervals + randomInterval).</br>
</br>
'StopLoop' stops the loop if 'loopRunning' is true.</br>
</br>
'StartMouseTrackingLoop' my weird way to track the mouse's position on screen on a seperate thread for better performance.</br>
I have no idea why this works.</br>
</br>
'MouseClick' clicks. - uses 'choice' to determine what button to press (Left,Middle or Right).</br>
if 'LimitMousePosition' is true - remember the mouse's current position - move the mouse to the locked position and click,</br>
then go back to the original position.</br>
else, just click where the mouse is.</br>
</br>
'OnButtonKeybindClicked' - user pressed the 'KeyBind' button - changes 'ChangingBind' to true.</br>
</br>
'OnButtonResetClicked' - resets all the settings to default.</br>
stops loop.</br>
unregisters the bind and registers F6(the default).</br>
and resets everything else to factory default settings.</br>
</br>
'OnButtonGithubClicked' redirects the user to my github auto clicker page or shows an error with the link.</br>
</br>
'OnSpinXUpdate' when the user changes the text in 'lock position X:' update cursorLockPos.x value.</br>
</br>
'OnSpinYUpdate' same thing but with cursorLockPos.y's value.</br>
</br>
'OnClose' program closes - stop the loop (prevents a crash message) and auto saves the user's preferences.</br>
'evt.Skip' allows the program to actually close.</br>
</br>
'LoadPrefs' reads from a config and adjusts the settings to match the user's last session's settings.</br>
</br>
'SavePrefs' saves the user's settings to a config file.</br>
'config->Flush' to actually save the changes.</br>
















