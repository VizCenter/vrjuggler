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

#include <vrj/vrjConfig.h>

#include <jccl/Config/ConfigChunk.h>
#include <vrj/Display/Viewport.h>
#include <vrj/Kernel/Kernel.h>

namespace vrj
{

void Viewport::config(jccl::ConfigChunkPtr chunk)
{
   vprASSERT(chunk.get() != NULL);

   // -- Get config info from chunk -- //
    float originX = chunk->getProperty<float>("origin", 0);
    float originY = chunk->getProperty<float>("origin", 1);
    float sizeX   = chunk->getProperty<float>("size", 0);
    float sizeY   = chunk->getProperty<float>("size", 1);
    std::string name  = chunk->getName();
    mView    = (Viewport::View)chunk->getProperty<int>("view");
    mActive  = chunk->getProperty<bool>("active");

   // -- Check for error in configuration -- //
   // NOTE: If there are errors, set them to some default value
   if(sizeX <= 0)
   {
      vprDEBUG(vrjDBG_DISP_MGR,2) << "WARNING: viewport sizeX set to: " << sizeX
                                << ".  Setting to 1." << std::endl
                                << vprDEBUG_FLUSH;
      sizeX = 1.0f;
   }

   if(sizeY <= 0)
   {
      vprDEBUG(vrjDBG_DISP_MGR,2) << "WARNING: viewport sizeY set to: " << sizeY
                                << ".  Setting to 1." << std::endl
                                << vprDEBUG_FLUSH;
      sizeY = 1.0f;
   }

    // -- Set local window attributes --- //
    setOriginAndSize(originX, originY, sizeX, sizeY);

    // Get the user for this display
    std::string user_name = chunk->getProperty<std::string>("user");
    mUser = Kernel::instance()->getUser(user_name);

    if(NULL == mUser)
    {
       vprDEBUG(vprDBG_ERROR,0) << clrOutNORM(clrRED, "ERROR:") << " User not found named: "
                              << user_name.c_str() << std::endl
                              << vprDEBUG_FLUSH;
      vprASSERT(false && "User not found in Viewport::config");
    }

    setName(name);
    mViewportChunk = chunk;        // Save the chunk for later use

    std::string bufname = "Head Latency " + name;
    mLatencyMeasure.setName (bufname);
}

std::ostream& operator<<(std::ostream& out, Viewport& viewport)
{
   return viewport.outStream(out);
}

std::ostream& Viewport::outStream(std::ostream& out)
{
   out << "user: " << getName()
       << "  org:" << mXorigin << ", " << mYorigin
       << "  sz:" << mXsize << ", " << mYsize
       << "  view:" << ((mView == Viewport::LEFT_EYE) ? "Left" : ((mView==Viewport::RIGHT_EYE)?"Right" : "Stereo") )
       << std::flush;

   return out;
}


};
