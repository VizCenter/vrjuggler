/*
 * VRJuggler
 *   Copyright (C) 1997,1998,1999,2000
 *   Iowa State University Research Foundation, Inc.
 *   All Rights Reserved
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
 */


#include <Kernel/GL/vjGlxWindow.h>
#include <vjConfig.h>

#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <GL/glx.h>

#include <Kernel/GL/vjGlWindow.h>
#include <Kernel/vjKernel.h>
#include <Kernel/vjDebug.h>
#include <Kernel/vjDisplay.h>


vjGlxWindow::vjGlxWindow() {
   window_is_open = 0;
   window_width = window_height = -1;
   x_display = NULL;
   visual_info = NULL;
   glx_context = NULL;
}



vjGlxWindow::~vjGlxWindow() {
    close();
}



void vjGlxWindow::swapBuffers() {
   glXSwapBuffers (x_display,  x_window);
}



int vjGlxWindow::open() {
    /* attempts to open the glxWindow & create the gl context.  Does nothing
     * if the window is already open (& returns true).
     * returns true for success, false for failure.
     * The newly opened window will be set as the calling proccess'
     * current gl context.
     */

    XEvent fooevent;
    XSetWindowAttributes w_attrib;
    int screen;
    char* foo;
    XSizeHints *sizehints;
    XClassHint *classhint;

    vjDEBUG(vjDBG_DRAW_MGR,3) << "glxWindow: Open window\n" << vjDEBUG_FLUSH;

    if (window_is_open)
        return true;

    if (window_width == -1) {
        vjDEBUG(vjDBG_ERROR, 0) << "ERROR: vjGlxWindow: Window has not been configured\n" << vjDEBUG_FLUSH;
        return false;
    }

    if (! (x_display = XOpenDisplay (display_name))) {
        vjDEBUG(vjDBG_ERROR, 0) << "ERROR: vjGlxWindow: Unable to open display '" << display_name << "'.\n" << vjDEBUG_FLUSH;
        return false;
    }

    screen = DefaultScreen (x_display);

    // get an XVisualInfo*, which we'll need below
    if ((visual_info = GetGlxVisInfo (x_display, screen)) == NULL) {
        vjDEBUG(vjDBG_ERROR,0) << "ERROR: vjGlxWindow: glXChooseVisual failed\n" << vjDEBUG_FLUSH;
        goto OPEN_FAIL;
    }

    // window attributes.
    if ((w_attrib.colormap = XCreateColormap (x_display,
                                              RootWindow(x_display, screen),
                                              visual_info->visual,
                                              AllocNone)) == 0) {
        vjDEBUG(vjDBG_ERROR,0) << "ERROR: vjGlxWindow: XCreateColorMap failed on '" << display_name << "'.\n" << vjDEBUG_FLUSH;
        goto OPEN_FAIL;
    }

    w_attrib.event_mask = ExposureMask | StructureNotifyMask
        | PointerMotionMask | KeyPressMask;
    w_attrib.border_pixel = 0x0;

    // create window
    if ((x_window = XCreateWindow (x_display, RootWindow(x_display, screen),
                                   origin_x, origin_y - window_height,
                                   window_width, window_height,
                                   0, visual_info->depth,
                                   InputOutput,
                                   visual_info->visual,
                                   CWEventMask | CWColormap | CWBorderPixel,
                                   /* ^--attrib mask*/
                                   &w_attrib))  /* Attributes */
        == 0)
        {
            vjDEBUG(vjDBG_DRAW_MGR,0) << "ERROR: vjGlxWindow: Couldn't create window for " << display_name << endl << vjDEBUG_FLUSH;
            goto OPEN_FAIL;
        }

    /***************** Set Window Name/Class/Size/Pos *********************/

    /* Before we map the window, we need a name for it (this is also useful for
     * the resource cruft that'll get rid of the borders).
     */
    classhint = XAllocClassHint();
    classhint->res_name = (char*)window_name.c_str();
    classhint->res_class = "vj GLX";
    //XSetClassHint (x_display, x_window, classhint);

    // InSoc makes things simple
    // X makes things complicated
    XTextProperty w_name;
    foo = (char*)window_name.c_str();
    XStringListToTextProperty(&foo, 1, &w_name);

    /* The next few lines of crud are needed to get it through the window
     * manager's skull that yes, we DO have a REASON for wanting the window
     * to be positioned WHERE WE TOLD YOU TO, and not where YOU think it should
     * go, thank you very much, I'M the APPLICATION so stop $%^*&%#@! SECOND
     * GUESSING ME!
     */
    sizehints = XAllocSizeHints();
    sizehints->flags = USPosition;

    XSetWMProperties (x_display, x_window, &w_name, &w_name, 
                      NULL, 0, sizehints, NULL, classhint);
    
    XFree (w_name.value);
    XFree (classhint);
    XFree (sizehints);

    /***************** Border Stuff ***************************/

    /* Get rid of window border, if configured to do so.
     * This technique doesn't require any modifications to the .XDefaults file
     * or anything, but it will only work with window managers based on MWM
     * (the Motif window manager).  That covers most cases.
     * Unfortunately, the generic X resources for communicating with a window
     * manager don't support this feature.
     */
    if (!border) {
        vjDEBUG(vjDBG_DRAW_MGR,5) << "attempting to make window borderless" << endl << vjDEBUG_FLUSH;
        Atom vjMotifHints = XInternAtom (x_display, "_MOTIF_WM_HINTS", 0);
        if (vjMotifHints == None) {
            vjDEBUG(vjDBG_DRAW_MGR,0) << "ERROR: vjGlxWindow: Could not get X atom for _MOTIF_WM_HINTS." << endl << vjDEBUG_FLUSH;
        }
        else {
            MotifWmHints hints;
            hints.flags = MWM_HINTS_DECORATIONS;
            hints.decorations = 0;
            XChangeProperty(x_display, x_window,
                            vjMotifHints, vjMotifHints, 32,
                            PropModeReplace, (unsigned char *) &hints, 4);
        }
    }

    /********************* Mapping Window **************************/

    /* Now that we've straightened that out with a minimum of bloodshed,
     * we can actually map the window.  The XIfEvent makes us wait until
     * it's actually on screen.
     */

    XMapWindow (x_display, x_window);
    XIfEvent (x_display, &fooevent, EventIsMapNotify, (XPointer)x_window);

    vjDEBUG(vjDBG_DRAW_MGR,4) << "vjGlxWindow: done mapping window\n" << vjDEBUG_FLUSH;

    /********************* OpenGL Context Stuff *********************/

    glx_context = glXCreateContext (x_display,visual_info, NULL, True);
    if (NULL == glx_context) {
        vjDEBUG(vjDBG_ERROR,0) << "ERROR: Couldn't create GlxContext for '" << display_name << "'\n" << vjDEBUG_FLUSH;
        goto OPEN_FAIL;
    }

//     vjASSERT(NULL != glx_context);
//     if (!glXMakeCurrent ( x_display, x_window, glx_context  )) {
//         vjDEBUG(vjDBG_ERROR,0) << "ERROR: Couldn't set GlxContext for '" << display_name << "'\n" << vjDEBUG_FLUSH;
//         return false;
//     }

    /* allen found a hint online that if we have the controlling process
     * "disown" the gl context we just created, we could then have another
     * process pick it up and use it.  However, it doesn't seem to have
     * worked for us.
     */
    // glXMakeCurrent (x_display,0,0);

    window_is_open = 1;

    return true;

 OPEN_FAIL:
    // close() is coincidentally safe to call on a partially-opened
    // vjGlxWindow, and will deallocate all the stuff we might have
    // allocated above.
    close(); 
    return false;
}



