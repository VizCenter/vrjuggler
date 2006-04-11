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

#ifndef _VRJ_GL_CONTEXT_DATA_H_
#define _VRJ_GL_CONTEXT_DATA_H_
//#pragma once

#include <vrj/vrjConfig.h>
#include <vrj/Draw/OGL/GlDrawManager.h>
#include <vrj/Util/Debug.h>

namespace vrj
{

/**
 * This class is needed as a base class for GlContextData
 * because of dificulties making friends with a template.
 *
 * @note This class for internal use only.
 */
class GlContextDataBase
{
protected:
   int getCurContext()
   {
      return GlDrawManager::instance()->getCurrentContext();
   }
};


/**
 * OpenGL helper class that has templatized user context data.
 *
 * This class allows the user to specify a data type that contains
 * data that needs to have a context specific copy.  This means that there
 * is a unique copy of the data structure for each openGL context in
 * the current environment.  Juggler will take care of the data copies
 * transparently for the user so the user never has to know about the
 * current configuration. <br>
 *
 * An example use would be a struct full of display list id's.
 * The user passes their user-defined data structure as the template parameter.
 * The resulting object will be a "smart" pointer to the context specific
 * data to use. <br> <br>
 *
 * Ex: <br>
 *   GlContextData<myStruct>   myData; <br>
 *   myData->dlSphere = 0;
 *
 * @note Requires that the type of the context data provide a default
 *  constructor used to initialize all of the copies of the data.
 */
template<class ContextDataType = int>
class GlContextData : private GlContextDataBase
{
public:
   GlContextData()
   {;}

   /**
    * Returns reference to user data for the current context.
    *
    * @pre We are in a draw process.
    * @note Should only be called from the draw function.
    *        Results are un-defined for other functions.
    */
   ContextDataType& operator*()
   { return (*getPtrToCur()); }

   /**
    * Returns reference to user data for the current context.
    *
    * @pre We are in a draw process.
    * @note Should only be called from the draw function.
    *        Results are un-defined for other functions.
    */
   ContextDataType* operator->()
   { return getPtrToCur(); }

   /**
    * This function gives exclusive access to ALL copies of the
    * context-specific data.
    *
    * @note THIS CAN NOT BE USED IN A DRAW PROCESS OR VERY BAD THINGS WILL
    *       HAPPEN.  Only for EXPERT use.
    *       Needed for casses where something must be done to each
    *       copy of the data during pre-draw.
    */
   std::vector<ContextDataType*>* getDataVector()
   {
      return &mContextDataVector;
   }

protected:
   /**
    * Returns a ptr to the correct data element in the current context.
    *
    * @pre We are in the draw function.
    * @sync Synchronized.
    */
   ContextDataType*  getPtrToCur()
   {
   vpr::Guard<vpr::Mutex>  guard(mDataVectorMutex);      // Only allow a single thread in (protect resize and addition)

      // Get current context
      int context_id = getCurContext();

      // Make sure that we will reference a valid element
      while((int)mContextDataVector.size() <= context_id)
      {
         mContextDataVector.push_back(new ContextDataType());
         vprDEBUG(vrjDBG_DRAW_MGR,3)
            << "Adding ContextDataVector element: size now: "
            << mContextDataVector.size() << std::endl << vprDEBUG_FLUSH;
      }

      return mContextDataVector[context_id];
   }

private:
   std::vector<ContextDataType*> mContextDataVector;   /**< Vector of user data */
   vpr::Mutex                   mDataVectorMutex;     /**< Mutex to protect that data vector */
};

};

#endif
