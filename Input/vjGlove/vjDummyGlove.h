#ifndef _VJ_DUMMY_GLOVE_H_
#define _VJ_DUMMY_GLOVE_H_

#include <vjConfig.h>
#include <Input/vjGlove/vjGlove.h>

//: A dummy glove for the system
// This class is used only when there is a proxy and it must be
// stupified.
//
// It returns the data in it's default init.
// i.e. 0 angles, and ident matrices
class vjDummyGlove : public vjGlove
{
public:
   //: Construct using chunk
   vjDummyGlove() : vjGlove()
   {
      active = 1;
      resetIndexes();
      mTheData[0][current].calcXforms();
   }

   //: Destroy the glove
   ~vjDummyGlove ()
   {;}

   virtual char* GetDeviceName() { return "vjDummyGlove";}

   virtual int StartSampling() { return 1; }
   virtual int StopSampling() { return 1; }
   virtual int Sample() { return 1; }
   virtual void UpdateData () {}

   void setGlovePosProxy(vjPosProxy* posProxy)
   { mGlovePos[0] = posProxy; }

};

#endif	/* _VJ_CYBER_GLOVE_H_ */
