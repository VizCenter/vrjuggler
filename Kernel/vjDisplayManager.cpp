/*************** <auto-copyright.pl BEGIN do not edit this line> **************
 *
 * VR Juggler is (C) Copyright 1998, 1999, 2000, 2001, 2002
 *   by Iowa State University
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


#include <vjConfig.h>
#include <Kernel/vjDisplayManager.h>
#include <Kernel/vjDisplay.h>
#include <Kernel/vjSurfaceDisplay.h>
#include <Kernel/vjSimDisplay.h>
#include <Kernel/vjDrawManager.h>
#include <Kernel/vjKernel.h>
#include <Math/vjCoord.h>

//vjDisplayManager* vjDisplayManager::_instance = NULL;
vjSingletonImp(vjDisplayManager);

std::vector<vjDisplay*> vjDisplayManager::getAllDisplays()
{
   std::vector<vjDisplay*> ret_val;
   ret_val.insert(ret_val.end(), mActiveDisplays.begin(), mActiveDisplays.end());
   ret_val.insert(ret_val.end(), mInactiveDisplays.begin(), mInactiveDisplays.end());
   return ret_val;
}


void vjDisplayManager::setDrawManager(vjDrawManager* drawMgr)
{
   vjDEBUG(vjDBG_DISP_MGR,3) << "vjDisplayManager: Setting draw manager.\n" << vjDEBUG_FLUSH;

   // set the draw manager
   mDrawManager = drawMgr;

   // Alert the draw manager about all the active windows currently configured
   if(mDrawManager != NULL)
   {
      for(unsigned int i=0;i<mActiveDisplays.size();i++)
      {
         mDrawManager->addDisplay(mActiveDisplays[i]);
      }
   }
}

//: Add the chunk to the configuration
//! PRE: configCanHandle(chunk) == true
bool vjDisplayManager::configAdd(vjConfigChunk* chunk)
{
   vjASSERT(configCanHandle(chunk));

   std::string chunk_type = (std::string)chunk->getType();

   if(   (chunk_type == std::string("surfaceDisplay"))
      || (chunk_type == std::string("simDisplay")) )
   {
      return configAddDisplay(chunk);
   }
   else if(chunk_type == std::string("displaySystem"))
   {
      // XXX: Put signal here to tell draw manager to lookup new stuff
      mDisplaySystemChunk = chunk;     // Keep track of the display system chunk
      return true;                     // We successfully configured.
                                       // This tell processPending to add it to the active config
   }
   else
   { return false; }
}

//: Remove the chunk from the current configuration
//! PRE: configCanHandle(chunk) == true
bool vjDisplayManager::configRemove(vjConfigChunk* chunk)
{
   vjASSERT(configCanHandle(chunk));

   std::string chunk_type = (std::string)chunk->getType();

   if(  (chunk_type == std::string("surfaceDisplay"))
     || (chunk_type == std::string("simDisplay")) )
   {
      return configRemoveDisplay(chunk);
   }
   else if(chunk_type == std::string("displaySystem"))
   {
      // XXX: Put signal here to tell draw manager to lookup new stuff
      mDisplaySystemChunk = NULL;     // Keep track of the display system chunk
      return true;                     // We successfully configured.
                                       // This tell processPending to remove it to the active config
   }
   else
   { return false; }

}


//: Is it a display chunk?
//! RETURNS: true - We have a display chunk
//+          false - We don't
bool vjDisplayManager::configCanHandle(vjConfigChunk* chunk)
{
   return (    ((std::string)chunk->getType() == std::string("surfaceDisplay"))
            || ((std::string)chunk->getType() == std::string("simDisplay"))
            || ((std::string)chunk->getType() == std::string("displaySystem")) );
}




//: Add the chunk to the configuration
//! PRE: configCanHandle(chunk) == true
//! POST: (display of same name already loaded) ==> old display closed, new one opened
//+       (display is new) ==> (new display is added)
//+       draw manager is notified of the display change
bool vjDisplayManager::configAddDisplay(vjConfigChunk* chunk)
{
   vjASSERT(configCanHandle(chunk));      // We must be able to handle it first of all

   vjDEBUG_BEGIN(vjDBG_DISP_MGR,vjDBG_STATE_LVL) << "------- vjDisplayManager::configAddDisplay -------\n" << vjDEBUG_FLUSH;

   // Find out if we already have a window of this name
   // If so, then close it before we open a new one of the same name
   // This basically allows re-configuration of a window
   vjDisplay* cur_disp = findDisplayNamed(chunk->getProperty("name"));
   if(cur_disp != NULL)                         // We have an old display
   {
      vjDEBUG(vjDBG_DISP_MGR,vjDBG_CONFIG_LVL) << "Removing old window: " << cur_disp->getName().c_str() << vjDEBUG_FLUSH;
      closeDisplay(cur_disp,true);              // Close the display and notify the draw manager to close the window
   }

   // --- Add a display (of the correct type) ---- //
   if((std::string)chunk->getType() == std::string("surfaceDisplay"))      // Surface DISPLAY
   {
      vjDisplay* newDisp = new vjSurfaceDisplay();    // Create display
      newDisp->config(chunk);                         // Config it
      addDisplay(newDisp, true);                            // Add it
      vjDEBUG(vjDBG_DISP_MGR,vjDBG_STATE_LVL) << "Adding display: "
                                                << newDisp->getName().c_str()
                                                << std::endl << vjDEBUG_FLUSH;
      vjDEBUG(vjDBG_DISP_MGR,vjDBG_STATE_LVL) << "Display: "  << newDisp
                                             << std::endl << vjDEBUG_FLUSH;
   }

   if((std::string)chunk->getType() == std::string("simDisplay"))      // Surface DISPLAY
   {
      vjDisplay* newDisp = new vjSimDisplay();     // Create display
      newDisp->config(chunk);                      // Config it
      addDisplay(newDisp, true);                         // Add it
      vjDEBUG(vjDBG_DISP_MGR,vjDBG_WARNING_LVL) << "Adding Display: "
                                                << newDisp->getName().c_str()
                                                << std::endl << vjDEBUG_FLUSH;
      vjDEBUG(vjDBG_DISP_MGR,vjDBG_VERB_LVL) << "Display: " << newDisp
                                             << std::endl << std::flush
                                             << vjDEBUG_FLUSH;
   }

   vjDEBUG_END(vjDBG_DISP_MGR,vjDBG_STATE_LVL) << "------- vjDisplayManager::configAddDisplay Done. --------\n" << vjDEBUG_FLUSH;
   return true;
}

//: Remove the chunk from the current configuration
//! PRE: configCanHandle(chunk) == true
//!RETURNS: success
bool vjDisplayManager::configRemoveDisplay(vjConfigChunk* chunk)
{
   vjASSERT(configCanHandle(chunk));      // We must be able to handle it first of all

   vjDEBUG_BEGIN(vjDBG_DISP_MGR,4) << "------- vjDisplayManager::configRemoveDisplay -------\n" << vjDEBUG_FLUSH;

   bool success_flag(false);

   if((std::string)chunk->getType() == std::string("surfaceDisplay") ||
      (std::string)chunk->getType() == std::string("simDisplay"))      // It is a display
   {
      vjDisplay* remove_disp = findDisplayNamed(chunk->getProperty("name"));
      if(remove_disp != NULL)
      {
         closeDisplay(remove_disp, true);                            // Remove it
         success_flag = true;
      }
   }

   vjDEBUG_END(vjDBG_DISP_MGR,4) << "------- vjDisplayManager::configRemoveDisplay done. --------\n" << vjDEBUG_FLUSH;
   return success_flag;
}




// notifyDrawMgr = 0; Defaults to 0
int vjDisplayManager::addDisplay(vjDisplay* disp, bool notifyDrawMgr)
{
   vjDEBUG(vjDBG_DISP_MGR,4) << "vjDisplayManager::addDisplay \n" << vjDEBUG_FLUSH;

   // Test if active or not, to determine correct list
   // The place it in the list
   // --- Update Local Display structures
   if(disp->isActive())
      mActiveDisplays.push_back(disp);
   else
      mInactiveDisplays.push_back(disp);

   // If we are supposed to notify about, and valid draw mgr, and disp is active
   if((notifyDrawMgr) && (mDrawManager != NULL) && (disp->isActive()))
      mDrawManager->addDisplay(disp);;    // Tell Draw Manager to add dislay;

   return 1;
}

//: Close the given display
//! PRE: disp is a display we know about
//! POST: disp has been removed from the list of displays
//+   (notifyDrawMgr == true) && (drawMgr != NULL) && (disp is active)
//+   ==> Draw manager has been told to clode the window for the display
int vjDisplayManager::closeDisplay(vjDisplay* disp, bool notifyDrawMgr)
{
   vjASSERT(isMemberDisplay(disp));       // Make sure that display actually exists

   // Notify the draw manager to get rid of it
   if((notifyDrawMgr) && (mDrawManager != NULL) && (disp->isActive()))
      mDrawManager->removeDisplay(disp);

   // Remove it from local data structures
   unsigned int num_before_close = mActiveDisplays.size() + mInactiveDisplays.size();
   mActiveDisplays.erase( std::remove(mActiveDisplays.begin(), mActiveDisplays.end(), disp),
                          mActiveDisplays.end());
   mInactiveDisplays.erase( std::remove(mInactiveDisplays.begin(), mInactiveDisplays.end(), disp),
                            mInactiveDisplays.end());
   vjASSERT(num_before_close == (1+mActiveDisplays.size() + mInactiveDisplays.size()));

   // Delete the object
   // XXX: Memory leak.  Can't delete display here because the draw manager
   //      may need access to it when the draw manager closes the window up.
   // ex: the glDrawManager window has ref to the display to get current user, etc.
   //XXX//delete disp;

   return 1;
}


// Is the display a member of the display manager
bool vjDisplayManager::isMemberDisplay(vjDisplay* disp)
{
   std::vector<vjDisplay*>::iterator i;

   i = std::find(mActiveDisplays.begin(),mActiveDisplays.end(),disp);
   if(i != mActiveDisplays.end())
      return true;

   i = std::find(mInactiveDisplays.begin(),mInactiveDisplays.end(),disp);
   if(i != mInactiveDisplays.end())
      return true;

   return false;  // Didn't find any
}

//: Find a display given the display name
//! RETURNS: NULL - not found
vjDisplay* vjDisplayManager::findDisplayNamed(std::string name)
{
   std::vector<vjDisplay*>::iterator i;

   for(i = mActiveDisplays.begin();i!=mActiveDisplays.end();i++)
      if((*i)->getName() == name)
         return (*i);

   for(i = mInactiveDisplays.begin();i!=mInactiveDisplays.end();i++)
      if((*i)->getName() == name)
         return (*i);

   return NULL;  // Didn't find any
}


void vjDisplayManager::updateProjections()
{
   // for (all displays) update the projections
   for (std::vector<vjDisplay*>::iterator i = mActiveDisplays.begin(); i != mActiveDisplays.end(); i++)
      (*i)->updateProjections();

   for (std::vector<vjDisplay*>::iterator j = mInactiveDisplays.begin(); j != mInactiveDisplays.end(); j++)
      (*j)->updateProjections();

   if(mDrawManager != NULL)
      mDrawManager->updateProjections();
}


