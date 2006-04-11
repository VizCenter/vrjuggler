/*
 *  File:	    $RCSfile$
 *  Date modified:  $Date$
 *  Version:	    $Revision$
 *
 *
 *                                VR Juggler
 *                                    by
 *                              Allen Bierbaum
 *                             Christopher Just
 *                             Patrick Hartling
 *                            Carolina Cruz-Neira
 *                               Albert Baker
 *
 *                         Copyright  - 1997,1998,1999
 *                Iowa State University Research Foundation, Inc.
 *                            All Rights Reserved
 */


#include <vjConfig.h>

#include <Sync/vjMutex.h>
#include <Kernel/vjDebug.h>

vjMutex DebugLock;
vjDebug* vjDebug::_instance = NULL;

#include <vjConfig.h>
#include <stdlib.h>
#include <assert.h>

#include <Sync/vjMutex.h>
#include <Threads/vjThread.h>
#include <Kernel/vjStreamLock.h>



vjDebug::vjDebug()
{
   indentLevel = 0;     // Initialy don't indent
   debugLevel = 0;      // Should actually try to read env variable

   char* debug_lev = getenv("VJ_DEBUG_NFY_LEVEL");
   if(debug_lev != NULL)
   {
      debugLevel = atoi(debug_lev);
      cout << "VJ_DEBUG_NFY_LEVEL: Set to " << debugLevel << endl << flush;
   } else {
      cout << "VJ_DEBUG_NFY_LEVEL: Not found. " << endl << flush;
      cout << "VJ_DEBUG_NFY_LEVEL: Defaults to " << debugLevel << endl << flush;
   }

   setDefaultCategoryNames();
   getAllowedCatsFromEnv();

}

ostream& vjDebug::getStream(int cat, int level, int indentChange)
{
   if(indentChange < 0)                // If decreasing indent
      indentLevel += indentChange;

   //cout << "VG " << level << ": ";
   cout << vjDEBUG_STREAM_LOCK << setw(6) << vjThread::self() << "  VG: ";

      // Insert the correct number of tabs into the stream for indenting
   for(int i=0;i<indentLevel;i++)
      cout << "\t";

   if(indentChange > 0)             // If increasing indent
      indentLevel += indentChange;

   return cout;
}

void vjDebug::addCategoryName(std::string name, int cat)
{
   mCategoryNames[name] = cat;
}

void vjDebug::addAllowedCategory(int cat)
{
   if(mAllowedCategories.size() < (cat+1))
      growAllowedCategoryVector(cat+1);

   mAllowedCategories[cat] = true;
}

// Are we allowed to print this category??
bool vjDebug::isCategoryAllowed(int cat)
{
   // If now entry for cat, grow the vector
   if(mAllowedCategories.size() < (cat+1))
      growAllowedCategoryVector(cat+1);

   // If I specified to listen to all OR
   // If it has category of ALL
   if((mAllowedCategories[vjDBG_ALL]) || (cat == vjDBG_ALL))
      return true;
   else
      return mAllowedCategories[cat];
}

void vjDebug::setDefaultCategoryNames()
{
   addCategoryName(vjDBG_ALLstr,vjDBG_ALL);
   addCategoryName(vjDBG_ERRORstr,vjDBG_ERROR);
   addCategoryName(vjDBG_KERNELstr,vjDBG_KERNEL);
   addCategoryName(vjDBG_INPUT_MGRstr,vjDBG_INPUT_MGR);
   addCategoryName(vjDBG_DRAW_MGRstr,vjDBG_DRAW_MGR);
   addCategoryName(vjDBG_DISP_MGRstr,vjDBG_DISP_MGR);
   addCategoryName(vjDBG_ENV_MGRstr, vjDBG_ENV_MGR);
   addCategoryName(vjDBG_PERFORMANCEstr, vjDBG_PERFORMANCE);
   addCategoryName(vjDBG_CONFIGstr, vjDBG_CONFIG);
}

void vjDebug::getAllowedCatsFromEnv()
{
   char* dbg_cats_env = getenv("VJ_DEBUG_CATEGORIES");

   if(dbg_cats_env != NULL)
   {
      std::string dbg_cats(dbg_cats_env);

      std::map< std::string, int >::iterator i;
      for(i=mCategoryNames.begin();i != mCategoryNames.end();i++)
      {
         std::string cat_name = (*i).first;
         if (dbg_cats.find(cat_name) != std::string::npos )    // Found one
         {
            cout << "vjDEBUG::getAllowedCatsFromEnv: Allowing: " << (*i).first << " val:" << (*i).second << endl << flush;
            addAllowedCategory((*i).second);                   // Add the category
         }
      }
   }
}

void vjDebug::growAllowedCategoryVector(int newSize)
{
   while(mAllowedCategories.size() < newSize)
      mAllowedCategories.push_back(false);
}

