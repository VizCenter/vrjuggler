/*************** <auto-copyright.pl BEGIN do not edit this line> **************
 *
 * VR Juggler is (C) Copyright 1998-2005 by Iowa State University
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

#include <gadget/gadgetConfig.h>

#include <iomanip>

#include <vpr/vpr.h>
#include <vpr/System.h>
#include <vpr/Sync/Guard.h>

#include <gadget/Util/Debug.h>

#include <gadget/Devices/KeyboardMouseDevice/InputAreaWin32.h> // my header

#ifndef GET_X_LPARAM
#  define GET_X_LPARAM(lp)   ((int)(short)LOWORD(lp))
#endif
#ifndef GET_Y_LPARAM
#  define GET_Y_LPARAM(lp)   ((int)(short)HIWORD(lp))
#endif

namespace gadget
{

InputAreaWin32::InputAreaWin32()
   : mWinHandle(NULL)
   , mPrevX(0)
   , mPrevY(0)
{;}

InputAreaWin32::~InputAreaWin32()
{;}

/**
 * Do not call from any other thread than controlLoop()!!!!
 * Peek and GetMessage retrieves messages for any window
 * that belongs to the calling thread...
 */
void InputAreaWin32::handleEvents()
{
   MSG msg;
   bool have_events_to_check = false;
   if ( mUseOwnDisplay )
   {
      // block until message received.
      int retval = ::GetMessage(&msg, mWinHandle, 0, 0);
      vprASSERT(retval != -1 && "invalid mWinHandle window handle or invalid lpMsg pointer");
      if ( retval == -1 ) return; // for opt mode...
      have_events_to_check = true;
   }
   // check for event, if none, then exit.
   else if ( ::PeekMessage(&msg, mWinHandle, 0, 0, PM_REMOVE) )
   {
      have_events_to_check = true;
   }
   else
   {
      have_events_to_check = false;
      return; // don't wait on the lock since there is nothing
              // afterwards that will be called when this is false.
   }

   // GUARD mKeys for duration of loop.
   // Doing it here gives makes sure that we process all events and don't get
   // only part of them for an update.
   // In order to copy data over to the mCurKeys array,
   // Lock access to the mKeys array for the duration of this function.
   vpr::Guard<vpr::Mutex> guard(mKeysLock);

   while ( have_events_to_check )
   {
      // Since we have no accelerators, no need to call
      // TranslateAccelerator here.
      ::TranslateMessage(&msg);

      // do our own special handling of kb/mouse...
      this->updKeys(msg);

      // send the message to the registered event handler
      ::DispatchMessage(&msg);

      // see if there is more messages immediately waiting
      // (don't block), process them all at once...
      int retval = ::PeekMessage(&msg, mWinHandle, 0, 0, PM_REMOVE);
      if ( retval != 0 ) // messages != 0, nomessages == 0
      {
         have_events_to_check = true;
      }
      else
      {
         have_events_to_check = false;
      }
   }
}

/**
 * UpdKeys: translates windows message into key updates
 * The WNDPROC uses its USERDATA pointer to the keyboard
 * to forward on messages to be handled from in the keyboard object.
 */
