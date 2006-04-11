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
#include <vrj/Display/Display.h>
#include <vrj/Display/Viewport.h>
#include <vrj/Display/SimViewport.h>
#include <vrj/Display/SurfaceViewport.h>

namespace vrj
{

void Display::updateProjections()
{
   for(unsigned i=0;i<mViewports.size();i++)
   {
      mViewports[i]->updateProjections();
   }
}


void Display::config(jccl::ConfigChunkPtr chunk)
{
   vprASSERT(chunk.get() != NULL);

   configDisplayWindow(chunk);
   configViewports(chunk);
}

void Display::configDisplayWindow(jccl::ConfigChunkPtr chunk)
{
   vprASSERT(chunk.get() != NULL);

   // -- Get config info from chunk -- //
    int originX = chunk->getProperty<int>("origin", 0);
    int originY = chunk->getProperty<int>("origin", 1);
    int sizeX   = chunk->getProperty<int>("size", 0);
    int sizeY   = chunk->getProperty<int>("size", 1);
    std::string name  = chunk->getName();
    mBorder     = chunk->getProperty<bool>("border");
    int pipe    = chunk->getProperty<int>("pipe");
    mActive     = chunk->getProperty<bool>("active");
    mInStereo   = chunk->getProperty<bool>("stereo");

   // -- Check for error in configuration -- //
   // NOTE: If there are errors, set them to some default value
   if(sizeX <= 0)
   {
      vprDEBUG(vrjDBG_DISP_MGR,2) << "WARNING: window sizeX set to: " << sizeX
                                << ".  Setting to 10." << std::endl
                                << vprDEBUG_FLUSH;
      sizeX = 10;
   }

   if(sizeY <= 0)
   {
      vprDEBUG(vrjDBG_DISP_MGR,2) << "WARNING: window sizeY set to: " << sizeY
                                << ".  Setting to 10." << std::endl
                                << vprDEBUG_FLUSH;
      sizeY = 10;
   }

   if(pipe < 0)
   {
      vprDEBUG(vrjDBG_DISP_MGR,2) << "WARNING: pipe was negative, pipe set to: " << pipe << ".  Setting to 0.\n" << vprDEBUG_FLUSH;
      pipe = 0;
   }

      // -- Set local window attributes --- //
    setOriginAndSize(originX, originY, sizeX, sizeY);

    setName(name);
    setPipe(pipe);

    mDisplayChunk = chunk;        // Save the chunk for later use
}

void Display::configViewports(jccl::ConfigChunkPtr chunk)
{
   vprASSERT(chunk.get() != NULL);

   unsigned num_sim_vps = chunk->getNum("sim_viewports");
   unsigned num_surface_vps = chunk->getNum("surface_viewports");

   jccl::ConfigChunkPtr vp_chunk;
   SimViewport* sim_vp = NULL;
   SurfaceViewport* surf_vp = NULL;

   unsigned i(0);

   // Create sim viewports
   // - Set the parent display
   // - Configure it
   for(i=0;i<num_sim_vps;i++)
   {
      vp_chunk = chunk->getProperty<jccl::ConfigChunkPtr>("sim_viewports",i);
      sim_vp = new SimViewport;
      sim_vp->setDisplay(this);
      sim_vp->config(vp_chunk);
      mViewports.push_back(sim_vp);
   }

   // Create sim viewports
   // - Set the parent display
   // - Configure it
   for(i=0;i<num_surface_vps;i++)
   {
      vp_chunk = chunk->getProperty<jccl::ConfigChunkPtr>("surface_viewports",i);
      surf_vp = new SurfaceViewport;
      surf_vp->setDisplay(this);
      surf_vp->config(vp_chunk);
      mViewports.push_back(surf_vp);
   }
}


VJ_IMPLEMENT(std::ostream&) operator<<(std::ostream& out, Display& disp)
{
   out << std::setw(15) << disp.mName.c_str() << std::endl
        << "  org:" << disp._xo << ", " << disp._yo
        << "  sz:" << disp._xs << ", " << disp._ys
        << "  p:" << disp.mPipe
        << "  act:" << (disp.mActive ? "Y" : "N") << std::endl;
   for(unsigned i=0;i<disp.mViewports.size();i++)
   {
      out << "vp: " << i << " " << *(disp.mViewports[i]) << std::endl;
   }

   return out;
}
};
