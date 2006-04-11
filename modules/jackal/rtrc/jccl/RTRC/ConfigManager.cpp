/*************** <auto-copyright.pl BEGIN do not edit this line> **************
 *
 * VR Juggler is (C) Copyright 1998-2003 by Iowa State University
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

#include <jccl/jcclConfig.h>

#include <boost/filesystem/path.hpp>

#include <vpr/vpr.h>
#include <vpr/System.h>
#include <vpr/IO/Socket/InetAddr.h>

#include <jccl/Config/ConfigElement.h>
#include <jccl/Config/ElementFactory.h>
#include <jccl/RTRC/DependencyManager.h>
#include <jccl/RTRC/ConfigElementHandler.h>
#include <jccl/Util/Debug.h>
#include <jccl/RTRC/RemoteReconfig.h>
#include <jccl/RTRC/ConfigManager.h>


namespace fs = boost::filesystem;

namespace jccl
{

vprSingletonImp(ConfigManager);


ConfigManager::ConfigManager()
   : mReconfigIf(NULL)
{
   mPendingCheckCount = 0;
   mLastPendingSize = 0;

   loadRemoteReconfig();
}

/**
 * This struct implements a callable object (a functor, basically).  An
 * instance can be passed in where a boost::function1<bool, void*> is expected.
 * In jccl::ConfigManager::loadRemoteReconfig(), instances are used to handle
 * dynamic loading of plug-ins via vpr::LibraryLoader.
 */
struct Callable
{
   Callable(jccl::ConfigManager* cfgMgr) : mgr(cfgMgr)
   {
   }

   /**
    * This will be invoked as a callback by methods of vpr::LibraryLoader.
    *
    * @param func A function pointer for the entry point in a dynamically
    *             loaded plug-in.  This must be cast to the correct signature
    *             before being invoked.
    */
   bool operator()(void* func)
   {
      jccl::RemoteReconfig* (*init_func)(jccl::ConfigManager*);

      // Cast the entry point function to the correct signature so that we can
      // call it.  All dynamically plug-ins must have an entry point function
      // that takes no argument and returns a pointer to an implementation of
      // the jccl::RemoteReconfig interface.
      init_func = (jccl::RemoteReconfig* (*)(jccl::ConfigManager*)) func;

      // Call the entry point function.
      jccl::RemoteReconfig* plugin = (*init_func)(mgr);

      if ( NULL != plugin )
      {
         mgr->setRemoteReconfigPlugin(plugin);
         return true;
      }
      else
      {
         return false;
      }
   }

   jccl::ConfigManager* mgr;
};

void ConfigManager::loadRemoteReconfig()
{
   vprASSERT(NULL == mReconfigIf && "RTRC interface object already instantiated.");

   const std::string jccl_base_dir("JCCL_BASE_DIR");
   const std::string vj_base_dir("VJ_BASE_DIR");
   std::string base_dir;

   if ( ! vpr::System::getenv(jccl_base_dir, base_dir).success() )
   {
      if ( ! vpr::System::getenv(vj_base_dir, base_dir).success() )
      {
         return;
      }
   }

#if defined(_ABIN32)
   const std::string bit_suffix("32");
#elif defined(_ABI64)
   const std::string bit_suffix("64");
#else
   const std::string bit_suffix("");
#endif

   std::vector<fs::path> search_path(1);
   search_path[0] = fs::path(base_dir, fs::native) /
                       (std::string("lib") + bit_suffix) /
                       std::string("jccl") / std::string("plugins");

   // In the long run, we may not want to hard-code the base name of the
   // plug-in we load.  If we ever reach a point where we have multiple ways
   // of implementing remote run-time reconfiguration, we could have options
   // for which plug-in to load.
   const std::string reconfig_dso("corba_rtrc");
   const std::string init_func("initPlugin");
   Callable functor(this);
   mPluginLoader.findAndInitDSO(reconfig_dso, search_path, init_func, functor);
}