void InputAreaWin32::updKeys(const MSG& message)
{
/*
   vprDEBUG(gadgetDBG_INPUT_MGR, vprDBG_HVERB_LVL)
      << mInstName << ": KeyWin32::updKeys: Processing keys.\n"
      << vprDEBUG_FLUSH;
*/

   gadget::Keys key;

   switch ( message.message )
   {
      case WM_SIZE:
         if ( message.lParam )
         {
            InvalidateRect(mWinHandle, NULL, TRUE);
         }
         InputAreaWin32::resize(LOWORD(message.lParam), HIWORD(message.lParam));
         break;
      case WM_ACTIVATE:
         // if was activated, and not minimized
         if ( (WA_ACTIVE == LOWORD(message.wParam) ||
               WA_CLICKACTIVE == LOWORD(message.wParam)) &&
              0 == HIWORD(message.wParam) )
         {
            // and was previously locked...
            if ( Unlocked != mLockState )
            {
               this->lockMouse();
            }
         }
         // if was deactivated and minimized
         // otherwise let CAPTURECHANGED handle the lose capture case
         else if ( WA_INACTIVE == LOWORD(message.wParam) &&
                   0 != HIWORD(message.wParam) )
         {
            // and was locked...
            if ( Unlocked != mLockState )
            {
               // we will leave mLockState in the locked state so ACTIVATE
               // puts it back.
               this->unlockMouse();
            }
         }
         break;

         // sent to the window that is losing the mouse capture
      case WM_CAPTURECHANGED:
         // if locked, unlock it
         if ( Unlocked != mLockState )
         {
            // we will leave mLockState in the locked state so ACTIVATE puts
            // it back.
            this->unlockMouse();
         }
         break;

         // keys and buttons
         // When we hit a key (or button), then set the mKey and mRealkey
         // When release, only set real so that we don't lose a press of the
         // button then when the updateData happens, the framekeys will be set
         // to non-pressed from real_keys

         // press
      case WM_SYSKEYDOWN:  //Need WM_SYSKEYDOWN to capture ALT key
      case WM_KEYDOWN:
         {
            // collect data about the keypress.
            key = this->VKKeyToKey(message.wParam);

            addKeyEvent(key, gadget::KeyPressEvent, message);

            // get the repeat count in case the key was pressed
            // multiple times since last we got this message
            int repeat_count = message.lParam & 0x0000ffff;

            // check if the key was down already
            // this indicates key repeat, which we [may] want to ignore.
            bool was_down = (message.lParam & 0x40000000) == 0x40000000;
            if ( was_down )
            {
               break;
            }

            // process the keypress.
            mKeyboardMouseDevice->mKeys[key] += repeat_count;
            mKeyboardMouseDevice->mRealkeys[key] += 1;

            // Check for Escape from bad state
            // this provides a sort of failsafe to
            // get out of the locked state...
            // @todo this is sort of hard coded, do we want it this way?
            if ( key == gadget::KEY_ESC )
            {
               if ( mLockState != Unlocked )
               {
                  mLockState = Unlocked;
                  this->unlockMouse();
               }
            }
            else if ( mLockState == Unlocked )
            {
               if ( (key != mLockToggleKey) &&
                    ((gadget::KEY_ALT == key) || (gadget::KEY_CTRL == key) ||
                     (gadget::KEY_SHIFT == key)) )
               {
                  mLockState = Lock_KeyDown;       // Switch state
                  mLockStoredKey = key;         // Store the VJ key that is down
                  this->lockMouse();
               }
               else if ( key == mLockToggleKey )
               {
                  mLockState = Lock_LockKey;
                  this->lockMouse();
               }
            }
            // Just switch the current locking state
            else if ( (mLockState == Lock_KeyDown) && (key == mLockToggleKey) )
            {
               mLockState = Lock_LockKey;
            }
            else if ( (mLockState == Lock_LockKey) && (key == mLockToggleKey) )
            {
               mLockState = Unlocked;
               this->unlockMouse();
            }
         }
         break;

         // release
      case WM_SYSKEYUP:  //Need WM_SYSKEYUP to capture ALT key
      case WM_KEYUP:
         key = VKKeyToKey(message.wParam);

         mKeyboardMouseDevice->mRealkeys[key] = 0;

         /*
         vprDEBUG(gadgetDBG_INPUT_MGR, vprDBG_HVERB_LVL)
            << mInstName << ": WM_KEYUP: " << key << std::endl
            << vprDEBUG_FLUSH;
         */

         addKeyEvent(key, gadget::KeyReleaseEvent, message);

         // -- Update lock state -- //
         // lock_keyDown[key==storedKey]/unlockMouse -> unlocked
         if ( (mLockState == Lock_KeyDown) && (key == mLockStoredKey) )
         {
            mLockState = Unlocked;
            this->unlockMouse();
         }
         break;

         // press...
      case WM_LBUTTONDOWN:
         mKeyboardMouseDevice->mRealkeys[gadget::MBUTTON1] = 1;
         mKeyboardMouseDevice->mKeys[gadget::MBUTTON1] += 1;

         addMouseButtonEvent(gadget::MBUTTON1, gadget::MouseButtonPressEvent,
                             message);
         vprDEBUG(gadgetDBG_INPUT_MGR, vprDBG_HVERB_LVL)
            << "LeftButtonDown\n" << vprDEBUG_FLUSH;
         break;
      case WM_MBUTTONDOWN:
         mKeyboardMouseDevice->mRealkeys[gadget::MBUTTON2] = 1;
         mKeyboardMouseDevice->mKeys[gadget::MBUTTON2] += 1;

         addMouseButtonEvent(gadget::MBUTTON2, gadget::MouseButtonPressEvent,
                             message);
         vprDEBUG(gadgetDBG_INPUT_MGR, vprDBG_HVERB_LVL)
            << "MiddleButtonDown\n" << vprDEBUG_FLUSH;
         break;
      case WM_RBUTTONDOWN:
         mKeyboardMouseDevice->mRealkeys[gadget::MBUTTON3] = 1;
         mKeyboardMouseDevice->mKeys[gadget::MBUTTON3] += 1;

         addMouseButtonEvent(gadget::MBUTTON3, gadget::MouseButtonPressEvent,
                             message);
         vprDEBUG(gadgetDBG_INPUT_MGR, vprDBG_HVERB_LVL)
            << "RightButtonDown\n" << vprDEBUG_FLUSH;
         break;

         // release...
      case WM_LBUTTONUP:
         mKeyboardMouseDevice->mRealkeys[gadget::MBUTTON1] = 0;

         addMouseButtonEvent(gadget::MBUTTON1,
                             gadget::MouseButtonReleaseEvent, message);
         vprDEBUG(gadgetDBG_INPUT_MGR, vprDBG_HVERB_LVL)
            << "LeftButtonUp\n" << vprDEBUG_FLUSH;
         break;
      case WM_MBUTTONUP:
         mKeyboardMouseDevice->mRealkeys[gadget::MBUTTON2] = 0;

         addMouseButtonEvent(gadget::MBUTTON2,
                             gadget::MouseButtonReleaseEvent, message);
         vprDEBUG(gadgetDBG_INPUT_MGR, vprDBG_HVERB_LVL)
            << "MiddleButtonUp\n" << vprDEBUG_FLUSH;
         break;
      case WM_RBUTTONUP:
         mKeyboardMouseDevice->mRealkeys[gadget::MBUTTON3] = 0;

         addMouseButtonEvent(gadget::MBUTTON3,
                             gadget::MouseButtonReleaseEvent, message);
         vprDEBUG(gadgetDBG_INPUT_MGR, vprDBG_HVERB_LVL)
            << "RightButtonUp\n" << vprDEBUG_FLUSH;
         break;

         // mouse movement
      case WM_MOUSEMOVE:
         {
            addMouseMoveEvent(message);

            int win_center_x(mWidth / 2), win_center_y(mHeight / 2);

            int cur_x, cur_y, dx, dy;

            // Determine how far the mouse pointer moved since the last event.
            // x & y are relative to the x window
            cur_x = GET_X_LPARAM(message.lParam);
            cur_y = GET_Y_LPARAM(message.lParam);

            vprDEBUG(vprDBG_ALL,vprDBG_HVERB_LVL) << "MotionNotify: x:"
               << std::setw(6) << cur_x
               << "  y:" << std::setw(6) << cur_y << std::endl
               << vprDEBUG_FLUSH;

            if ( mLockState == Unlocked )
            {
               dx = cur_x - mPrevX;
               dy = cur_y - mPrevY;

               mPrevX = cur_x;
               mPrevY = cur_y;
            }
            else
            {
               dx = cur_x - win_center_x; // Base delta off of center of window
               dy = cur_y - win_center_y;
               mPrevX = win_center_x;    // Must do this so if state changes, we have accurate dx,dy next time
               mPrevY = win_center_y;

               // Warp back to center, IF we are not there already
               if ( (dx != 0) || (dy != 0) )
               {
                  vprDEBUG(vprDBG_ALL,vprDBG_HVERB_LVL) << "CORRECTING: x:"
                     << std::setw(6) << dx << "  y:"
                     << std::setw(6) << dy
                     << std::endl << vprDEBUG_FLUSH;

                  // convert windows coords to screen coords.
                  POINT pt;
                  pt.x = win_center_x;
                  pt.y = win_center_y;
                  ::ClientToScreen(mWinHandle, &pt);
                  ::SetCursorPos(pt.x, pt.y);
               }
            }

            // Update mKeys based on key pressed and store in the key array
            if ( dx > 0 )
            {
               mKeyboardMouseDevice->mKeys[gadget::MOUSE_POSX] += dx;     // Positive movement in the x direction.
            }
            else
            {
               mKeyboardMouseDevice->mKeys[gadget::MOUSE_NEGX] += -dx;    // Negative movement in the x direction.
            }

            if ( dy > 0 )
            {
               mKeyboardMouseDevice->mKeys[gadget::MOUSE_POSY] += dy;     // Positive movement in the y direction.
            }
            else
            {
               mKeyboardMouseDevice->mKeys[gadget::MOUSE_NEGY] += -dy;    // Negative movement in the y direction.
            }
         }
         break;
   } // end of switch...
}

