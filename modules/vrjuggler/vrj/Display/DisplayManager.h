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

#ifndef _VRJ_DISPLAY_MANAGER_H_
#define _VRJ_DISPLAY_MANAGER_H_

#include <vrj/vrjConfig.h>

#include <vpr/Util/Singleton.h>

//#include <gadget/Type/Position.h>
#include <vrj/Util/Debug.h>
#include <jccl/RTRC/ConfigChunkHandler.h>
#include <jccl/Config/ConfigChunk.h>

namespace vrj
{

class DrawManager;
class Display;

/**
 * Singleton Container class for all Displays.
 *
 * PURPOSE:
 * This class is responsible for holding the data about display aspects
 * of the application.  The display object are window/system independant
 * this class is in charge of holding all the display data and keeping it
 * current.  This includes updating projections, adding/deleting new displays,
 * etc.
 *
 * @date 9-7-97
 */
class VJ_CLASS_API DisplayManager : public jccl::ConfigChunkHandler
{
public:     // --- Config stuff -- //
   /**
    * Adds the chunk to the configuration.
    * @pre configCanHandle(chunk) == true
    */
   virtual bool configAdd(jccl::ConfigChunkPtr chunk);

   /**
    * Removes the chunk from the current configuration.
    * @pre configCanHandle(chunk) == true
    */
   virtual bool configRemove(jccl::ConfigChunkPtr chunk);

   /**
    * Can the handler handle the given chunk?
    *
    * @return true if we can handle it, false if we can't.
    */
   virtual bool configCanHandle(jccl::ConfigChunkPtr chunk);

public:
   /**
    * This function updates the projections for all contained displays.
    * It also tells the Draw Manager that the projections have changed.
    */
   virtual void updateProjections();

   /**
    * Sets the Draw Manager that the system is running.
    * We need to know this in order to notify the Draw Manager of any display
    * changes.
    *
    * @post Draw Manager is notified of any displays currently configured.
    */
   void setDrawManager(DrawManager* drawMgr);

   /**
    * Returns a list of the current displays.
    * @note DO NOT EDIT THE DISPLAYS
    */
   std::vector<vrj::Display*> getActiveDisplays()
   { return mActiveDisplays;}

   /**
    * Returns list of inactive displays.
    * @note DO NOT EDIT THE DISPLAYS
    */
   std::vector<vrj::Display*> getInActiveDisplays()
   { return mInactiveDisplays;}

   /**
    * Returns list of all displays (inactive and active).
    * @note DO NOT EDIT THE DISPLAYS
    */
   std::vector<vrj::Display*> getAllDisplays();

   jccl::ConfigChunkPtr getDisplaySystemChunk();

private:
   /**
    * Adds the chunk to the configuration.
    *
    * @pre configCanHandle(chunk) == true
    * @post (display of same name already loaded) ==> old display closed, new one opened<br>
    *       (display is new) ==> (new display is added)<br>
    *       Draw Manager is notified of the display change.
    */
   bool configAddDisplay(jccl::ConfigChunkPtr chunk);

   /**
    * Removes the chunk from the current configuration.
    *
    * @pre configCanHandle(chunk) == true
    * @return success
    */
   bool configRemoveDisplay(jccl::ConfigChunkPtr chunk);


   /**
    * Adds a display to the current system.
    *
    * @pre disp is a valid display
    * @post disp has been added to the list of displays
    *   (notifyDrawMgr == true) && (drawMgr != NULL) && (disp is active)
    *   ==> Draw manager now has been given new window to display
    */
   int addDisplay(vrj::Display* disp, bool notifyDrawMgr = true);

   /**
    * Closes the given display.
    * @pre disp is a display we know about
    * @post disp has been removed from the list of displays
    *    (notifyDrawMgr == true) && (drawMgr != NULL) && (disp is active)
    *    ==> Draw manager has been told to clode the window for the display
    */
   int closeDisplay(vrj::Display* disp, bool notifyDrawMgr = true);

   /// Is the display a member of the display manager
   bool isMemberDisplay(vrj::Display* disp);

   /**
    * Finds a display given the display name.
    * @return NULL if nothing found
    */
   vrj::Display* findDisplayNamed(std::string name);

public:
   std::vector<vrj::Display*> mActiveDisplays; /**< List of currently active displays */
   std::vector<vrj::Display*> mInactiveDisplays; /**< List of currently inactive displays */

protected:
   DrawManager*    mDrawManager;           /**< The current Draw Manager to communicate with */
   jccl::ConfigChunkPtr    mDisplaySystemChunk;    /**< Config chunk for the displaySystem */



protected:
   DisplayManager() : mDrawManager(NULL)
   {
       ;
   }

   virtual ~DisplayManager()
   {;}

   DisplayManager (const DisplayManager& o) {;}
   void operator= (const DisplayManager& o) {;}

   vprSingletonHeader( DisplayManager );
};

} // end namespace vrj
#endif
