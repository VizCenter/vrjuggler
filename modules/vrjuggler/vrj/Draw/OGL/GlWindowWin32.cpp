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


#include <jccl/Config/ConfigChunk.h>

#include <vrj/vrjConfig.h>
#include <vpr/Util/Assert.h>
#include <vrj/Util/Debug.h>
#include <vrj/Kernel/Kernel.h>
#include <vrj/Display/Display.h>
#include <vrj/Display/DisplayManager.h>
#include <vrj/Draw/OGL/GlWindow.h>
#include <vrj/Draw/OGL/GlWindowWin32.h>

#define GL_WINDOW_WIN32_CLASSNAME "vrj::GlWindowWin32"

namespace vrj
{

GlWindowWin32::GlWindowWin32()
   : mMatch(NULL), mWinHandle(NULL), mRenderContext(NULL),
      mDeviceContext(NULL), mAreKeyboardDevice( false )
{
}
GlWindowWin32::~GlWindowWin32()
{
   this->close();
}

// Open the window
// - Creates a window
// - Creates a rendering context
// - Registers new window with the window list
int GlWindowWin32::open()
{
   if ( false == GlWindowWin32::registerWindowClass() )
   {
      return 0;
   }

   if ( window_is_open )
   {
      return 1;
   }

   HMODULE hMod = GetModuleHandle(NULL);
   DWORD style;
   int root_height;

   // OpenGL requires WS_CLIPCHILDREN and WS_CLIPSIBLINGS.
   style = WS_CLIPCHILDREN | WS_CLIPSIBLINGS;

   // If we want a border, create an overlapped window.  This will have
   // a titlebar and a border.
   if ( border )
   {
      vprDEBUG(vrjDBG_DRAW_MGR, 5) << "attempting to give window a border"
         << std::endl << vprDEBUG_FLUSH;
      style |= WS_OVERLAPPEDWINDOW;
   }
   // Otherwise, come as close as possible to having no border by using
   // the thin-line border.
   else
   {
      vprDEBUG(vrjDBG_DRAW_MGR, 5) << "attempting to make window borderless"
         << std::endl << vprDEBUG_FLUSH;
      style |= WS_OVERLAPPED | WS_POPUP | WS_VISIBLE;
   }

   root_height = GetSystemMetrics(SM_CYSCREEN);

   // Create the main application window
   mWinHandle = CreateWindow(GL_WINDOW_WIN32_CLASSNAME,
                             GL_WINDOW_WIN32_CLASSNAME, style,
                             origin_x, root_height - origin_y - window_height,
                             window_width, window_height, NULL, NULL, hMod,
                             NULL);

   // If window was not created, quit
   if ( NULL == mWinHandle )
   {
      return 0;
   }
   
   // Attach a pointer to the device for use from the WNDPROC
   SetWindowLong( mWinHandle, GWL_USERDATA, (LPARAM)this );

   // We have a valid window, so... Create the context
   //case WM_CREATE:
   mDeviceContext = GetDC(mWinHandle);            // Store the device context
   if ( false == setPixelFormat(mDeviceContext) ) // Select the pixel format
   {
      return 0;
   }

   // Create the rendering context and make it current
   mRenderContext = wglCreateContext(mDeviceContext);
   vprASSERT(mRenderContext != NULL);
   wglMakeCurrent(mDeviceContext, mRenderContext);

   // Register the window with the window list
   GlWindowWin32::addWindow(mWinHandle,this);

   // Display the window
   ShowWindow(mWinHandle, SW_SHOW);
   UpdateWindow(mWinHandle);             // Tell the window to paint
   window_is_open = true;

   // ----------- Keyboard device starting -------------- //
   // Are we going to act like a keyboard device?
   if (true == mAreKeyboardDevice)     
   {
      this->becomeKeyboardDevice();
   }

   return 1;
}

/** do the stuff needed to become a keyboard device. */
void GlWindowWin32::becomeKeyboardDevice()
{
   // Set the parameters that we will need to get events
   gadget::KeyboardWin32::m_hWnd = mWinHandle;

   // Start up the device
   /*   Do it in out check event function
   gadget::KeyboardWin32::startSampling();
   */

   gadget::Input* dev_ptr = dynamic_cast<gadget::Input*>(this);
   vprASSERT( dev_ptr != NULL );

   // @todo Possibly not the best way to add this to input manager
   // - What happens when the keyboard is removed at run-time???
   // - What happens when this is called again?
   vrj::Kernel::instance()->getInputManager()->addDevice( dev_ptr );
}

void GlWindowWin32::removeKeyboardDevice()
{
   gadget::KeyboardWin32::m_hWnd = 0;

   gadget::Input* dev_ptr = dynamic_cast<gadget::Input*>(this);
   vprASSERT( dev_ptr != NULL );

   // @todo Possibly not the best way to add this to input manager
   // - What happens when the keyboard is removed at run-time???
   // - What happens when this is called again?

   // @todo reenable this when InputManager::removeDevice(Input*) is public
   //vrj::Kernel::instance()->getInputManager()->removeDevice( dev_ptr );
}

/**
 * Closes the OpenGL window.
 * @note Must be called by the same thread that called open.
 */
int GlWindowWin32::close()
{
   //vprASSERT( !mXfuncLock.test() && "Attempting to close a display window that is locked" );
   // Assert that we have not impllemented correct shutdown for the case that we
   // are a keyboard window as well
   //vprASSERT( !mAreKeyboardDevice  && "Need to implement win32 window close with gadget-keyboard window" );

   // if not open, then don't bother.
   if ( !window_is_open )
   {
      return false;
   }

   if (mAreKeyboardDevice)
   {
      this->removeKeyboardDevice();
   }

   // Remove window from window list
   GlWindowWin32::removeWindow(mWinHandle);

   window_is_open = false;

   // destroy the win32 window
   return(1 == DestroyWindow(mWinHandle));
}

/**
 * Sets the current OpenGL context to this window.
 * @post this.context is active context.
 */
bool GlWindowWin32::makeCurrent()
{
   vprASSERT((mDeviceContext != NULL) && (mRenderContext != NULL));
   wglMakeCurrent(mDeviceContext, mRenderContext);  // Make our context current
   return true;
}

// Swap the front and back buffers
// Process events here
void GlWindowWin32::swapBuffers()
{
   vprASSERT(mDeviceContext != NULL);
   SwapBuffers(mDeviceContext);
}

void GlWindowWin32::checkEvents()
{
   if (true == mAreKeyboardDevice)
   {
      /** Sample from the window */
      gadget::KeyboardWin32::sample();   
      // if keyboard device, use its event processor
      // it will pass all events up to this->processEvent()
      // when done, so that we can see the messages.
   }
   else
   {
      // not a keyboard device, so pump our own events
      MSG win_message;
      while (PeekMessage( &win_message, NULL, 0, 0, PM_REMOVE ))
      {
         // Test if quit
         if (win_message.message == WM_QUIT)
         {
            break;
         }

         TranslateMessage( &win_message );     // Translate the accelerator keys
         DispatchMessage( &win_message );      // Send to the WinProc
      }
   }   
}

void GlWindowWin32::processEvent( UINT message, UINT wParam, LONG lParam )
{
   
}

void GlWindowWin32::config( vrj::Display* disp )
{
   const char neg_one_STRING[] = "-1";
   vprASSERT( disp != NULL );
   vrj::GlWindow::config( disp );

    // Get the vector of display chunks
   jccl::ConfigChunkPtr dispSysChunk = DisplayManager::instance()->getDisplaySystemChunk();
   jccl::ConfigChunkPtr displayChunk = disp->getConfigChunk();

   window_name = disp->getName();
   mPipe = disp->getPipe();
   vprASSERT( mPipe >= 0 );

   mXDisplayName = dispSysChunk->getProperty<std::string>("xpipes", mPipe);
   if (mXDisplayName == neg_one_STRING)    // Use display env
   {
       const std::string DISPLAY_str("DISPLAY");    // DISPLAY_str[] = "DISPLAY";
       const char* d = getenv(DISPLAY_str.c_str());
       if (NULL != d)
       {
          mXDisplayName = std::string( d );
       }
   }
   vprDEBUG(vrjDBG_DRAW_MGR,4) << "glxWindow::config: display name is: "
                             << mXDisplayName << std::endl << vprDEBUG_FLUSH;

   bool was_i_a_keyboard = mAreKeyboardDevice;
   mAreKeyboardDevice = displayChunk->getProperty<bool>( "act_as_keyboard_device" );

   // if i'm being configured to NOT be a keyboard device, 
   // and I was one already.
   if (false == mAreKeyboardDevice && true == was_i_a_keyboard)
   {
      this->removeKeyboardDevice();
   }

   // if i'm being configured to be a keyboard device, 
   // and I wasn't one already.
   else if (true == mAreKeyboardDevice && false == was_i_a_keyboard)
   {
      // Configure keyboard device portion
      jccl::ConfigChunkPtr keyboard_chunk =
         displayChunk->getProperty<jccl::ConfigChunkPtr>( "keyboard_device_chunk" );

      // Set the name of the chunk to the same as the parent chunk (so we can point at it)
      //keyboard_chunk->setProperty("name", displayChunk->getName();

      gadget::KeyboardWin32::config( keyboard_chunk );

      // Custom configuration
      gadget::KeyboardWin32::m_width = GlWindowWin32::window_width;
      gadget::KeyboardWin32::m_height = GlWindowWin32::window_height;

      mWeOwnTheWindow = false;      // Keyboard device does not own window

      // if the window is already open, then make it a keyboard device
      // otherwise, this will be called once the window opens.
      if (window_is_open)
      {
         this->becomeKeyboardDevice();
      }
   }
}

// WindowProcedure to deal with the events generated.
// Called only for the window that we are controlling
LRESULT GlWindowWin32::handleEvent( HWND hWnd, UINT message, WPARAM wParam,
                                    LPARAM lParam )
{
   switch ( message )
   {
      // ---- Window creation, setup for OpenGL ---- //
      case WM_CREATE:
         vprASSERT(false);                               // Should never get called because
                                                         //we are not registered when this gets called

         mDeviceContext = GetDC(hWnd);                   // Store the device context
         if ( false == setPixelFormat(mDeviceContext) )  // Select the pixel format
         {
            return 0;
         }

         // Create the rendering context and make it current
         mRenderContext = wglCreateContext(mDeviceContext);
         wglMakeCurrent(mDeviceContext, mRenderContext);
         break;

         // ---- Window is being destroyed, cleanup ---- //
      case WM_DESTROY:

         // Deselect the current rendering context and delete it
         wglMakeCurrent(mDeviceContext, NULL);
         wglDeleteContext(mRenderContext);

         // Tell the application to terminate after the window
         // is gone.
         PostQuitMessage(0);
         break;

         // --- Window is resized. --- //
      case WM_SIZE:
         // Call our function which modifies the clipping
         // volume and viewport
         sizeChanged(LOWORD(lParam), HIWORD(lParam));
         break;


         // The painting function.  This message sent by Windows
         // whenever the screen needs updating.
      case WM_PAINT:
         {
            PAINTSTRUCT   ps;           // Paint structure
            BeginPaint(hWnd, &ps);  // Validate the drawing of the window
            EndPaint(hWnd, &ps);
         }
         break;

      default:   // Passes it on if unproccessed
         return(DefWindowProc(hWnd, message, wParam, lParam));

   }

   return(0L);
}

// Set the pixel format for the given window
bool GlWindowWin32::setPixelFormat(HDC hDC)
{
   int pixel_format;
   PIXELFORMATDESCRIPTOR pfd;
   mMatch = NULL;

   memset(&pfd, 0, sizeof(PIXELFORMATDESCRIPTOR));
   pfd.nSize = (sizeof(PIXELFORMATDESCRIPTOR));
   pfd.nVersion = 1;

   /* Defaults. */
   pfd.dwFlags = PFD_SUPPORT_OPENGL | PFD_DRAW_TO_WINDOW | PFD_DOUBLEBUFFER;

   if ( mDisplay->inStereo() )
   {
      in_stereo = true;
      pfd.dwFlags |= PFD_STEREO;
   }
   else
   {
      in_stereo = false;
   }

   pfd.iPixelType = PFD_TYPE_RGBA;
   pfd.cColorBits = 32;
   pfd.cRedBits = 8; /* Try to get the maximum. */
   pfd.cGreenBits = 8;
   pfd.cBlueBits = 8;
   pfd.cAlphaBits = 8;
   pfd.cDepthBits = 32;
   pfd.cStencilBits = 0;
   pfd.cAccumBits = 0;
   pfd.cAuxBuffers = 0;

   // Let Win32 choose one for us
   pixel_format = ChoosePixelFormat(hDC, &pfd);
   if ( pixel_format > 0 )
   {
      mMatch = (PIXELFORMATDESCRIPTOR *) malloc(sizeof(PIXELFORMATDESCRIPTOR));
      DescribePixelFormat(hDC, pixel_format, sizeof(PIXELFORMATDESCRIPTOR),
                          mMatch);

      /* ChoosePixelFormat is dumb in that it will return a pixel
         format that doesn't have stereo even if it was requested
         so we need to make sure that if stereo was selected, we
         got it. */
      if ( mDisplay->inStereo() )
      {
         if ( !(mMatch->dwFlags & PFD_STEREO) )
         {
            free(mMatch);
            return NULL;
         }
      }
   }

   // Set the pixel format for the device context
   SetPixelFormat(hDC, pixel_format, &pfd);
   return true;
}

// the user has changed the size of the window
void GlWindowWin32::sizeChanged( long width, long height )
{
   window_width = width;
   window_height = height;

   if ( window_width == 0 )        // Make sure we don't have window of 1 size (divide by zero would follow)
   {
      window_width = 1;
   }

   if ( window_height == 0 )
   {
      window_height = 1;
   }

   // XXX: Should reset viewport here
}



/**
 * Global Window event handler.
 */
LRESULT CALLBACK GlWindowWin32::WndProc(HWND hWnd, UINT message,
                                        WPARAM wParam, LPARAM lParam)
{
   GlWindowWin32* glWin = getGlWin(hWnd);

   if ( glWin != NULL )       // Message for one of ours
   {
      glWin->processEvent( message, wParam, lParam );
      return glWin->handleEvent(hWnd, message, wParam, lParam);
   }
   else
   {
      return DefWindowProc(hWnd, message, wParam, lParam);
   }
}

/**
 * Window registration.
 */
bool GlWindowWin32::mWinRegisteredClass = false;
WNDCLASS GlWindowWin32::mWinClass;           // The window class to register
std::map<HWND, GlWindowWin32*> GlWindowWin32::mGlWinMap;


bool GlWindowWin32::registerWindowClass()
{
   if ( mWinRegisteredClass )
   {
      return true;
   }

   char lpszAppName[1024];
   GetModuleFileName(NULL,lpszAppName,sizeof(lpszAppName));

   mWinRegisteredClass = true;     // We have registered now

   HINSTANCE hInstance = GetModuleHandle(NULL);

   // Register Window style
   mWinClass.style       = CS_HREDRAW | CS_VREDRAW | CS_OWNDC;
   mWinClass.lpfnWndProc = (WNDPROC)GlWindowWin32::WndProc;
   mWinClass.cbClsExtra  = 0;
   mWinClass.cbWndExtra  = 0;
   mWinClass.hInstance   = hInstance;            // Get handle to the module that created current process
   mWinClass.hIcon       = LoadIcon(NULL, IDI_WINLOGO);;
   mWinClass.hCursor     = LoadCursor(NULL, IDC_ARROW);

   // No need for background brush for OpenGL window
   mWinClass.hbrBackground  = NULL;

   mWinClass.lpszMenuName   = NULL;
   mWinClass.lpszClassName  = GL_WINDOW_WIN32_CLASSNAME;

   // Register the window class
   if ( RegisterClass(&mWinClass) == 0 )
   {
      return false;
   }
   else
   {
      return true;
   }
}

//----------------------//
//      WinList         //
//----------------------//
void GlWindowWin32::addWindow(HWND handle, GlWindowWin32* glWin)
{
   vprASSERT(glWin != NULL);

   if ( mGlWinMap.find(handle) == mGlWinMap.end() )     // Not already there
   {
      mGlWinMap[handle] = glWin;
   }
   //else
   // vprASSERT(false);
}

void GlWindowWin32::removeWindow(HWND handle)
{
   mGlWinMap.erase(handle);     // Erase the entry in the list
}

GlWindowWin32* GlWindowWin32::getGlWin(HWND handle)
{
   std::map<HWND, GlWindowWin32*>::iterator glWinIter;

   glWinIter = mGlWinMap.find(handle);
   if ( glWinIter == mGlWinMap.end() )     // Not found
   {
      return NULL;
   }
   else
   {
      return(*glWinIter).second;                 // Return the found window
   }
}

} // End of vrj namespace