void InputAreaWin32::lockMouse()
{
   // Center the mouse
   int win_center_x(mWidth / 2), win_center_y(mHeight / 2);

   // convert windows coords to screen coords.
   ::POINT pt;
   pt.x = win_center_x;
   pt.y = win_center_y;
   ::ClientToScreen(mWinHandle, &pt);
   ::SetCursorPos(pt.x, pt.y);

   // capture the mouse for this window...
   HWND previous_capture = ::SetCapture(mWinHandle);
}

void InputAreaWin32::unlockMouse()
{
   BOOL result = ::ReleaseCapture();
}

gadget::Keys InputAreaWin32::VKKeyToKey(int vkKey)
{
   switch ( vkKey )
   {
      case VK_UP      : return gadget::KEY_UP;
      case VK_DOWN    : return gadget::KEY_DOWN;
      case VK_LEFT    : return gadget::KEY_LEFT;
      case VK_RIGHT   : return gadget::KEY_RIGHT;
      case VK_CONTROL  : return gadget::KEY_CTRL;
      case VK_SHIFT   : return gadget::KEY_SHIFT;
      case VK_MENU     : return gadget::KEY_ALT;
      case /*VK_1*/0x31  : return gadget::KEY_1;
      case VK_NUMPAD1   : return gadget::KEY_1;
      case /*VK_2*/0x32  : return gadget::KEY_2;
      case VK_NUMPAD2   : return gadget::KEY_2;
      case /*VK_3*/0x33 : return gadget::KEY_3;
      case VK_NUMPAD3   : return gadget::KEY_3;
      case /*VK_4*/0x34  : return gadget::KEY_4;
      case VK_NUMPAD4   : return gadget::KEY_4;
      case /*VK_5*/0x35 : return gadget::KEY_5;
      case VK_NUMPAD5   : return gadget::KEY_5;
      case /*VK_6*/0x36  : return gadget::KEY_6;
      case VK_NUMPAD6   : return gadget::KEY_6;
      case /*VK_7*/0x37  : return gadget::KEY_7;
      case VK_NUMPAD7   : return gadget::KEY_7;
      case /*VK_8*/0x38  : return gadget::KEY_8;
      case VK_NUMPAD8   : return gadget::KEY_8;
      case /*VK_9*/0x39  : return gadget::KEY_9;
      case VK_NUMPAD9   : return gadget::KEY_9;
      case /*VK_0*/0x30  : return gadget::KEY_0;
      case VK_NUMPAD0   : return gadget::KEY_0;
      case /*VK_A*/0x41   : return gadget::KEY_A;
      case /*VK_B*/0x42   : return gadget::KEY_B;
      case /*VK_C*/0x43   : return gadget::KEY_C;
      case /*VK_D*/0x44   : return gadget::KEY_D;
      case /*VK_E*/0x45   : return gadget::KEY_E;
      case /*VK_F*/0x46   : return gadget::KEY_F;
      case /*VK_G*/0x47   : return gadget::KEY_G;
      case /*VK_H*/0x48   : return gadget::KEY_H;
      case /*VK_I*/0x49   : return gadget::KEY_I;
      case /*VK_J*/0x4a   : return gadget::KEY_J;
      case /*VK_K*/0x4b   : return gadget::KEY_K;
      case /*VK_L*/0x4c   : return gadget::KEY_L;
      case /*VK_M*/0x4d   : return gadget::KEY_M;
      case /*VK_N*/0x4e   : return gadget::KEY_N;
      case /*VK_O*/0x4f   : return gadget::KEY_O;
      case /*VK_P*/0x50   : return gadget::KEY_P;
      case /*VK_Q*/0x51   : return gadget::KEY_Q;
      case /*VK_R*/0x52   : return gadget::KEY_R;
      case /*VK_S*/0x53   : return gadget::KEY_S;
      case /*VK_T*/0x54   : return gadget::KEY_T;
      case /*VK_U*/0x55   : return gadget::KEY_U;
      case /*VK_V*/0x56   : return gadget::KEY_V;
      case /*VK_W*/0x57   : return gadget::KEY_W;
      case /*VK_X*/0x58   : return gadget::KEY_X;
      case /*VK_Y*/0x59   : return gadget::KEY_Y;
      case /*VK_Z*/0x5a   : return gadget::KEY_Z;

      case VK_ESCAPE  : return gadget::KEY_ESC;
      case VK_TAB     : return gadget::KEY_TAB;
//      case VK_BACKTAB : return gadget::KEY_BACKTAB;
      case VK_BACK    : return gadget::KEY_BACKSPACE;
      case VK_RETURN  : return gadget::KEY_RETURN;
      case VK_INSERT  : return gadget::KEY_INSERT;
      case VK_DELETE  : return gadget::KEY_DELETE;
      case VK_PAUSE   : return gadget::KEY_PAUSE;
//      case VK_???     : return gadget::KEY_SYSREQ;
      case VK_HOME    : return gadget::KEY_HOME;
      case VK_END     : return gadget::KEY_END;
      case VK_PRIOR   : return gadget::KEY_PRIOR;
      case VK_NEXT    : return gadget::KEY_NEXT;
      case VK_CAPITAL : return gadget::KEY_CAPS_LOCK;
      case VK_NUMLOCK : return gadget::KEY_NUM_LOCK;
      case VK_SCROLL  : return gadget::KEY_SCROLL_LOCK;

      case VK_F1  : return gadget::KEY_F1;
      case VK_F2  : return gadget::KEY_F2;
      case VK_F3  : return gadget::KEY_F3;
      case VK_F4  : return gadget::KEY_F4;
      case VK_F5  : return gadget::KEY_F5;
      case VK_F6  : return gadget::KEY_F6;
      case VK_F7  : return gadget::KEY_F7;
      case VK_F8  : return gadget::KEY_F8;
      case VK_F9  : return gadget::KEY_F9;
      case VK_F10 : return gadget::KEY_F10;
      case VK_F11 : return gadget::KEY_F11;
      case VK_F12 : return gadget::KEY_F12;
      case VK_F13 : return gadget::KEY_F13;
      case VK_F14 : return gadget::KEY_F14;
      case VK_F15 : return gadget::KEY_F15;
      case VK_F16 : return gadget::KEY_F16;
      case VK_F17 : return gadget::KEY_F17;
      case VK_F18 : return gadget::KEY_F18;
      case VK_F19 : return gadget::KEY_F19;
      case VK_F20 : return gadget::KEY_F20;
      case VK_F21 : return gadget::KEY_F21;
      case VK_F22 : return gadget::KEY_F22;
      case VK_F23 : return gadget::KEY_F23;
      case VK_F24 : return gadget::KEY_F24;

      case VK_HELP  : return gadget::KEY_HELP;
      case VK_SPACE : return gadget::KEY_SPACE;

      default: return gadget::KEY_UNKNOWN;
   }
}
void InputAreaWin32::addKeyEvent(const gadget::Keys& key,
                                   const gadget::EventType& type,
                                   const MSG& msg)
{
   // XXX: Missing modifier key information here...
   // XXX: Missing ASCII character value here...
   gadget::EventPtr key_event(new gadget::KeyEvent(type, key, 0, msg.time));
   mKeyboardMouseDevice->addEvent(key_event);
}

