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


#ifndef _VJ_CYBER_GLOVE_H_
#define _VJ_CYBER_GLOVE_H_

#include <vjConfig.h>
#include <Input/vjGlove/vjGlove.h>
#include <Input/vjGlove/CyberGloveBasic.h>

//: Cyberglove dcevice
//!PUBLIC_API:
class vjCyberGlove : virtual public vjGlove
{
public:
   //: Construct using chunk
   vjCyberGlove()
   {
      mGlove = NULL;
      mCalDir = NULL;
   }

   //: Destroy the glove
   ~vjCyberGlove ();

   virtual bool config(vjConfigChunk* c);

   virtual char* getDeviceName() { return "vjCyberGlove";}

   virtual int startSampling();
   virtual int stopSampling();
   virtual int sample();
   virtual void updateData ();

   static std::string getChunkType() { return std::string("CyberGlove");}

protected:
	//: The main control loop for the object
   void controlLoop(void* nullParam);

   void copyDataFromGlove();

protected:
   CyberGloveBasic*  mGlove;              // The actual glove
   char*             mCalDir;             // Calibration file directory
};

#endif	/* _VJ_CYBER_GLOVE_H_ */
