/* Ripped shamelessly from: http://emg-2.blogspot.com/2008/01/xfree86xorg-keylogger.html

   compile with:
   g++ -o Space2Ctrl Space2Ctrl.cpp -L/usr/X11R6/lib -lX11 -lXtst

   To install libx11:
   in Ubuntu: sudo apt-get install libx11-dev

   To install libXTst:
   in Ubuntu: sudo apt-get install libxtst-dev

   Needs module XRecord installed. To install it, add line Load "record" to Section "Module" in /etc/X11/xorg.conf like this:

   Section "Module"
   Load  "record"
   EndSection

*/

#include <X11/Xlibint.h>
#include <X11/keysym.h>
#include <X11/extensions/record.h>
#include <X11/extensions/XTest.h>
#include <iostream>
#include <sys/time.h>
//#include <stdio.h>
using namespace std;

struct CallbackClosure {
  Display *ctrlDisplay;
  Display *dataDisplay;
  int curX;
  int curY;
  void *initialObject;
};
typedef union {
  unsigned char type;
  xEvent event;
  xResourceReq req;
  xGenericReply reply;
  xError error;
  xConnSetupPrefix setup;
} XRecordDatum;

class Space2Ctrl {
  string m_displayName;
  CallbackClosure userData;
  std::pair<int,int> recVer;
  XRecordRange *recRange;
  XRecordClientSpec recClientSpec;
  XRecordContext recContext;

  void setupXTestExtension(){
    int ev, er, ma, mi;
    if(!XTestQueryExtension(userData.ctrlDisplay, &ev, &er, &ma, &mi)){
      cout << "%sThere is no XTest extension loaded to X server.\n" << endl;
      throw exception();
    }
  }

  void setupRecordExtension() {
    XSynchronize(userData.ctrlDisplay, True);

    // Record extension exists?
    if (!XRecordQueryVersion (userData.ctrlDisplay,
			      &recVer.first, &recVer.second))
      {
	cout << "%sThere is no RECORD extension loaded to X server.\n"
	  "You must add following line:\n"
	  "   Load  \"record\"\n"
	  "to /etc/X11/xorg.conf, in section `Module'.%s" << endl;
	throw exception();
      }

    recRange = XRecordAllocRange ();
    if (!recRange) {
      // "Could not alloc record range object!\n";
      throw exception();
    }
    recRange->device_events.first = KeyPress;
    recRange->device_events.last = ButtonRelease;
    recClientSpec = XRecordAllClients;

    // Get context with our configuration
    recContext = XRecordCreateContext (userData.ctrlDisplay, 0,
				       &recClientSpec, 1, &recRange, 1);
    if (!recContext) {
      cout << "Could not create a record context!" << endl;
      throw exception();
    }
  }

  static int diff_ms(timeval t1, timeval t2)
  {
    return (((t1.tv_sec - t2.tv_sec) * 1000000) +
            (t1.tv_usec - t2.tv_usec))/1000;
  }

  // Called from Xserver when new event occurs.
  static void eventCallback(XPointer priv, XRecordInterceptData *hook) {
    if (hook->category != XRecordFromServer) {
      XRecordFreeData(hook);
      return;
    }
    CallbackClosure *userData = (CallbackClosure *)priv;
    XRecordDatum *data = (XRecordDatum *) hook->data;
    static bool space_down = false;
    static bool ctrl_down = false;
    static bool key_combo = false;
    static struct timeval startWait, endWait;

    if(data->event.u.u.type == KeyPress) {
      int c = data->event.u.u.detail;

      // Spacebar pressed
      if(c==65){
	space_down = true;
	gettimeofday(&startWait, NULL);
      }
      // Any Ctrl key pressed
      else if( (c == XKeysymToKeycode(userData->ctrlDisplay,XK_Control_L)) || (c == XKeysymToKeycode(userData->ctrlDisplay,XK_Control_R))){
	ctrl_down = true;
	if(space_down){
	  XTestFakeKeyEvent(userData->ctrlDisplay,255, True,CurrentTime);
	  XTestFakeKeyEvent(userData->ctrlDisplay,255, False,CurrentTime);
  	}
      }
      // Some other key pressed
      else {
	if(space_down) {key_combo=true;}
	else {key_combo = false;}
      }
    }

    else if(data->event.u.u.type == KeyRelease) {
      int c = data->event.u.u.detail;

      // Spacebar released
      if(c==65){
	space_down = false;
	if(!key_combo){
	gettimeofday(&endWait, NULL);
	if ( diff_ms(endWait, startWait) < 600 ) { // if minimum timeout elapsed since space was pressed
	  XTestFakeKeyEvent(userData->ctrlDisplay,255, True,CurrentTime);
	  XTestFakeKeyEvent(userData->ctrlDisplay,255, False,CurrentTime);
	}
	}
	key_combo = false;

      }
      // Any Ctrl key released
      else if( (c == XKeysymToKeycode(userData->ctrlDisplay,XK_Control_L)) || (c == XKeysymToKeycode(userData->ctrlDisplay,XK_Control_R))){
	ctrl_down = false;
	if(space_down){key_combo = true;}
      }
    }

    // Any mouse click
    else if(data->event.u.u.type == ButtonPress) {
      if(space_down) {key_combo=true;}
      else {key_combo = false;}
    }

    XRecordFreeData(hook);
  }

public:
  Space2Ctrl() { }
  ~Space2Ctrl() {
    stop();
  }

  bool connect(string displayName) {
    m_displayName = displayName;
    if (NULL == (userData.ctrlDisplay =
		 XOpenDisplay(m_displayName.c_str())) )
      return false;
    if (NULL == (userData.dataDisplay =
		 XOpenDisplay(m_displayName.c_str())) ) {
      XCloseDisplay(userData.ctrlDisplay);
      userData.ctrlDisplay = NULL;
      return false;
    }
    // You may want to set custom X error handler here

    userData.initialObject = (void *)this;
    setupXTestExtension();
    setupRecordExtension();
    return true;
  }

  void start() {

    //Remap keycode 255 to Keysym space:
    KeySym spc=XK_space;
    XChangeKeyboardMapping(userData.ctrlDisplay,255,1,&spc,1);
    XFlush(userData.ctrlDisplay);

    if (!XRecordEnableContext (userData.dataDisplay,
			       recContext, eventCallback, (XPointer) &userData))
      {
	throw exception();
      }

  }

  void stop() {
    if(!XRecordDisableContext (userData.ctrlDisplay, recContext))
      throw exception();
  }
};

int main() {

  Space2Ctrl space2ctrl;
  if(space2ctrl.connect(":0")) {
    space2ctrl.start();
  }
  return 0;
}
