/*
 * VRJuggler
 *   Copyright (C) 1997,1998,1999,2000
 *   Iowa State University Research Foundation, Inc.
 *   All Rights Reserved
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
 */

 
#ifndef _ASCENSION_MOTION_STAR_H_
#define _ASCENSION_MOTION_STAR_H_

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <time.h>

#define MYPORT 4000 /* Indy port number */ 
#define TCP_PORT 6000 /* bird server port */
#define TRUE 1 /* true value */
#define FALSE 0 /* false value */ 

struct SINGLE_BIRD_STATUS { 
  unsigned char status; 
  unsigned char id;
  unsigned short int            softwareRev;
  unsigned char         errorCode;
  unsigned char         setup;
  unsigned char         dataFormat;
  unsigned char         reportRate;
  unsigned short int    scaling;
  unsigned char         hemisphere;
  unsigned char         FBBaddress;
  unsigned short int    spare1;
  unsigned short int    spare2;
};
  
struct FILTER_TABLE
{
  unsigned short int    entry[7];
};
 
struct ANGLES_TABLE
{
  unsigned short int    angle[3];
};
 
struct BIRD_STATUS
{
  struct SINGLE_BIRD_STATUS     status;
  struct FILTER_TABLE           alphaMin;
  struct FILTER_TABLE           alphaMax;
  struct FILTER_TABLE           Vm;
  struct ANGLES_TABLE           xyzReferenceFrame;
  struct ANGLES_TABLE           xyzAngleAlign;
};
 
struct HEADER
{
  unsigned short int    sequence;
 
  /*time_t time;   SGI formatted as 6 bytes (see time_t) */
  /*unsigned short int tm1;
    unsigned short int tm2;*/
 
  unsigned char         time[4];
  unsigned short int    milliseconds;
 
  unsigned char         type;
  unsigned char         xtype;
  unsigned char         protocol;
 
  unsigned char         error_code;
  unsigned short int    error_code_extension;
 
  unsigned short int    number_bytes;
};
 
struct RSP_GET_STATUS_all
{
  unsigned char         all;
  unsigned char         FBBerror;
  unsigned char         flockNumber;
  unsigned char         serverNumber;
  unsigned char         transmitterNumber;
  char                  measurementRate[6];
  unsigned char         chassisNumber;
  unsigned char         chassisDevices;
  unsigned char         firstAddress;
  unsigned short int    softwareRevision;
};
 
struct DATAPACKET
{
  struct HEADER header;
  unsigned char buffer[2048];
}; 

class aMotionStar {

 public:

    // constructor

      aMotionStar(int hemisphere,
		  unsigned int birdFormat,
		  unsigned int birdsRequired,
		  int runMode,
		  double birdRate,
		  unsigned char reportRate);

    // destructor

      ~aMotionStar();

    // returns whether the MotionStar is active

      bool& isActive();

    // initializes the MotionStar, setting the status for each bird

      void start();

    // gets singleShots # of samplings, if 0 then runs continuously

      void sample();

    // stops the flock

      void stop();

    // return the position/angle of the trackers

      float xPos(int i);
      float yPos(int i);
      float zPos(int i);
      float zRot(int i);
      float yRot(int i);
      float xRot(int i);


    // getters, setters

      void setHemisphere ( int n );
inline unsigned int getHemisphere () {return hemisphere;}

      void setBirdFormat ( unsigned int n );
inline unsigned int getBirdFormat () {return birdFormat;}

      void setNumBirds ( unsigned int n );
inline unsigned int getNumBirds () {return birdsRequired;}

      void setBirdRate ( double n );
inline double getBirdRate () {return birdRate;}

      void setRunMode ( int n );
inline int getRunMode () {return runMode;}

      void setReportRate ( unsigned char n );
inline unsigned char getReportRate () {return reportRate;}

//*****************************************************
 
private:
  // sends a wakeup call to the MotionStar

    void send_wakeup();

  // tells the MotionStar to sample continuously

    void runContinuous();

  // requests a single sampling of the MotionStar

    void singleShot();

  // gets the system status

    void get_status_all();

  // sets the system status

    void set_status_all();

  // prints out the MotionStar information

    void printInfo();

  // prints out the MotionStar's header information

    void display_hdr();

  // gets the status of an individual bird

    void get_status_fbb(unsigned char);

  // sets the status of an individual bird

    void set_status_fbb(unsigned char);

  // converts the raw hex data to a floating point

    float rawToFloat(char r1, char r2);


float posinfo[3][6]; 
bool 			active;
char                    ip_address[16];
int x;
unsigned char           all;
unsigned char           FBBerror;
unsigned char           flockNumber;
unsigned char           serverNumber;
unsigned char           chassisNumber;
unsigned char           chassisDevices;
unsigned char           firstAddress;
unsigned short int      softwareRevision;
unsigned char           transmitterNumber;
char                    szRate[7];
int                     mRate;
double                  realRate;
unsigned int            statusSize, temp;
unsigned char           ertFlag;
unsigned char           buttonState;
 
struct BIRD_STATUS Bird[120];
struct HEADER command, *lpCommand;
struct RSP_GET_STATUS_all RspGetStatAll, *lpRspGetStatAll;
struct DATAPACKET response, *lpResponse;
struct sockaddr_in client,server;
 
int s,rtn;
unsigned  long i,n;
unsigned int sequenceNumber;
 
char *newptr;
 
int errno; // EXTERN!!
 
int                     numberBytes;
int                     bytesReceived;
#if defined(__FreeBSD__) || defined(_AIX)
socklen_t		Size, *lpSize;
#else
int                     Size, *lpSize;
#endif
int                     sockfd;
time_t                  timeSeconds;
char                    timeChar[64];
struct tm               newtime;
int                     digit, no, j;
char                    rate[7];

unsigned int    format;
int             hemisphere;
unsigned int    birdFormat;
unsigned int    birdsRequired;
double          birdRate;
int             runMode;
unsigned char   addr;
unsigned char   reportRate;
 

};


#endif 
