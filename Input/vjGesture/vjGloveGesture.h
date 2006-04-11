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


#ifndef _VJ_GLOVE_GESTURE_H_
#define _VJ_GLOVE_GESTURE_H_
#pragma once

#include <vjConfig.h>
#include <Input/vjGesture/vjGesture.h>
#include <Input/vjGlove/vjGlove.h>
#include <Input/InputManager/vjGloveProxy.h>
#include <Config/vjConfigChunk.h>

//: Abstract base class for all glove gesture recognition
//
//
//!PUBLIC_API:
class vjGloveGesture : virtual public vjGesture
{
public:
   vjGloveGesture() {
      mGloveProxy = NULL;
   }

   virtual bool config(vjConfigChunk* c)
   { return vjGesture::config(c);}

public:
   //: Get a gesture name
   //! RETURNS: Name of gesture with the given id (gestureId)
   // NOTE: if gestureId = -1, returns name of current gesture
   virtual std::string getGestureString(int gestureId = -1);

   //: Create a new gesture
   //! ARGS: gestureName - Name of the new gesture
   //! RETURNS: int id of the new gesture
   virtual int createGesture(std::string gestureName);

   //: Load the header of a glove data file.
   // This is both for the samples and for the trained files.
   //
   // The header format is:
   //   -Comments -- # starting
   //   - <num gestures>
   //   -Gesture names
   //   -Gesture samples
   // infile is a file that is already open and ready for reading.
   //
   //!POST: After running, this routines will leave the file pointer immedately after
   // the header.
   //!POST: mGestureNames & mGestureExamples will be filled with correct data
   virtual void loadFileHeader(ifstream& infile);

   //: Save a file header
   //! NOTE: The client of this routine may add their own initial lines to the
   //+ header as long as they remove them before calling loadFileHeader.
   virtual void saveFileHeader(ofstream& outFile);

   //: Return the identifier of the string gesture.
   //! RETURNS: -1 if not found
   virtual int getGestureIndex(std::string gestureName);

   //: Return the number of gestures in system
   virtual int getNumGestures()
   { return mGestureNames.size(); }

protected:
   // mGestureNames and mGestureExamples MUST always be same size
   std::vector<std::string> mGestureNames;      //: List of all gesture names
   std::vector<vjGloveData> mGestureExamples;   //: Examples of all gestures
   vjGloveProxy*            mGloveProxy;        //: Proxy to the glove
};

#endif
