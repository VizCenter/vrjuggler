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
// Test Osg VR Juggler program
//
// main.cpp
//----------------------------------------
#include <OsgNav.h>

      // --- Lib Stuff --- //
#include <Kernel/vjKernel.h>

#ifndef WIN32
#include <sched.h>
#endif

//#define OSG_USE_IO_DOT_H


int main(int argc, char* argv[])
{
   vjKernel* kernel = vjKernel::instance();           // Get the kernel
   OsgNav* application = new OsgNav(kernel);       // Instantiate an instance of the app

   if (argc <= 2)
   {
      // display some usage info (holding the user by the hand stuff)
      //  this will probably go away once the kernel becomes separate 
      //  and can load application plugins.
      std::cout<<"\n"<<flush;
	   std::cout<<"\n"<<flush;

      std::cout<<"Usage: "<<argv[0]<<" modelname vjconfigfile[0] vjconfigfile[1] ... vjconfigfile[n]\n"<<flush;
      
      std::cout<<"\n"<<flush;
      std::cout<<"\n"<<flush;
      exit(1);
   }
   
   application->setModelFileName(std::string(argv[1]));
   
   // Load any config files specified on the command line
   for(int i=2;i<argc;i++)
      kernel->loadConfigFile(argv[i]);

   kernel->start();

   /*
   for(int i=0;i<20;i++)
   {
      usleep(50000);
      cout << i << "." << flush;
   }
   */

   kernel->setApplication(application);

   while(1)
   {
      usleep(250000);
   }
   return (1);
}
