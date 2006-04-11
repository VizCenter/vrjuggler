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


/////////////////////////////////////////////////////////////////////////
// vjGestureProxy.h
//
// vjGesture proxy class
//
////////////////////////////////////////////////////////////////////////
#ifndef _VJ_GESTURE_PROXY_H_
#define _VJ_GESTURE_PROXY_H_

#include <vjConfig.h>
#include <math.h>
#include <assert.h>

#include <Input/vjGesture/vjGesture.h>
#include <Input/InputManager/vjProxy.h>

//: Proxy to vjGesture object
//
// A proxy is used by the user to actually acces the gesture data.
// The proxy allows the user to query the current gesture information.
// Clients call the get* routines to get the current gesture (id or string)
// Once the client has the current gesture, they should test it
// against the gestures they want to respond to.
//
// See also: vjGesture
//!PUBLIC_API:
class vjGestureProxy : public vjProxy
{
public:
   //: Construct the proxy to point to the given gesture device.
   vjGestureProxy()
   { mGesturePtr = NULL; }

   ~vjGestureProxy()
   {}

   //: Set the gloveProxy to point to another device and subUnit number.
   //! PRE: gesturePtr must point to a valid gesture device
   //! POST: this now proxies the given gesture device
   void set(vjGesture* gesturePtr)
   {
      vjASSERT( gesturePtr->fDeviceSupport(DEVICE_GESTURE) );
      mGesturePtr = gesturePtr;
   }

   //: Get the current gesture.
   //! RETURNS: id of current gesture
   int getGesture()
   { return mGesturePtr->getGesture(); }

   //: Return the identifier of the string gesture.
   //! ARGS: name - string name of a gesture
   //! RETURNS: -1 if not found
   int getGestureIndex(std::string name)
   { return mGesturePtr->getGestureIndex(name); }

   //: Get a gesture name
   //! RETURNS: Name of gesture with the given id (gestureId)
   //! NOTE: if gestureId = -1, returns name of current gesture
   std::string getGestureString(int gestureId = -1)
   { return mGesturePtr->getGestureString(gestureId); }

   //: Returns a pointer to the device held by this proxy.
   vjGesture* getGesturePtr()
   { return mGesturePtr; }

   static std::string getChunkType() { return "GestureProxy"; }

   bool config(vjConfigChunk* chunk);

   virtual vjInput* getProxiedInputDevice()
   {
      vjInput* ret_val = dynamic_cast<vjInput*>(mGesturePtr);
      vjASSERT(ret_val != NULL);
      return ret_val;
   }

private:
   //: Pointer to the gesture device we are proxying.
   vjGesture* mGesturePtr;
};

#endif
