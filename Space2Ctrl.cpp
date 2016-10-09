/* 
   Compile with:
   g++ -o Space2Ctrl Space2Ctrl.cpp -W -Wall -L/usr/X11R6/lib -lX11 -lXtst

   To install libx11:
   in Ubuntu: sudo apt-get install libx11-dev

   To install libXTst:
   in Ubuntu: sudo apt-get install libxtst-dev

   Needs module XRecord installed. To install it, add line Load "record" to Section "Module" in
   /etc/X11/xorg.conf like this:

   Section "Module"
   Load  "record"
   EndSection

*/

#include <iostream>
#include <X11/Xlibint.h>
#include <X11/keysym.h>
#include <X11/extensions/record.h>
#include <X11/extensions/XTest.h>
#include <sys/time.h>
#include <signal.h>

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
    if (!XTestQueryExtension(userData.ctrlDisplay, &ev, &er, &ma, &mi)) {
      cout << "%sThere is no XTest extension loaded to X server.\n" << endl;
      throw exception();
    }
  }

  void setupRecordExtension() {
    XSynchronize(userData.ctrlDisplay, True);

    // Record extension exists?
    if (!XRecordQueryVersion(userData.ctrlDisplay, &recVer.first, &recVer.second)) {
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
    recContext = XRecordCreateContext(userData.ctrlDisplay, 0, &recClientSpec, 1, &recRange, 1);
    if (!recContext) {
      cout << "Could not create a record context!" << endl;
      throw exception();
    }
  }

  static int diff_ms(timeval t1, timeval t2) {
    return ( ((t1.tv_sec - t2.tv_sec) * 1000000)
             + (t1.tv_usec - t2.tv_usec) ) / 1000;
  }

  // Called from Xserver when new event occurs.
  static void eventCallback(XPointer priv, XRecordInterceptData *hook) {

    if (hook->category != XRecordFromServer) {
      XRecordFreeData(hook);
      return;
    }

    CallbackClosure *userData = (CallbackClosure *) priv;
    XRecordDatum *data = (XRecordDatum *) hook->data;
    static bool space_down = false;
    static bool key_combo = false;
    static bool modifier_down = false;
    static struct timeval startWait, endWait;

    unsigned char t = data->event.u.u.type;
    int c = data->event.u.u.detail;

    // cout << "\nState:" << endl;
    // if (space_down)
    //   cout << "space_down = true" << endl;
    // else
    //   cout << "space_down = false" << endl;

    // if (key_combo)
    //   cout << "key_combo = true" << endl;
    // else
    //   cout << "key_combo = false" << endl;

    // // if (modifier_down)
    // //   cout << "modifier_down = true" << endl;
    // // else
    // //   cout << "modifier_down = false" << endl;
    // cout << endl;

    switch (t) {
    case KeyPress:
      {
        // cout << "KeyPress";
        if (c == 65) {
          space_down = true; // space pressed
          gettimeofday(&startWait, NULL);
          // cout << "    65" << endl;

        } else if ( (c == XKeysymToKeycode(userData->ctrlDisplay, XK_Control_L))
                    || (c == XKeysymToKeycode(userData->ctrlDisplay, XK_Control_R)) ) {
          // cout << "    Control_{L||R}" << endl;
          // ctrl pressed
          if (space_down) { // space ctrl sequence
            XTestFakeKeyEvent(userData->ctrlDisplay, 255, True, CurrentTime);
            XTestFakeKeyEvent(userData->ctrlDisplay, 255, False, CurrentTime);
          }
        } else if ( (c == XKeysymToKeycode(userData->ctrlDisplay, XK_Shift_L))
                    || (c == XKeysymToKeycode(userData->ctrlDisplay, XK_Shift_R))
                    || (c == 108)
                    ) {
          // cout << "    Modifier" << endl;
          // TODO: Find a better way to get those modifiers!!!
          modifier_down = true;

        } else { // another key pressed
          // cout << "    Another" << endl;
          if (space_down) {
            key_combo = true;
          } else {
            key_combo = false;
          }

        }

        break;
      }
    case KeyRelease:
      {
        // cout << "KeyRelease";
        if (c == 65) {
          // cout << "    65";
          space_down = false; // space released

          if (!key_combo && !modifier_down) {
            // cout << "    (!key_combo && !modifier_down)";
            gettimeofday(&endWait, NULL);
            if ( diff_ms(endWait, startWait) < 600 ) {
              // if minimum timeout elapsed since space was pressed
              XTestFakeKeyEvent(userData->ctrlDisplay, 255, True, CurrentTime);
              XTestFakeKeyEvent(userData->ctrlDisplay, 255, False, CurrentTime);
            }
          }
          key_combo = false;
          // cout << endl;
        } else if ( (c == XKeysymToKeycode(userData->ctrlDisplay, XK_Control_L))
                    || (c == XKeysymToKeycode(userData->ctrlDisplay, XK_Control_R)) ) {
          // cout << "    Control_{L||R}" << endl;
          // ctrl release
          if (space_down)
            key_combo = true;
        } else if ( (c == XKeysymToKeycode(userData->ctrlDisplay, XK_Shift_L))
                    || (c == XKeysymToKeycode(userData->ctrlDisplay, XK_Shift_R))
                    || (c == 108)
                    ) {
          // cout << "    Modifier" << endl;
          // TODO: Find a better way to get those modifiers!!!
          modifier_down = false;
        }

        break;
      }
    case ButtonPress:
      {

        if (space_down) {
          key_combo = true;
        } else {
          key_combo = false;
        }

        break;
      }
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
    if (NULL == (userData.ctrlDisplay = XOpenDisplay(m_displayName.c_str())) ) {
      return false;
    }
    if (NULL == (userData.dataDisplay = XOpenDisplay(m_displayName.c_str())) ) {
      XCloseDisplay(userData.ctrlDisplay);
      userData.ctrlDisplay = NULL;
      return false;
    }

    // You may want to set custom X error handler here

    userData.initialObject = (void *) this;
    setupXTestExtension();
    setupRecordExtension();

    return true;
  }

  void start() {
    // // Remap keycode 255 to Keysym space:
    // KeySym spc = XK_space;
    // XChangeKeyboardMapping(userData.ctrlDisplay, 255, 1, &spc, 1);
    // XFlush(userData.ctrlDisplay);

    // TODO: document why the following event is needed
    XTestFakeKeyEvent(userData.ctrlDisplay, 255, True, CurrentTime);
    XTestFakeKeyEvent(userData.ctrlDisplay, 255, False, CurrentTime);

    if (!XRecordEnableContext(userData.dataDisplay, recContext, eventCallback,
                              (XPointer) &userData)) {
      throw exception();
    }
  }

  void stop() {
    if (!XRecordDisableContext (userData.ctrlDisplay, recContext)) {
      throw exception();
    }
  }

};

Space2Ctrl* space2ctrl;

void stop(int param) {
  delete space2ctrl;
  if(param == SIGTERM)
    cout << "-- Terminating Space2Ctrl --" << endl;
  exit(1);
}

int main() {
  cout << "-- Starting Space2Ctrl --" << endl;
  space2ctrl = new Space2Ctrl();

  void (*prev_fn)(int);

  prev_fn = signal (SIGTERM, stop);
  if (prev_fn==SIG_IGN) signal (SIGTERM,SIG_IGN);

  if (space2ctrl->connect(":0")) {
    space2ctrl->start();
  }
  return 0;
}
