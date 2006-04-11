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

#include <gadget/gadgetConfig.h>
#include <sys/time.h>

// need stdio for sprintf
#include <stdio.h>

#include <vpr/vpr.h>
#include <vpr/System.h>
#include <jccl/Config/ConfigChunk.h>
#include <gadget/InputManager.h>
#include <gadget/Util/Debug.h>
#include <gadget/Devices/VirtualTechnologies/vt_types.h>
#include <gadget/Devices/VirtualTechnologies/vt_globals.h>
#include <gadget/Devices/VirtualTechnologies/vt_types.h>
#include <gadget/Devices/VirtualTechnologies/vt_error.h>
#include <gadget/Devices/VirtualTechnologies/CyberGlove.h>

namespace gadget
{

bool CyberGlove::config(jccl::ConfigChunkPtr c)
{
   if(! (Input::config(c) && Glove::config(c) ))
      return false;


   vprASSERT(mThread == NULL);      // This should have been set by Input(c)

   mPortName = c->getProperty<std::string>("port");
   mBaudRate = c->getProperty<int>("baud");

   char* home_dir = c->getProperty("calDir").cstring();
   if (home_dir != NULL)
   {
       mCalDir = new char [strlen(home_dir) + 1];
       strcpy(mCalDir,home_dir);
   }

   std::string glove_pos_proxy = c->getProperty("glovePos");    // Get the name of the pos_proxy
   if(glove_pos_proxy == std::string(""))
   {
      vprDEBUG(gadgetDBG_INPUT_MGR,0)
         << clrOutNORM(clrRED, "ERROR:") << " Cyberglove has no posProxy."
         << std::endl << vprDEBUG_FLUSH;
      return false;
   }

   // init glove proxy interface
   /* XXX: Doesn't appear to be used
   int proxy_index = gadget::InputManager::instance()->getProxyIndex(glove_pos_proxy);
   if(proxy_index != -1)
   {
      mGlovePos[0] = gadget::InputManager::instance()->->getPosProxy(proxy_index);
   }
   else
   {
      vprDEBUG(gadgetDBG_INPUT_MGR,0)
         << clrOutNORM(clrRED, "ERROR:")
         << " CyberGlove::CyberGlove: Can't find posProxy."
         << std::endl << std::endl << vprDEBUG_FLUSH;
   }
   */

   mGlove = new CyberGloveBasic( mCalDir, mPortName, mBaudRate );

   return true;
};

int
CyberGlove::startSampling()
{
   if (mThread == NULL)
   {
      resetIndexes();

      // Create a new thread to handle the control
      vpr::ThreadMemberFunctor<CyberGlove>* memberFunctor =
         new vpr::ThreadMemberFunctor<CyberGlove>(this, &CyberGlove::controlLoop, NULL);

      mThread = new vpr::Thread(memberFunctor);

      if (!mThread->valid())
      {
         return 0;
      }
      else
      {
         vprDEBUG(gadgetDBG_INPUT_MGR,1) << "gadget::CyberGlove is active "
                                         << std::endl << vprDEBUG_FLUSH;
         mActive = true;
         return 1;
      }
  }
  else
     return 0; // already sampling
}

// -Opens the glove port
// -Starts sampling the glove
void CyberGlove::controlLoop(void* nullParam)
{
   // Open the port and run with it
   if(mGlove->open() == 0)
   {
     vprDEBUG(gadgetDBG_INPUT_MGR,0)
        << clrOutNORM(clrRED, "ERROR:")
        << " Can't open Cyberglove or it is already opened.\n"
        << vprDEBUG_FLUSH;
     return;
   }

   // Spin in the sampling
   while(1)
   {
      sample();
   }
}

int CyberGlove::sample()
{
   mGlove->sample();       // Tell the glove to sample

   copyDataFromGlove();                   // Copy the data across
   mTheData[0][progress].calcXforms();    // Update the xform data

   swapValidIndexes();
   return 1;
}

void CyberGlove::updateData()
{
vpr::Guard<vpr::Mutex> updateGuard(lock);
   // Copy the valid data to the current data so that both are valid
   mTheData[0][current] = mTheData[0][valid];   // ASSERT: we only have one glove

   // swap the indicies for the pointers
   swapCurrentIndexes();

  return;
}

int CyberGlove::stopSampling()
{
   if (mThread != NULL)
   {
      mThread->kill();
      delete mThread;
      mThread = NULL;
      vpr::System::usleep(100);

      mGlove->close();
      vprDEBUG(gadgetDBG_INPUT_MGR,1) << "stopping CyberGlove.."
                                      << std::endl << vprDEBUG_FLUSH;
   }
   return 1;
}

CyberGlove::~CyberGlove ()
{
   stopSampling();      // Stop the glove
   delete mGlove;       // Delete the glove
}



void CyberGlove::copyDataFromGlove()
{
   CYBER_GLOVE_DATA* glove_data = mGlove->getData();     // Get ptr to data

   for(int i=0;i<GloveData::NUM_COMPONENTS;i++)
      for(int j=0;j<GloveData::NUM_JOINTS;j++)
         mTheData[0][progress].angles[i][j] = glove_data->joints[i][j];

   vprASSERT(mTheData[0][progress].angles[GloveData::MIDDLE][GloveData::MPJ]
             == glove_data->joints[MIDDLE][MCP]);
}


} // End of gadget namespace
