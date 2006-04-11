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


#ifndef _VJ_SYSTEM_H_
#define _VJ_SYSTEM_H_

#include <vjConfig.h>
#include <SharedMem/vjMemory.h>
#include <Sync/vjMutex.h>

//------------------------------------------------------
//:  This class holds the state of the library system
//
//-------------------------------------------------------
class vjSystem
{
// ----------- Base vjMemPool Stuff    --------- //
//  Since we need to put the singleton in Shared memory
// Nothing in this section is safe after any forking !!!
// Must be set BEFORE instance is created.
public:
    static void setBaseSize(size_t size)
    { baseMemPoolSize = size; }

    static void setBaseNumProcs(int numProcs)
    { baseNumProcs = numProcs; }

private:
    static vjSharedPool*  baseMemPool;
    static size_t baseMemPoolSize;
    static int    baseNumProcs;


//---------------------------------------------//
//------------  Singleton Stuff ---------------//
//---------------------------------------------//


// -------- Pure Singleton Stuff ------------- //
public:
    static void init()  // Just makes sure that we have been allocated
    { getInstance(); }

    static vjSystem* getInstance()
    {
   if (_instance == NULL) {
       // Called automatically --- vjSharedPool::init();   // Initialize the vjMemPoolStuff
       baseMemPool = new vjSharedPool(baseMemPoolSize, baseNumProcs);
       _instance = new vjSystem(baseMemPool);
   }
   return _instance;
    }

protected:
   // -- Constructor protected so it cannot be called except by getInstance --- //
    vjSystem()
    {

    }
private:
    static vjSystem* _instance;
};

#endif  /* _VJ_SYSTEM_H_ */