void InputAreaWin32::resize(long width, long height)
{
   mWidth = width;
   mHeight = height;
}

void InputAreaWin32::addMouseButtonEvent(const gadget::Keys& button,
                                         const gadget::EventType& type,
                                         const MSG& msg)
{
   // XXX: Missing keyboard modifier information here...
   gadget::EventPtr mouse_event(new gadget::MouseEvent(type, button,
                                                       GET_X_LPARAM(msg.lParam),
                                                       GET_Y_LPARAM(msg.lParam),
                                                       msg.pt.x, msg.pt.y, 0,
                                                       msg.time));
   mKeyboardMouseDevice->addEvent(mouse_event);
}

void InputAreaWin32::addMouseMoveEvent(const MSG& msg)
{
   gadget::EventPtr mouse_event(new gadget::MouseEvent(gadget::MouseMoveEvent,
                                                       gadget::NO_MBUTTON,
                                                       GET_X_LPARAM(msg.lParam),
                                                       GET_Y_LPARAM(msg.lParam),
                                                       msg.pt.x, msg.pt.y, 0,
                                                       msg.time));
   mKeyboardMouseDevice->addEvent(mouse_event);
}

void InputAreaWin32::doInternalError( const std::string& msg )
{
   // Get the last error code.
   DWORD dwErr = GetLastError();
   LPTSTR lpMsgBuf;

   // Get the string representation of the error code.
   FormatMessage( FORMAT_MESSAGE_ALLOCATE_BUFFER |
                  FORMAT_MESSAGE_FROM_SYSTEM |
                  FORMAT_MESSAGE_IGNORE_INSERTS,
                  NULL, dwErr, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), // Default language
                  (LPTSTR) &lpMsgBuf, 0,NULL );

   // Create a stringstream to format the output.
   std::stringstream error_output;
   error_output << msg << std::endl
                << "Failed with error: " << dwErr << ": "
                << lpMsgBuf << std::flush;

   vprDEBUG(vprDBG_ERROR, vprDBG_CRITICAL_LVL)
      << clrOutNORM(clrRED,"ERROR") << ": " << error_output.str() << std::endl
      << vprDEBUG_FLUSH;

   // Display a message box to inform the user of the error.
   MessageBox(NULL, error_output.str().c_str(), "VR Juggler Error",
              MB_OK | MB_ICONINFORMATION);

   // Free memory allocated by FormatMessage.
   LocalFree(lpMsgBuf);
}

} // end namespace