void ConfigManager::setRemoteReconfigPlugin(jccl::RemoteReconfig* plugin)
{
   // If we already have a remote reconfig plug-in, discard it first.
   if ( NULL != mReconfigIf )
   {
      vprDEBUG(jcclDBG_RECONFIG, vprDBG_STATE_LVL)
         << "[ConfigManager::setRemoteReconfigPlugin()] "
         << "Removing old remote reconfig plug-in\n" << vprDEBUG_FLUSH;

      if ( mReconfigIf->isEnabled() )
      {
         mReconfigIf->disable();
      }

      delete mReconfigIf;
   }

   vprDEBUG(jcclDBG_RECONFIG, vprDBG_VERB_LVL)
      << "[ConfigManager::setRemoteReconfigPlugin()] "
      << "Enabling new remote reconfig plug-in\n" << vprDEBUG_FLUSH;
   mReconfigIf = plugin;

   if ( NULL != mReconfigIf )
   {
      // Attempt to initialize the remote run-time reconfiguration component.
      if ( mReconfigIf->init().success() )
      {
         // Now, attempt to enable remote run-time reconfiguration.
         if ( ! mReconfigIf->enable().success() )
         {
            vprDEBUG(jcclDBG_RECONFIG, vprDBG_WARNING_LVL)
               << clrOutBOLD(clrYELLOW, "WARNING:")
               << " Failed to enable remote run-time reconfiguration.\n"
               << vprDEBUG_FLUSH;
            delete mReconfigIf;
            mReconfigIf = NULL;
         }
      }
      // Initialization failed.
      else
      {
         vprDEBUG(jcclDBG_RECONFIG, vprDBG_WARNING_LVL)
            << clrOutBOLD(clrYELLOW, "WARNING:")
            << " Failed to initialize remote run-time reconfiguration.\n"
            << vprDEBUG_FLUSH;
         delete mReconfigIf;
         mReconfigIf = NULL;
      }
   }
}

ConfigManager::~ConfigManager()
{
   if ( NULL != mReconfigIf && mReconfigIf->isEnabled() )
   {
      mReconfigIf->disable();

      delete mReconfigIf;
      mReconfigIf = NULL;
   }
}

//-------------------- Pending List Stuff -------------------------------

void ConfigManager::removePending(std::list<PendingElement>::iterator item)
{
   vprASSERT(1 == mPendingLock.test());
   mPendingConfig.erase(item);
}

void ConfigManager::refreshPendingList()
{
   vprASSERT(0 == mPendingCountMutex.test());
   mPendingCountMutex.acquire();
   mPendingCheckCount = 0;
   mPendingCountMutex.release();
}

static const int pending_repeat_limit = 3;    // Must be one or greater.  1 means only allow one time of no changes

// Do we need to check the pending list
// CONCURRENCY: concurrent
// The routine counts the number of pending elements each time it is called.
// if it goes pending_repeat_limit calls without changing size, then it returns
// false until mLastPendingSize changes
bool ConfigManager::pendingNeedsChecked()
{
   std::list<PendingElement>::size_type cur_pending_size(0);
   bool ret_val = false;

   mPendingCountMutex.acquire();

   cur_pending_size = mPendingConfig.size();
   if ( cur_pending_size != mLastPendingSize )
   {
      ret_val = true;                           // Flag it for a check
      mPendingCheckCount = 0;                     // Reset the counter
      mLastPendingSize = cur_pending_size;      // Keep track of size
   }
   else if ( mPendingCheckCount < pending_repeat_limit )
   {
      // allowed in at least once [1...pending_repeat_limit]
      mPendingCheckCount++;   // Increment it

      if ( mPendingCheckCount < pending_repeat_limit )
      {
         ret_val = true;        // Repeats still allowed
      }
      else
      {
         vprDEBUG(vprDBG_ALL, vprDBG_CRITICAL_LVL)
            << "ConfigManager::pendingNeedsChecked: Pending list is now\n"
            << vprDEBUG_FLUSH;
         vprDEBUG_NEXT(vprDBG_ALL, vprDBG_CRITICAL_LVL)
            << clrOutNORM(clrGREEN,"STALE: ")
            << cur_pending_size << " items still in the pending list\n"
            << vprDEBUG_FLUSH;

         if ( cur_pending_size > 0 )
         {
            vprDEBUG_NEXT(vprDBG_ALL, vprDBG_CRITICAL_LVL)
               << "NOTE: These items have been specified in the configuration\n"
               << vprDEBUG_FLUSH;
            vprDEBUG_NEXT(vprDBG_ALL, vprDBG_CRITICAL_LVL)
               << "      but have not been loaded.\n" << vprDEBUG_FLUSH;
            vprDEBUG_NEXT(vprDBG_ALL, vprDBG_CRITICAL_LVL)
               << "      This may be a problem in the configuration OR\n"
               << vprDEBUG_FLUSH;
            vprDEBUG_NEXT(vprDBG_ALL, vprDBG_CRITICAL_LVL)
               << "      it may be waiting for more configuration information.\n"
               << vprDEBUG_FLUSH;
         }

         //vprDEBUG(vprDBG_ALL, vprDBG_CRITICAL_LVL) << vprDEBUG_FLUSH;

         lockPending();
         debugDumpPending(vprDBG_CRITICAL_LVL); // Output the stale pending list
         unlockPending();

         ret_val = false;
      }
   }
   else
   {
      ret_val = false;
   }

   mPendingCountMutex.release();

   return ret_val;
}