//: Closes the window given
//! NOTE: this function mucks with the current rendering context */
int vjGlxWindow::close() {
//     if (!window_is_open)
//         return true;

   if (glx_context)
   {
      makeCurrent();    // Might not need this
      //glFlush();      // This is done by the changing context
      glXMakeCurrent ( x_display, None, NULL);     // Release the context, and don't assign a new one
      glXDestroyContext (x_display,  glx_context);
      glx_context = NULL;
   }
   if (x_window)
   {
      XDestroyWindow (x_display, x_window);
      x_window = 0;
   }
   if (visual_info)
   {
      XFree (visual_info);
      visual_info = NULL;
   }
   if (x_display)
   {
      XCloseDisplay (x_display);
      x_display = NULL;
   }

   window_is_open = false;
   return true;

} /* close() */



bool vjGlxWindow::makeCurrent() {
   /* returns true for success,
    * false for failure (eg window not open or glXMakeCurrent fails)
    */
   if (!window_is_open)
      return false;

   vjASSERT(glx_context != NULL);
   return glXMakeCurrent ( x_display, x_window, glx_context  );
}

void vjGlxWindow::config(vjDisplay* _display)
{
   vjGlWindow::config(_display);

    // Get the vector of display chunks
   vjConfigChunk* dispSysChunk;
   //dispSysChunk = vjKernel::instance()->getInitialChunkDB()->getMatching("displaySystem");
   dispSysChunk = vjDisplayManager::instance()->getDisplaySystemChunk();

   window_name = _display->getName();
   pipe = _display->getPipe();

   display_name = dispSysChunk->getProperty("xpipes", pipe).cstring();
   if(strcmp(display_name, "-1") == 0)    // Use display env
      strcpy(display_name, getenv("DISPLAY"));
   vjDEBUG(vjDBG_DRAW_MGR,4) << "glxWindow::config: display name is: " << display_name << endl << vjDEBUG_FLUSH;
}


