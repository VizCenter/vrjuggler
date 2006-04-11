/****************** <SNX heading BEGIN do not edit this line> *****************
 *
 * sonix
 *
 * Original Authors:
 *   Kevin Meinert
 *
 * -----------------------------------------------------------------
 * File:          $RCSfile$
 * Date modified: $Date$
 * Version:       $Revision$
 * -----------------------------------------------------------------
 *
 ****************** <SNX heading END do not edit this line> ******************/

/*************** <auto-copyright.pl BEGIN do not edit this line> **************
 *
 * VR Juggler is (C) Copyright 1998-2005 by Iowa State University
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
 *************** <auto-copyright.pl END do not edit this line> ***************/

#include <snx/snxConfig.h>

#include <iostream>
#include <fstream>
#include <string>

#include <boost/filesystem/exception.hpp>
#include <boost/filesystem/operations.hpp>

#include <vpr/vpr.h>
#include <vpr/System.h>
#include <vpr/Util/FileUtils.h>
#include <vpr/DynLoad/LibraryFinder.h>

#include <snx/Util/Debug.h>
#include "snx/ISoundImplementation.h"
#include "snx/StubSoundImplementation.h" // in case lookup fails...

#include "snx/SoundFactory.h" // my header.


namespace snx
{

/**
 * Create the plugin.
 * Symbol name to look up from the shared lib is "newPlugin".
 */
typedef snx::ISoundImplementation* (*newPluginFunc)(void);

/**
 * Gets the name of the plugin.
 * Symbol name to lookup from the shared lib is "getName".
 */
typedef char* (*getNameFunc)(void);

/**
 * Get the version of sonix plugin was compiled against.
 * Symbol name to lookup from the shared lib is "getVersion".
 */
typedef char* (*getVersionFunc)(void);

vprSingletonImp(SoundFactory);

SoundFactory::SoundFactory()
{
   std::vector<std::string> search_paths;

   const std::string envvar("SNX_BASE_DIR");
   std::string dummy_result;

   if ( vpr::System::getenv(envvar, dummy_result).failure() )
   {
      vprDEBUG(snxDBG, vprDBG_WARNING_LVL)
         << clrOutBOLD(clrYELLOW, "WARNING:")
         << " SNX_BASE_DIR environment variable not set!\n" << vprDEBUG_FLUSH;
      vprDEBUG_NEXT(snxDBG, vprDBG_WARNING_LVL)
         << "Sonix plug-in loading may fail.\n" << vprDEBUG_FLUSH;
   }

#if defined(VPR_OS_IRIX) && defined(_ABIN32)
   std::string snx_lib_dir("${SNX_BASE_DIR}/lib32/snx/plugins");
#elif defined(VPR_OS_IRIX) && defined(_ABI64) || \
      defined(VPR_OS_Linux) && defined(__x86_64__)
   std::string snx_lib_dir("${SNX_BASE_DIR}/lib64/snx/plugins");
#else
   std::string snx_lib_dir("${SNX_BASE_DIR}/lib/snx/plugins");
#endif

#ifdef SNX_DEBUG
   snx_lib_dir += std::string("/dbg");
#else
   snx_lib_dir += std::string("/opt");
#endif

#if defined(VPR_OS_Windows)
   const std::string driver_ext("dll");
#elif defined(VPR_OS_Darwin)
   const std::string driver_ext("dylib");
#else
   const std::string driver_ext("so");
#endif

   search_paths.push_back(snx_lib_dir);
   search_paths.push_back( "${HOME}/.sonix/plugins" );

   for (unsigned int x = 0; x < search_paths.size(); ++x)
   {
      search_paths[x] = vpr::replaceEnvVars(search_paths[x]);
      vprDEBUG(snxDBG, vprDBG_CONFIG_LVL) << "Finding plugins in "
                                          << search_paths[x] << std::endl
                                          << vprDEBUG_FLUSH;

      try
      {
         boost::filesystem::path dirPath(search_paths[x], boost::filesystem::native);
         if (boost::filesystem::exists(dirPath))
         {
            vpr::LibraryFinder finder(search_paths[x], driver_ext);
            vpr::LibraryFinder::LibraryList libs = finder.getLibraries();
            this->loadPlugins( libs );

#ifdef SNX_DEBUG
            vprDEBUG(snxDBG, vprDBG_CONFIG_LVL) << "filelist:\n"
                                                << vprDEBUG_FLUSH;
            for ( unsigned int i = 0; i < libs.size(); ++i )
            {
               vprDEBUG(snxDBG, vprDBG_CONFIG_LVL) << "\t" << libs[i]
                                                   << std::endl
                                                   << vprDEBUG_FLUSH;
            }
#endif
         }
         else
         {
            vprDEBUG(snxDBG, vprDBG_STATE_LVL)
               << "The directory does not exist: '" << search_paths[x] << "'\n"
               << vprDEBUG_FLUSH;
         }
      }
      catch (boost::filesystem::filesystem_error& fsEx)
      {
         vprDEBUG(snxDBG, vprDBG_CRITICAL_LVL)
            << clrOutNORM(clrRED, "ERROR:")
            << " File system exception caught: " << fsEx.what() << std::endl
            << vprDEBUG_FLUSH;
      }
   }
}

void SoundFactory::errorOutput(vpr::LibraryPtr lib, const char* test)
{
   vprDEBUG(snxDBG, vprDBG_WARNING_LVL)
      << "ERROR: Plugin '" << lib->getName() << "' does not export symbol '"
      << test << "'\n" << vprDEBUG_FLUSH;
}

bool SoundFactory::isPlugin(vpr::LibraryPtr lib)
{
   if (lib->findSymbol( "newPlugin" ) == NULL)
   {
      errorOutput(lib, "newPlugin");
      return false;
   }
   if (lib->findSymbol( "getVersion" ) == NULL)
   {
      errorOutput(lib, "getVersion");
      return false;
   }
   if (lib->findSymbol( "getName" ) == NULL)
   {
      errorOutput(lib, "getName");
      return false;
   }

   // @todo give sonix an internal version number string!
   //getVersionFunc getVersion = (getVersionFunc)lib.lookup( "getVersion" );
   //if (getVersion != NULL && getVersion() != snx::version) return false;
   vprDEBUG(snxDBG, vprDBG_CONFIG_STATUS_LVL) << "Plugin '" << lib->getName()
                                              << "' is a valid Sonix plugin.\n"
                                              << vprDEBUG_FLUSH;
   return true;
}

void SoundFactory::loadPlugins( vpr::LibraryFinder::LibraryList libs )
{
   for (unsigned int x = 0; x < libs.size(); ++x)
   {
      // open the library
      vpr::ReturnStatus didLoad = libs[x]->load();

      if (didLoad == vpr::ReturnStatus::Succeed)
      {
        //If the plug-in implements the nessiasry interface
        if ( this->isPlugin(libs[x]) )
        {
            // get the name..
            getNameFunc getName = (getNameFunc)libs[x]->findSymbol( "getName" );
            std::string name;
            if (getName != NULL)
            {
                name = getName();

                // Check to see it plug-in is already registered
                int pluginsWithName =  mRegisteredImplementations.count(name);
                if ( pluginsWithName < 1 )
                {
                    // Keep track of the plug-ins we actual use
                    mPlugins.push_back(libs[x]);

                    vprDEBUG(snxDBG, vprDBG_STATE_LVL)
                       << "Got plug-in '" << name << "'--registering\n"
                       << vprDEBUG_FLUSH;

                    // create the implementation
                    newPluginFunc newPlugin = (newPluginFunc)libs[x]->findSymbol( "newPlugin" );
                    ISoundImplementation* si = NULL;
                    if (newPlugin != NULL)
                    {
                        si = newPlugin();
                        if (NULL != si)
                        {
                            this->reg( name, si );
                        }
                    }
                }
                else
                {
                    //Plug in was already registered so we can unload it
                    libs[x]->unload();
                }
            }
        }
      }
      else
      {
        //Lib failed to load
        vprDEBUG(snxDBG, vprDBG_WARNING_LVL)
            << "ERROR: Plugin '" << libs[x]->getName() << "' Failed to load\n" << vprDEBUG_FLUSH;
      }
   }
}

void SoundFactory::unloadPlugins()
{
   for (unsigned int x = 0; x < mPlugins.size(); ++x)
   {
      // get the name..
      getNameFunc getName = (getNameFunc)mPlugins[x]->findSymbol( "getName" );
      std::string name;
      ISoundImplementation* impl = NULL;
      if (getName != NULL)
      {
         name = getName();
         impl = mRegisteredImplementations[name];

         // unreg it.
         this->reg( name, NULL );
      }

      // delete the memory using the delete func that comes with the library...
      if ( NULL != impl )
      {
         delete impl;
      }

      // close the library
      mPlugins[x]->unload();
   }
   mPlugins.clear();
}

void SoundFactory::reg(const std::string& apiName,
                       snx::ISoundImplementation* impl)
{
   if (impl != NULL)
   {
      vprDEBUG(snxDBG, vprDBG_CONFIG_LVL)
         << "NOTICE: Registering sound API '" << apiName << "'\n"
         << vprDEBUG_FLUSH;

      impl->setName( apiName );
      mRegisteredImplementations[apiName] = impl;
   }
   else
   {
      mRegisteredImplementations.erase( apiName );

      vprDEBUG(snxDBG, vprDBG_CONFIG_LVL)
         << "NOTICE: Unregistered sound API '" << apiName << "'\n"
         << vprDEBUG_FLUSH;
   }
}

// Factory function used to create an implementation of a sound API.
void SoundFactory::createImplementation( const std::string& apiName,
                           snx::ISoundImplementation* &mImplementation )
{
   if (mRegisteredImplementations.count( apiName ) > 0)
   {
      mRegisteredImplementations[apiName]->clone( mImplementation );
   }

   else
   {
      mImplementation = new snx::StubSoundImplementation;
   }

   //mImplementation->setName( apiName );
}

} // end namespace
