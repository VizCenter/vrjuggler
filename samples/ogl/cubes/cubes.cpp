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

//----------------------------------------
// Test OpenGL VR Juggler program
//
// cubes.cpp
//----------------------------------------
#include <cubesApp.h>

      // --- Lib Stuff --- //
#include <Kernel/vjKernel.h>
#include <Kernel/vjProjection.h>
//#include <unistd.h>

int main(int argc, char* argv[])
{
   vjProjection::setNearFar(0.01, 10000.0f);

   vjKernel* kernel = vjKernel::instance();        // Get the kernel
   cubesApp* application = new cubesApp(kernel);   // Declare an instance of the app

   if (argc <= 1)
   {
      // display some usage info (holding the user by the hand stuff)
      //  this will probably go away once the kernel becomes separate 
      //  and can load application plugins.
      std::cout<<"\n"<<std::flush;
      std::cout<<"\n"<<std::flush;
      std::cout<<"Usage: "<<argv[0]
               <<" vjconfigfile[0] vjconfigfile[1] ... vjconfigfile[n]\n"
               <<std::flush;
      std::cout<<"\n"<<std::flush;
      std::cout<<"\n"<<std::flush;
      exit(1);
   }
   
   // Load any config files specified on the command line
   for(int i=1;i<argc;i++)
      kernel->loadConfigFile(argv[i]);

   kernel->start();
      //- Kernel load global config  -- Environment variable
      //- App KernelConfig function
      //- Load user local config

   kernel->setApplication(application);         // Set application

   while(1)
   {
       usleep (250000);
   }
}