/**** Static Helpers *****/
/* static */ bool vjGlxWindow::createHardwareSwapGroup(std::vector<vjGlWindow*> wins)
{
   // Convert to glx windows
   std::vector<vjGlxWindow*> glx_wins;

   if(wins.size() <= 0)
      vjDEBUG(vjDBG_ERROR,0) << "WARNING: createHardwareSwapGroup called with no windows\n" << vjDEBUG_FLUSH;

   for(unsigned int i=0;i<wins.size();i++)
   {
      vjGlxWindow* glx_win = dynamic_cast<vjGlxWindow*>(wins[i]);
      vjASSERT(glx_win != NULL);    // Make sure we have the right type
      glx_wins.push_back(glx_win);
   }

   // Create hardware group
#ifdef VJ_OS_IRIX
   for(i=0;i<glx_wins.size();i++)      // For each window
   {
      if(glx_wins[i] != this)    // If not me
      {                          //then add with me to the swap group
         glXJoinSwapGroupSGIX(x_display, x_window, glx_wins[i]->x_window);
      }
   }

#else
   vjDEBUG(vjDBG_ERROR,0) << "WARNING: createHardwareSwapGroup not supported.\n" << vjDEBUG_FLUSH;
#endif
   return true;
}


/***********************************************************/
/* private member functions.  these get profoundly painful */
/***********************************************************/
XVisualInfo* vjGlxWindow::GetGlxVisInfo (Display *display, int screen)
{
   /* pre:  screen is a screen on the current XDisplay, and
    *       XDisplay is already defined and valid.
    * post: returns a pointer to an XVisualInfo to be used for GLX.
    *       Note that it doesn't necessarily have _all_ of the requested
    *       features... it will attempt to get a visual in mono or
    *       without alpha if it's virst attempts fail.
    *       Returns NULL if it can't come up with a reasonably close
    *       XVisualInfo, or if the display in question doesn't support
    *       GLX
    */


   XVisualInfo *vi;
   const int MaxAttributes = 32;
   int viattrib[MaxAttributes] = {
      GLX_DOUBLEBUFFER,
      GLX_RGBA,
      GLX_DEPTH_SIZE, 1,
      GLX_RED_SIZE, 1,
      GLX_GREEN_SIZE, 1,
      GLX_BLUE_SIZE, 1,
      GLX_ALPHA_SIZE, 1,
   };
   int NumAttribs = 12;
   int AlphaAttribIndex = 11;
   
   /* Notes on viattrib:  by using 1 for GLX_RED_SIZE et.al. we ask
    * for the _largest_ available buffers.  If this fails,  we might
    * want to try setting alpha size to 0 (smallest possible, maybe 0)
    * which is required eg. for alpha on the indys.
    *
    * Implementation note: the code below makes assumptions about the
    * exact order of the arguments in viattrib.  Alter those, & you'll
    * Need to redo the indices used.
    */
   

   if (!glXQueryExtension (display, NULL, NULL))
   {
      vjDEBUG(vjDBG_ERROR,0) << "ERROR: Display '"<< display_name <<
      "' doesn't support GLX.\n" << vjDEBUG_FLUSH;
      return NULL;
   }

   if (mDisplay->inStereo())
   {
      viattrib[NumAttribs++] = GLX_STEREO;
      viattrib[NumAttribs] = None;
      in_stereo = true;
   }
   else
      in_stereo = false;


   // first, see if we can get exactly what we want.
   if ( (vi = glXChooseVisual (display, screen, viattrib)) )
      return vi;

   // still no luck. if we were going for stereo, let's try without.
   if (mDisplay->inStereo())
   {
      vjDEBUG(vjDBG_DRAW_MGR,0) << "WARNING: Display process for '" << display_name
                 << "' couldn't get display in stereo - trying mono.\n"
                 << vjDEBUG_FLUSH;
      in_stereo = false;
      viattrib[NumAttribs-1] = GLX_USE_GL; // should be a reasonable 'ignore' tag
      if( (vi = glXChooseVisual (display, screen, viattrib)) )
         return vi;
   }

   // if we reach here, we didn't.  Maybe we should make alpha optional.
   vjDEBUG(vjDBG_DRAW_MGR,0) << "WARNING: Display process for '" << display_name
              << "' couldn't get display with alpha channel - trying without.\n"
              << vjDEBUG_FLUSH;
   viattrib[AlphaAttribIndex] = 0;
   if( (vi = glXChooseVisual (display, screen, viattrib)) )
      return vi;

   // But they told me to please just go f___ myself
   // You know you just can't win.  -d. gilmour
   return NULL;
}

//!PRE:  window is an xwindow under display
//!POST: returns true if e is a mapnotify event for window, else false
//!NOTE: this is a utility function for InitGfx,  used to wait
//+       until a window has actually been mapped.
int vjGlxWindow::EventIsMapNotify (Display *display,  XEvent *e,  XPointer window) {

    return ((e->type == MapNotify) && (e->xmap.window == (Window)window));
}
