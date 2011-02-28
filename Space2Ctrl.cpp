// Modified from: http://developers-blog.org/blog/default/2010/08/10/XServer-Event-Handling-C-Example
// and from: http://www.doctort.org/adam/nerd-notes/x11-fake-keypress-event.html

// compile with:
// g++ -o Space2Ctrl Space2Ctrl.cpp -L/usr/X11R6/lib -lX11

// To install libx11:
// in Ubuntu: sudo apt-get install libx11-dev

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <X11/Xlib.h>
#include <X11/keysym.h>
#include <X11/XKBlib.h>
#include <X11/Xproto.h>

//represenation of X display
static Display *dpy;

//represenation of X window
static Window focuswin = None;

//Root window
static Window winRoot;

XEvent ev;
XKeyEvent fake_event;

// Key combos
bool space_down = false;
bool ctrl_down = false;
bool key_combo = false;

static int revert_to = 0;

static int xerrors (Display *d, XErrorEvent *e)
{
  if (e->request_code == X_ChangeWindowAttributes && e->error_code == BadWindow) // XSelectInput()
    {return 0;}
  
}

static void attach_to_focuswin(void) {
  
  XGetInputFocus(dpy, &focuswin, &revert_to);
  
  if (focuswin != None){
    XSelectInput(dpy, focuswin, FocusChangeMask | KeyPressMask | KeyReleaseMask );
  }
  else
    sleep(1);
}

static XKeyEvent createKeyEvent(Display *display, Window &win,
				Window &winRoot, bool press,
				int keycode, int modifiers)
{
  XKeyEvent event;
  
  event.display     = display;
  event.window      = win;
  event.root        = winRoot;
  event.subwindow   = None;
  event.time        = CurrentTime;
  event.x           = 1;
  event.y           = 1;
  event.x_root      = 1;
  event.y_root      = 1;
  event.same_screen = True;
  event.keycode     = keycode;
  event.state       = modifiers;
  
  if(press)
    event.type = KeyPress;
  else
    event.type = KeyRelease;
  
  return event;
}


static void handle_event(void) {
  
  //get current event of X display
  XNextEvent(dpy, &ev);
  
  if ( (ev.xkey.type == KeyPress) && (ev.xkey.keycode == 65) ) {    // 65 = Space-bar
    
    space_down = true;
    
  }
  
  if ( (ev.xkey.type == KeyPress) && (ev.xkey.keycode != 65)) {     // 65 = Space-bar
    
    if ( space_down ) {key_combo = true;}
    
  }
  
  if ( (ev.xkey.type == KeyPress) && (ev.xkey.keycode == XKeysymToKeycode(dpy,XK_Control_L) || ev.xkey.keycode == XKeysymToKeycode(dpy,XK_Control_R)) ) {  
    
    ctrl_down = true;
    
  }
  
  if ( (ev.xkey.type == KeyRelease) && (ev.xkey.keycode == XKeysymToKeycode(dpy,XK_Control_L) || ev.xkey.keycode == XKeysymToKeycode(dpy,XK_Control_R)) ) {
    
    ctrl_down = false;
    if(space_down){  
      fake_event = createKeyEvent(dpy, focuswin, winRoot, true, 255, 4);   
      XSendEvent(fake_event.display, fake_event.window, True, KeyPressMask, (XEvent *)&fake_event);
      fake_event = createKeyEvent(dpy, focuswin, winRoot, false, 255, 4);  
      XSendEvent(fake_event.display, fake_event.window, True, KeyPressMask, (XEvent *)&fake_event);
    }
    
  }
  
  if ( (ev.xkey.type == KeyRelease) && (ev.xkey.keycode == 65) ){    // 65 = Space-bar
    
    space_down = false;	
    if(!key_combo && !ctrl_down){ 
      fake_event = createKeyEvent(dpy, focuswin, winRoot, true, 255, 0);   
      XSendEvent(fake_event.display, fake_event.window, True, KeyPressMask, (XEvent *)&fake_event);
      fake_event = createKeyEvent(dpy, focuswin, winRoot, false, 255, 0);  
      XSendEvent(fake_event.display, fake_event.window, True, KeyPressMask, (XEvent *)&fake_event);
    }
    if(ctrl_down){  
      fake_event = createKeyEvent(dpy, focuswin, winRoot, true, 255, 4);   
      XSendEvent(fake_event.display, fake_event.window, True, KeyPressMask, (XEvent *)&fake_event);
      fake_event = createKeyEvent(dpy, focuswin, winRoot, false, 255, 4);  
      XSendEvent(fake_event.display, fake_event.window, True, KeyPressMask, (XEvent *)&fake_event);
    }
    key_combo = false;
  }
  
  if ( ev.xany.type == FocusOut ) {    
    focuswin = None;
    space_down = false;
    key_combo = false;
    ctrl_down = false;
  }
  
}

int main(void) {
  //how to get xserver display
  dpy = XOpenDisplay(getenv("DISPLAY"));
  
  //Remap keycode 255 to Keysym space:
  KeySym spc=XK_space;
  XChangeKeyboardMapping(dpy,255,1,&spc,1); 
  XFlush(dpy);  
  
  
  //if a display dont't exist    
  if (dpy == NULL) {
    fprintf(stdout, "cannot init display\n");          
    exit(1);
  }   
  
  // Get the root window for the current display.
  winRoot = XDefaultRootWindow(dpy);
  
  // Avoid auto repeat	
  XkbSetDetectableAutoRepeat(dpy, True, NULL);
  
  // Set new error handler
  XSetErrorHandler (xerrors);
  
  //daemon loop
  while (1) {
    if (focuswin == None)
      attach_to_focuswin();
    else
      handle_event();
  }
}
