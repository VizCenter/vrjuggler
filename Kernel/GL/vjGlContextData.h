#ifndef _VJ_GL_CONTEXT_DATA_H_
#define _VJ_GL_CONTEXT_DATA_H_
#pragma once

#include <Kernel/GL/vjGlDrawManager.h>

class vjGlContextDataBase
{
protected:
   int getCurContext()
   {
      return vjGlDrawManager::instance()->currentContext();
   }
};


//-----------------------------------------------------------------------
//: OpenGL helper class that has templatized user context data
//
//  This class allows the user to specify a data type that contains
// data that needs to have a context specific copy.  For example a struct
// full of display list id's.  The user passes their user-defined data
// structure as the template parameter.
//  The resulting object will be a "smart" pointer to the context specific
// data to use.
//
//! NOTE: Requires that the type of the context data provide a default
//+ constructor used to initialize all of the copies of the data.
//-----------------------------------------------------------------------
template<class ContextDataType = int>
class vjGlContextData : public vjGlContextDataBase
{
public:
   vjGlContextData()
   {;}

   //: Returns reference to user data for the current context
   //! PRE: We are in a draw process
   //! NOTE: Should only be called from the draw function.
   //+       Results are un-defined for other functions.
   ContextDataType& operator*()
   { return (*getPtrToCur()); }

   ContextDataType* operator->()
   { return getPtrToCur(); }

protected:
   //: Return a ptr to the correct data element in the current context
   //! PRE: We are in the draw function
   ContextDataType*  getPtrToCur()
   {
      // Get current context
      int context_id = getCurContext();

      // Make sure that we will reference a valid element
      while(mContextDataVector.size() <= context_id)
      {
         mContextDataVector.push_back(ContextDataType());
         vjDEBUG(0) << "Adding ContextDataVector element: size now: " << mContextDataVector.size() << endl << vjDEBUG_FLUSH;
      }

      return &mContextDataVector[context_id];
   }

private:
   vector<ContextDataType>    mContextDataVector;     //: Vector of user data
};

#endif
