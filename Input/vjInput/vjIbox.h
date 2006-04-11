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
//
// ibox tracking class
//
////////////////////////////////////////////////////////////////////////
#ifndef _VJ_IBOX_H_
#define _VJ_IBOX_H_

#include <vjConfig.h>
#include <Input/vjInput/vjInput.h>
#include <Input/vjInput/vjDigital.h>
#include <Input/vjInput/vjAnalog.h>
#include <Input/ibox/hci.h>
#include <Input/ibox/ibox.h>

struct vjIBOX_DATA {
   int button[4];
   int analog[4];
};

//----------------------------------------------------------------------------
//: The Immersion Box input class.
//
//  The Immersion Box is a 4 Digital, 4 Analog input device, the vjIbox class
//  therefore must inherit from both vjDigital and vjAnalog.  The class uses
//  the HCI library for a simple interface to the IBox.
//
//-----------------------------------------------------------------------------
//!PUBLIC_API:
class vjIBox : public vjDigital, public vjAnalog
{
public:
   /** @name Construction/Destruction */
   //@{
   vjIBox()
   {;}
   ~vjIBox();
   //@}

   virtual bool config(vjConfigChunk* c);

   /** @name vjInput Pure Virtual Functions */
   //@{
   int startSampling();
   int stopSampling();
   int sample();
   void updateData();
   char* getDeviceName() { return "IBox";}
   //@}

   static std::string getChunkType() { return std::string("IBox");}

   /** @name vjDigital Pure Virtual Functions */
   //@{
   int getDigitalData(int d = 0);
   //@}

   /** @name vjAnalog Pure Virtual Functions */
   //@{
   int getAnalogData(int d = 0);
   //@}


private:
   /** @name Private member variables */
   //@{
   vjIBOX_DATA theData[3];
   hci_rec thingie;
   //@}
};

#endif
