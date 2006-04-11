/*************** <auto-copyright.pl BEGIN do not edit this line> **************
 *
 * VR Juggler is (C) Copyright 1998-2002 by Iowa State University
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


/////////////////////////////////////////////////////////////////////////////
// KeyboardWin32.h
//
// Keyboard input device for win32
//
// History:
//
// Andy Himberger:    v0.0 - 12-1-97 - Inital version
// Allen Bierbaum:    v1.0 -  7-23-99 - Refactored to use new keyboard method
////////////////////////////////////////////////////////////////////////////

#ifndef _GADGET_KEYBOARD_WIN32_H_
#define _GADGET_KEYBOARD_WIN32_H_

// Get windows stuff
#include <windows.h>
#include <commctrl.h>

#include <vpr/Thread/Thread.h>
#include <vpr/System.h>
#include <jccl/Config/ConfigChunk.h>

#include <gadget/gadgetConfig.h>
#include <gadget/Type/Input.h>
#include <gadget/Type/Keyboard.h>
#include <gadget/Util/Debug.h>
#include <gadget/InputManager.h>

#include <gadget/Devices/Keyboard/KeyboardWin32.h> // my header


namespace gadget
{

class GADGET_CLASS_API KeyboardWin32 : public Input, public Keyboard
{
public:
   // Enum to keep track of current lock state for state machine
   // Unlocked - The mouse is free
   // Lock_LockKey - The mouse is locked due to lock toggle key press
   // Lock_LockKeyDown - The mouse is locked due to a key being held down
   enum lockState { Unlocked, Lock_LockKey, Lock_KeyDown};

   KeyboardWin32() : mControlLoopDone( false ), 
                     mPrevX( 0 ), 
                     mPrevY( 0 ), 
                     mLockState( Unlocked ),
                     mExitFlag( false ),
                     mWeOwnTheWindow( true )
   {
   }
   virtual ~KeyboardWin32() 
   { 
      stopSampling();
   }


   virtual bool config(jccl::ConfigChunkPtr c);

   // Main thread of control for this active object
   void controlLoop( void* nullParam );

   /* Pure Virtuals required by Input */
   int startSampling();
   int stopSampling();

   // process the current x-events
   // Called repetatively by the controlLoop
   int sample();
   void updateData();
   static std::string getChunkType();

   // returns the number of times the key was pressed during the
   // last frame, so you can put this in an if to check if was
   // pressed at all, or if you are doing processing based on this
   // catch the actual number..
   int isKeyPressed( int Key );
   virtual int keyPressed( int keyId );
   virtual bool modifierOnly( int modKey );

protected:
   virtual void processEvent( UINT message, UINT wParam, LONG lParam ){}
   void lockMouse();
   void unlockMouse();
   friend LONG APIENTRY MenuWndProc( HWND hWnd, UINT message, UINT wParam, LONG lParam );

   HINSTANCE   m_hInst;
   HWND        m_hWnd;
   void createWindowWin32();
   void updKeys( UINT message,  UINT wParam, LONG lParam );

   //: Handle any events in the system
   // Copies m_keys to m_curKeys
   void handleEvents();

   /** @name Private functions for processing input data */
   /* Private functions for processing input data */
   //@{
   int onlyModifier( int );
   //@}

   /** @name Windows utility functions */
   //@{
   int VKKeyToKey( int vkKey );
   char* checkArgs( char* look_for );

   BOOL MenuInit( HINSTANCE hInstance );
   //@}

   bool         mWeOwnTheWindow;       // True if this class owns the window (is reposible for opening, closing, and event processing)                      
   int          m_screen, m_x, m_y;    // screen id, x_origin, y_origin
   unsigned int m_width,m_height;

      /* Keyboard state holders */
   // NOTE: This driver does not use the normal triple buffering mechanism.
   // Instead, it just uses a modified double buffering system.
   int      m_keys[256];         // (0,*): The num key presses during an UpdateData (ie. How many keypress events)
   int      m_curKeys[256];      // (0,*): Copy of m_keys that the user reads from between updates
   int      m_realkeys[256];     // (0,1): The real keyboard state, all events processed (ie. what is the key now)

   vpr::Mutex  mKeysLock;           // Must hold this lock when accessing m_keys
   bool     mExitFlag;           // Should we exit

   lockState   mLockState;       // The current state of locking
   int         mLockStoredKey;   // The key that was pressed down
   int         mLockToggleKey;   // The key that toggles the locking

   float m_mouse_sensitivity;
   int   mSleepTimeMS;            // Amount of time to sleep in milliseconds between updates
   int   mPrevX, mPrevY;          // Previous mouse location
   bool  mControlLoopDone;
};

};

#endif
