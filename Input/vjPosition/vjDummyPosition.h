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


#ifndef _VJ_DUMMYTRACKER_H_
#define _VJ_DUMMYTRACKER_H_

#include <vjConfig.h>
#include <Input/vjPosition/vjPosition.h>

//: Position derived class for impersonating a position device.
//
// vjDummyPosition is a dummy positional device for holding semi-static
// device data.
//!PUBLIC_API:
class vjDummyPosition: public vjPosition {

  public:

    //: Constructor
    vjDummyPosition() : vjPosition() { 
	active = 1;
	instName = NULL;
	mTimeStamp.set();
    }

    //: Destructor
    ~vjDummyPosition();


   virtual bool config(vjConfigChunk *c);


    //: vjInput pure virtual functions
    int startSampling();
    int stopSampling();
    int sample();
    void updateData();

    //: vjInput virtual functions
    char* getDeviceName() { return "vjDummyPosition"; }

	
    //: vjPosition pure virtual functions
    vjMatrix* getPosData (int d = 0);
    vjTimeStamp* getPosUpdateTime(int d = 0);
	
	
    //: Function to get access to the position matrix
    vjMatrix& posData()
	{
	    return mydata;
	}

    static std::string getChunkType() { return std::string("DummyPosition");}

private:
    vjMatrix mydata;     // Location data
    vjTimeStamp mTimeStamp;  // dummy stamp value;
};

#endif
