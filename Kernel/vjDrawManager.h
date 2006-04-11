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


#ifndef _VJ_DRAW_MANAGER_
#define _VJ_DRAW_MANAGER_

#include <vjConfig.h>
#include <Kernel/vjConfigChunkHandler.h>

class vjDisplayManager;
class vjApp;
class vjConfigChunkDB;
class vjDisplay;
//--------------------------------------------------------
//: Abstract base class for API specific Draw Manager.
//
// Concrete classes are resonsible for all rendering.
//
// @author Allen Bierbaum
//  Date: 9-7-97
//--------------------------------------------------------
class vjDrawManager : public vjConfigChunkHandler
{
public:
   vjDrawManager (void) {
      mDisplayManager = NULL;
   }

   //: Function to initialy config API specific stuff.
   // Takes a chunkDB and extracts API specific stuff
   //**//virtual void configInitial(vjConfigChunkDB*  chunkDB) = 0;

   //: Enable a frame to be drawn
   virtual void draw() = 0;

   //: Blocks until the end of the frame
   //! POST: The frame has been drawn
   virtual void sync() = 0;

   //: Set the app the draw whould interact with.
   //! NOTE: The member variable is not in the base class
   //+    because it's "real" type is only known in the derived classes
   virtual void setApp(vjApp* _app) = 0;

   void setDisplayManager(vjDisplayManager* _dispMgr);

   //: Initialize the drawing API (if not already running)
   //! NOTE: If the draw manager should be an active object,
   //        start the process here.
   virtual void initAPI() = 0;

   //: Initialize the drawing state for the API
   // based on the data in the display manager.
   //! PRE: API is running (initAPI has been called)
   //! POST: API is ready do draw
   //virtual void initDrawing() = 0;

   //: Callback when display is added to display manager
   virtual void addDisplay(vjDisplay* disp) = 0;

   //: Callback when display is removed to display manager
   virtual void removeDisplay(vjDisplay* disp) = 0;

   //: Shutdown the drawing API
   //! NOTE: If it was an active object, kill process here
   virtual void closeAPI() = 0;

   //: Called when projections change
   // Called by display manager when projections change
   // Should do any API specific stuff.
   virtual void updateProjections()
   {;}

   friend ostream& operator<<(ostream& out, vjDrawManager& drawMgr);
   virtual void outStream(ostream& out)
   { out << "vjDrawManager: outstream\n"; }  // Set a default

protected:
   vjDisplayManager* mDisplayManager;  //: The display manager dealing with
};

#endif