void ConfigManager::mergeIncomingToPending()
{
   lockPending();
   {
      vpr::Guard<vpr::Mutex> g(mIncomingLock);

      for ( std::list<PendingElement>::iterator i = mIncomingConfig.begin();
            i != mIncomingConfig.end();
            ++i )
      {
         mPendingConfig.push_back(*i);
      }

      mIncomingConfig.clear();
   }
   unlockPending();
}

bool ConfigManager::isPendingStale()
{
   mPendingCountMutex.acquire();
   bool return_value = (mPendingCheckCount >= pending_repeat_limit);
   mPendingCountMutex.release();
   return return_value;
}

void ConfigManager::addPending(PendingElement& pendingElement)
{
   // Assert that the pending list is locked prior to calling this method.
   vprASSERT(1 == mPendingLock.test());

   mPendingConfig.push_back(pendingElement);

   refreshPendingList();
}

// Look for items in the active list that don't have their dependencies filled
// anymore.
//
// POST: Any elements in active with dependencies not filled are added to the
//       the pending list. (A remove and an add are added to the pending).
// RETURNS: The number of lost dependencies found.
int ConfigManager::scanForLostDependencies()
{
   vprASSERT(0 == mActiveLock.test());
   // We can't hold the lock upon entry

   vpr::DebugOutputGuard og(vprDBG_ALL, vprDBG_CONFIG_LVL,
                            "ConfigManager::scanForLostDependencies()\n",
                            "ConfigManager::scanForLostDependencies() done.\n");

   DependencyManager* dep_mgr = DependencyManager::instance();
   std::vector<ConfigElementPtr> elements;
   int num_lost_deps(0);

   // NOTE: can't hold this lock because the isSatisfied routines make
   // use of the activeLock as well
   // NOTE: Make the copy of the elements so that we can iterate without
   // fear of active changing
   mActiveLock.acquire();
   elements = mActiveConfig.vec();   // Get a copy of the elements
   mActiveLock.release();

   // Now test them
   for ( unsigned int i=0;i<elements.size();i++ )
   {
      if ( !dep_mgr->isSatisfied(elements[i]) )     // We are not satisfied
      {
         vprDEBUG_NEXT(vprDBG_ALL, vprDBG_WARNING_LVL)
            << elements[i]->getProperty<std::string>("name")
            << " type: " << elements[i]->getID()
            << " has lost dependencies.\n" << vprDEBUG_FLUSH;

         num_lost_deps++;              // Keep a count of the number lost deps found

         // Add the pending removal
         PendingElement pending;
         pending.mType = PendingElement::REMOVE;
         pending.mElement = elements[i];
         lockPending();
         addPending(pending);
         unlockPending();

         // Add the pending re-addition
//         ConfigElementPtr copy_of_element;          // Need a copy so that the remove can delete the element
//         copy_of_element = new ConfigElement(*elements[i]);
         pending.mType = PendingElement::ADD;
         pending.mElement = elements[i];//copy_of_element;
         lockPending();
         addPending(pending);                   // Add the add item
         unlockPending();
      }
   }

   return num_lost_deps;
}

