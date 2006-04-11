/*************** <auto-copyright.pl BEGIN do not edit this line> **************
 *
 * VR Juggler is (C) Copyright 1998, 1999, 2000 by Iowa State University
 *
 * Original Authors:
 *   Allen Bierbaum, Christopher Just,
 *   Patrick Hartling, Kevin Meinert,
 *   Carolina Cruz-Neira, Albert Baker
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public
 * License along with this library; if not, write to the
 * Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.
 *
 * -----------------------------------------------------------------
 * File:          $RCSfile$
 * Date modified: $Date$
 * Version:       $Revision$
 * -----------------------------------------------------------------
 *
 *************** <auto-copyright.pl END do not edit this line> ***************/
#ifndef _VJ_XWIN_KEYBOARD_H_
#define _VJ_XWIN_KEYBOARD_H_

#include <vjConfig.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/keysym.h>

#include <Input/vjInput/vjKeyboard.h>
#include <Sync/vjMutex.h>

class vjConfigChunk;

//---------------------------------------------------------------
//: XWin Keyboard class
// Converts keyboard input into simulated input devices
//
// This device is a source of keyboard events.  The device should not be
// used directly, it should be referenced through proxies.
//
// Mouse Locking:
//    This device recieves input from the XWindows display.  As such,
//  the xwindow must have focus to generate events.  In order to help
//  users keep the window in focus, there are two cases where the
//  driver will "lock" the mouse to the window, thus preventing lock of focus.
//  CASE 1: The user holds down any key. (ie. a,b, ctrl, shift, etc)
//  CASE 2: The user can toggle locking using a special "locking" key
//           defined in the configuration chunk.
//
// See also: vjKeyboard vjKeyboardProxy
//--------------------------------------------------------------
class vjXWinKeyboard : virtual public vjKeyboard
{
public:
   // Enum to keep track of current lock state for state machine
   // Unlocked - The mouse is free
   // Lock_LockKey - The mouse is locked due to lock toggle key press
   // Lock_LockKeyDown - The mouse is locked due to a key being held down
   enum lockState { Unlocked, Lock_LockKey, Lock_KeyDown};

   vjXWinKeyboard()
   {
      m_visual = NULL;
      m_display = NULL;
      //myThread = NULL; -- Should be done in base constructor

      mPrevX = 0; mPrevY = 0;
      mLockState = Unlocked;     // Initialize to unlocked.
      mExitFlag = false;
      mUpdKeysHasBeenCalled = false;      // Initialize it to not being called yet
   }
   ~vjXWinKeyboard() { stopSampling();}

   virtual bool config(vjConfigChunk* c);

   // Main thread of control for this active object
   void controlLoop(void* nullParam);

   /* Pure Virtuals required by vjInput */
   int startSampling();
   int stopSampling();

   // process the current x-events
   // Called repetatively by the controlLoop
   int sample() { updKeys(); return 1; }
   void updateData();

   char* getDeviceName() { return "vjXwinKeyboard";}
   static std::string getChunkType() { return std::string("Keyboard");}

   // returns the number of times the key was pressed during the
   // last frame, so you can put this in an if to check if was
   // pressed at all, or if you are doing processing based on this
   // catch the actual number..
   int isKeyPressed(int vjKey)
   {  return m_curKeys[vjKey];}

   virtual int keyPressed(int keyId)
   { return isKeyPressed(keyId); }

   virtual bool modifierOnly(int modKey)
   { return onlyModifier(modKey); }

private:
   /* Private functions for processing input data */
   int onlyModifier(int);

   //: Update the keys.
   // Copies m_keys to m_curKeys
   void updKeys();

   /* X-Windows utility functions */
   //: Convert XKey to vjKey
   //! NOTE: Keypad keys are transformed ONLY to number keys
   int xKeyTovjKey(KeySym xKey);

   // Open the X window to sample from
   int openTheWindow();

   int filterEvent( XEvent* event, int want_exposes,
                    int width, int height);
   char*    checkArgs(char* look_for);
   void     checkGeometry();
   Window   createWindow (Window parent, unsigned int border, unsigned long
                        fore, unsigned long back, unsigned long event_mask);
   void     setHints(Window window, char*  window_name, char*  icon_name,
                 char* class_name, char* class_type);

   //: Perform anything that must be done when state switches
   void lockMouse();
   void unlockMouse();

   Window       m_window;
   XVisualInfo* m_visual;
   Display*     m_display;
   XSetWindowAttributes m_swa;
   int          m_screen, m_x, m_y;    // screen id, x_origin, y_origin
   unsigned int m_width,m_height;

   /* Keyboard state holders */
   // NOTE: This driver does not use the normal triple buffering mechanism.
   // Instead, it just uses a modified double buffering system.
   int      m_keys[256];         // (0,*): The num key presses during an UpdateData (ie. How many keypress events)
   int      m_curKeys[256];      // (0,*): Copy of m_keys that the user reads from between updates
   int      m_realkeys[256];     // (0,1): The real keyboard state, all events processed (ie. what is the key now)
   vjMutex  mKeysLock;           // Must hold this lock when accessing m_keys OR mUpdKeysHasBeenCalled
   bool     mUpdKeysHasBeenCalled;  // This flag keeps track of wether or not updKeys has been called since the last updateData.
                                    // It is used by updateData to make sure we don't get a "blank" update where no keys are pressed.
   bool     mExitFlag;           // Should we exit

   lockState   mLockState;       // The current state of locking
   int         mLockStoredKey;   // The key that was pressed down
   int         mLockToggleKey;   // The key that toggles the locking

   std::string mXDisplayString;   // The display string to use from systemDisplay config info

   float m_mouse_sensitivity;
   int   mSleepTimeMS;            // Amount of time to sleep in milliseconds between updates
   int   mPrevX, mPrevY;          // Previous mouse location
};

#endif