void ConfigManager::debugDumpPending(int debug_level)
{
   vprASSERT(1 == mPendingLock.test());
   vprDEBUG(vprDBG_ALL,debug_level)
      << clrSetNORM(clrGREEN)
      << "---- Pending list: " << mPendingConfig.size() << " items ----\n"
      << clrRESET << vprDEBUG_FLUSH;
   std::list<ConfigManager::PendingElement>::iterator current, end;
   current = getPendingBegin();
   end = getPendingEnd();

   while ( current != end )
   {
      ConfigElementPtr cur_element = (*current).mElement;

      if ( (*current).mType == PendingElement::ADD )
      {
         vprDEBUG_NEXT(vprDBG_ALL,debug_level) << "   ADD -->" << vprDEBUG_FLUSH;
      }
      else if ( (*current).mType == PendingElement::REMOVE )
      {
         vprDEBUG_NEXT(vprDBG_ALL,debug_level) << "REMOVE -->" << vprDEBUG_FLUSH;
      }

      vprDEBUG_CONT(vprDBG_ALL,debug_level)
         << cur_element->getName() << " type: " << cur_element->getID()
         << std::endl << vprDEBUG_FLUSH;
      current++;
   }
   vprDEBUG_NEXT(vprDBG_ALL,debug_level)
      << "----------------------------------\n" << vprDEBUG_FLUSH;
}


//------------------ Active List Stuff -------------------------------

jccl::ConfigElementPtr ConfigManager::getElementFromActive(const std::string& elementName)
{
   jccl::ConfigElementPtr elt;

   vpr::Guard<vpr::Mutex> guard(mActiveLock);     // Lock the list

   std::vector<ConfigElementPtr>::iterator i;
   for ( i = mActiveConfig.vec().begin(); i != mActiveConfig.vec().end(); ++i )
   {
      if ( (*i)->getName() == elementName )
      {
         elt = *i;
         break;
      }
   }

   return elt;
}

jccl::ConfigElementPtr ConfigManager::getElementFromPending(const std::string& elementName)
{
   jccl::ConfigElementPtr elt;

   vpr::Guard<vpr::Mutex> guard(mPendingLock);     // Lock the list

   std::list<PendingElement>::iterator i;
   for ( i = mPendingConfig.begin(); i != mPendingConfig.end(); ++i )
   {
      if ( (*i).mElement->getName() == elementName )
      {
         elt = (*i).mElement;
      }
   }

   return elt;
}

jccl::ConfigElementPtr ConfigManager::getElementNamed(const std::string& elementName)
{
   jccl::ConfigElementPtr elt = getElementFromActive(elementName);

   // If the element is not in the active list, check the pending list.
   if ( elt.get() == NULL )
   {
      elt = getElementFromPending(elementName);
   }

   return elt;
}

// Is the element in the active configuration??
// CONCURRENCY: concurrent
// NOTE: This locks the active list to do processing.
bool ConfigManager::isElementInActiveList(const std::string& element_name)
{
   vpr::Guard<vpr::Mutex> guard(mActiveLock);     // Lock the current list

   std::vector<ConfigElementPtr>::iterator i;
   for ( i = mActiveConfig.vec().begin(); i != mActiveConfig.vec().end(); ++i )
   {
      if ( std::string((*i)->getName()) == element_name )
      {
         return true;
      }
   }

   return false;     // Not found, so return false
}

// Is there an element of this type in the active configuration?
// CONCURRENCY: concurrent
// NOTE: This locks the active list to do processing.
bool ConfigManager::isElementTypeInActiveList(const std::string& elementType)
{
   vpr::Guard<vpr::Mutex> guard(mActiveLock);     // Lock the current list

   std::vector<ConfigElementPtr>::iterator i;
   for ( i = mActiveConfig.vec().begin(); i != mActiveConfig.vec().end(); ++i )
   {
      if ( std::string((*i)->getID()) == elementType )
      {
         return true;
      }
   }
   return false;     // Not found, so return false
}

// Is there an element of this type in the pending list?
// CONCURRENCY: concurrent
// NOTE: This locks the pending list to do processing.
bool ConfigManager::isElementTypeInPendingList(const std::string& elementType)
{
   vpr::Guard<vpr::Mutex> guard(mPendingLock);     // Lock the current list

   std::list<PendingElement>::iterator i;
   for ( i = mPendingConfig.begin(); i != mPendingConfig.end(); ++i )
   {
      if ( std::string((*i).mElement->getID()) == elementType )
      {
         return true;
      }
   }
   return false;     // Not found, so return false
}

bool ConfigManager::hasElementType(const std::string& elementType)
{
   return isElementTypeInActiveList(elementType) ||
          isElementTypeInPendingList(elementType);
}

// Add an item to the active configuration
// NOTE: This DOES NOT process the element it just places it into the active
//       configuration list.
// PRE: Current list must NOT be locked.
void ConfigManager::addActive(ConfigElementPtr element)
{
   vprASSERT(0 == mActiveLock.test());
   lockActive();
   mActiveConfig.vec().push_back(element);
   unlockActive();
}

// Erase an item from the list.
// PRE: Active list must be locked && item must be in list.
// POST: list = old(list).erase(item) && item is invalid.
void ConfigManager::removeActive(const std::string& elementName)
{
   vprASSERT(0 == mActiveLock.test());
   lockActive();
   mActiveConfig.remove(elementName);
   unlockActive();
}

void ConfigManager::addConfigurationAdditions(jccl::Configuration* cfg)
{
   vpr::Guard<vpr::Mutex> g(mIncomingLock);

   PendingElement pending;
   pending.mType = PendingElement::ADD;

   for ( std::vector<ConfigElementPtr>::iterator i = cfg->vec().begin();
         i != cfg->vec().end();
         ++i )
   {
      pending.mElement = (*i);
      mIncomingConfig.push_back(pending);
   }
}

void ConfigManager::addConfigurationRemovals(jccl::Configuration* cfg)
{
   vpr::Guard<vpr::Mutex> g(mIncomingLock);

   PendingElement pending;
   pending.mType = PendingElement::REMOVE;

   for ( std::vector<ConfigElementPtr>::iterator i = cfg->vec().begin();
         i != cfg->vec().end();
         ++i )
   {
      pending.mElement = (*i);
      mIncomingConfig.push_back(pending);
   }
}

void ConfigManager::addConfigElement(jccl::ConfigElementPtr elt,
                                     ConfigManager::PendingElement::Type t)
{
   PendingElement pending;
   pending.mType = t;
   pending.mElement = elt;
   mIncomingConfig.push_back(pending);
}

std::list<ConfigManager::PendingElement>::size_type ConfigManager::getNumIncoming()
{
   vpr::Guard<vpr::Mutex> g(mIncomingLock);
   return mIncomingConfig.size();
}

//------------------ DynamicReconfig Stuff ------------------------------

void ConfigManager::addConfigElementHandler(ConfigElementHandler* h)
{
   mElementHandlers.push_back(h);
}

void ConfigManager::removeConfigElementHandler(ConfigElementHandler* h)
{
   std::vector<ConfigElementHandler*>::iterator it;
   for ( it = mElementHandlers.begin(); it != mElementHandlers.end(); it++ )
   {
      if ( *it == h )
      {
         mElementHandlers.erase(it);
         break;
      }
   }
}

int ConfigManager::attemptReconfiguration()
{
   int elements_processed(0);     // Needs to return this value

   // Copy the contents of the incoming config element list to the pending
   // list.  Config element handlers may have added new elements to the
   // incoming list.
   mergeIncomingToPending();

   if ( pendingNeedsChecked() )
   {
      vprDEBUG_OutputGuard(vprDBG_ALL, vprDBG_STATE_LVL,
      std::string("ConfigManager::attemptReconfiguration: Examining pending list.\n"),
      std::string("ConfigManager::attemptReconfiguration: Done Examining pending list.\n"));

      lockPending();

      for (unsigned int i = 0 ; i < mElementHandlers.size() ; i++)
      {
         elements_processed += mElementHandlers[i]->configProcessPending();
      }

      unlockPending();
   }

/*
   if ( elements_processed > 0 )
   {
      mConfigCommunicator->configChanged();
   }
*/

   return elements_processed;
}

} // namespace jccl
