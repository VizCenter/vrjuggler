/*************** <auto-copyright.pl BEGIN do not edit this line> **************
 *
 * VR Juggler is (C) Copyright 1998-2002 by Iowa State University
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

#ifndef _GADGET_ASCENSION_MOTION_STAR_H_
#define _GADGET_ASCENSION_MOTION_STAR_H_

#include <gadget/gadgetConfig.h>

#include <string>
#include <vpr/Thread/Thread.h>

#include <gadget/Type/Input.h>
#include <gadget/Type/Position.h>
#include <gadget/Devices/Ascension/MotionStarStandalone.h>

namespace gadget
{

/**
 *  Position-derived class for running an Ascension MotionStar device that
 *  is connected to a Flock of Birds.  It is a wrapper class for the real
 *  driver class 'MotionStarStandalone'.
 *
 *  MotionStar adds to the MotionStarStandalone class shared memory and
 *  threading.<br>
 *  MotionStar is a positional device driver for the Flock of Birds.  The
 *  config chunk in the constructor should set up all the settings.  For these
 *  to be changed, the MotionStar has to be deleted and a new instance created
 *  with an updated configchunk.
 *  <br>
 *
 * @note Some functions still remain for changing the options of the
 *        MotionStar when its not in Sampling mode, but in order to stay
 *        consistent with the Input/vjPosition functionality these are only
 *        left for building apps without jccl::ConfigChunks.
 * @note A note on reciever access:
 *   Clients of Juggler should access tracker recievers as [0-n].  For
 *   example, if you have recievers 1,2, and 4 with transmitter on 3, then
 *   you can access the data, in order, as 0, 1, 2.
 *
 * See also: Position
 */
class MotionStar : public Input, public Position
{
public:

   /**
    *  Constructor.  This invokes the MotionStarStandalone constructor and
    *  initializes member variables.
    *
    * @pre None.
    * @post m_motion_star is initialized, and mThread is set to NULL.
    *
    * @param address           The IP address or hostname of the MotionStar
    *                           PC to which we are connecting.  This defaults
    *                           to NULL.
    * @param port              The server port to which we are connecting.
    *                           The default is 6000.
    * @param proto             The transmission protocol to use in
    *                           communicating with the server.  The default
    *                           is BIRDNET::TCP which matches the default
    *                           port number 6000.
    * @param master            A flag stating whether or not this server
    *                           chassis is a master or a slave.  The default
    *                           is slave (false).
    * @param hemisphere        The hemisphere in use on the transmitter.
    *                           The default is FLOCK::FRONT_HEMISPHERE.
    * @param bird_format       The bird format.  The default is
    *                           FLOCK::POSITION_ANGLES.
    * @param run_mode          The run mode for the device.  The default is
    *                           BIRDNET::CONTINUOUS.
    * @param report_rate      
    * @param measurement_rate 
    * @param birds_required    The number of birds required.  The default is
    *                           10.
    */
   MotionStar(const char* address = NULL, const unsigned short port = 6000,
              const enum BIRDNET::protocol proto = BIRDNET::TCP,
              const bool master = false,
              const FLOCK::hemisphere hemisphere = FLOCK::FRONT_HEMISPHERE,
              const FLOCK::data_format bird_format = FLOCK::POSITION_ANGLES,
              const BIRDNET::run_mode run_mode = BIRDNET::CONTINUOUS,
              const unsigned char report_rate = 1,
              const double measurement_rate = 68.3,
              const unsigned int birds_required = 10);

   /**
    *  Destructor.  Sampling is stopped, and the data pool is deallocated.
    *
    * @pre None.
    * @post Sampling is stopped, and the data pool is deallocated.
    */
   virtual ~MotionStar(void);

   // ========================================================================
   // Input overrides (or methods with the same name anyway).
   // ========================================================================

   /**
    *  Return what chunk type is associated with this class.
    *
    * @pre None.
    * @post A std::string constructed from a static constant string is
    *        returned to the caller.
    *
    * @return A std::string containing the chunk type for this device.
    */
   static std::string getChunkType (void)
   {
      return std::string("MotionStar");
   }

   /**
    *  See if the device is active or not.
    *
    * @pre m_motion_star has been initialized.
    * @post The active state of the MotionStar driver is returned to the
    *        caller.
    *
    * @return true - The MotionStar driver is active.
    * @return false - The MotionStar driver is inactive.
    */
   bool isActive (void)
   {
      return m_motion_star.isActive();
   }

   // ========================================================================
   // Position overrides.
   // ========================================================================

   /**
    *  Configure the MotionStar with the given config chunk.
    *
    * @pre None.
    * @post If c is a valid config chunk, the device is configured using its
    *        contents.  Otherwise, configuration fails and false is returned
    *        to the caller.
    *
    * @param c  A pointer to a MotionStar config chunk.
    *
    * @return true - The device was configured succesfully.
    * @return false - The config chunk is invalid.
    */
   virtual bool config(jccl::ConfigChunkPtr c);

   /**
    *  Begin sampling.
    *
    * @pre m_my_thread is either NULL or points to a valid vpr::Thread object.
    * @post If the driver is not already active and a thread has not already
    *        been created, an attempt is made to activate the device.  First,
    *        a connection attempt to the server is made.  If successful, the
    *        device is started.  If that too succeeds, a new threada is
    *        created for sampling the device.
    *
    * @return 0 - Sampling was unable to begin for some reason.
    * @return 1 - Sampling was started successfully.
    */
   virtual int startSampling(void);

   /**
    *  Stop sampling.
    *
    * @pre m_my_thread is either NULL or points to a valid vpr::Thread object.
    * @post If the driver is not currently active, no stop attempt is made.
    *        Otherwise, if m_my_thread is non-NULL (assumed to point to a
    *        valid vpr::Thread object), the thread is killed and deleted, and a
    *        stop attempt is made on the device.
    *
    * @return 0 - Sampling could not be stopped.  This will occur if the
    *               device is not active or the stop attempt failed.
    * @return 1 - Sampling was stopped successfully.
    */
   virtual int stopSampling(void);

   /**
    *  Sample data.
    *
    * @pre m_motion_star has been initialized.
    * @post If the device is active, a sample is made from it.  Data are
    *        read from each of the birds into the current read buffer, and
    *        transforms are constructed based on what was received.  Once
    *        this is done, the valid index is swapped for the next round of
    *        sampling.
    *
    * @return 0 - No sample could be made.
    */
   virtual int sample(void);

   /**
    *  Update to the sampled data.
    *
    * @pre None.
    * @post If the device is active, the valid data for all the birds are
    *        copied into the current data so that both are valid.  The
    *        current indices are then swapped.
    */
   virtual void updateData(void);
    

   // ========================================================================
   // MotionStar-specific methods.
   // ========================================================================

   /**
    *  Set the address (either IP address or hostname) for the server.
    *
    * @pre m_motion_star has been initialized.
    * @post If the device is not active, the address is set to the given
    *        value.
    *
    * @param n  The new value for the server address.
    */
   void setAddressName(const char* n);

   /**
    *  Get the current server address for the device.
    *
    * @pre m_motion_star has been initialized.
    * @post The current server address is returned to the caller.
    *
    * @return A character array naming the server address.
    */
   std::string getAddressName (void) const
   {
      return m_motion_star.getAddressName();
   }

   /**
    *  Set the port on the server to which we connect.
    *
    * @pre m_motion_star has been initialized.
    * @post If the device is not active, the port is set to the given value.
    *
    * @param port  The new value for the server port.
    */
   void setServerPort(const unsigned short port);

   /**
    *  Get the server port.
    *
    * @pre m_motion_star has been initialized.
    * @post The current server port is returned to the caller.
    *
    * @return An unsigned short integer containing the server port number.
    */
   unsigned short getServerPort (void) const
   {
      return m_motion_star.getServerPort();
   }

   //:
   //
   /**
 * @pre m_motion_star has been initialized.
    * @post If the device is not active, the transmission protocol is set to
    *        the given value.
    *
    * @param protocol  The new value for the transmission protocol.
    */
   void setProtocol(const enum BIRDNET::protocol proto);

   /**
    *  Get the transmission protocol used between the client and the server.
    *
    * @pre m_motion_star has been initialized.
    * @post The current transmission protocol is returned to the caller.
    *
    * @return
    */
   enum BIRDNET::protocol getProtocol (void) const
   {
      return m_motion_star.getProtocol();
   }

   //
   /**
 * @pre m_motion_star has been initialized.
    * @post If the device is not active, the master status is set to the
    *        given value.
    *
    * @param msater  The new value for the master status.
    */
   void setMasterStatus(const bool master);

   /**
    *  Get the master status for this server chassis.
    *
    * @pre m_motion_star has been initialized.
    * @post The current master status for this chassis is returned to the
    *        caller.
    *
    * @return  true - This server chassis is a master.
    * @return false - This server chassis is a slave.
    */
   bool getMasterStatus (void) const
   {
      return m_motion_star.getMasterStatus();
   }

   /**
    *  Change the hemisphere of the transmitter.
    *
    * @pre m_motion_star has been initialized.
    * @post If the device is not active, the hemisphere for it is reset to
    *        the given value.
    *
    * @param i  The new hemisphere to use.
    */
   void setHemisphere(const unsigned char hemisphere);

   /**
    *  Get the transmitter hemisphere currently in use by the device.
    *
    * @pre m_motion_star has been initialized.
    * @post The current hemisphere value is returned to the caller.
    *
    * @return An integer value for the hemisphere in use.
    */
   unsigned char getHemisphere (void) const
   {
      return m_motion_star.getHemisphere();
   }

   /**
    *  Set the bird format to the given value.
    *
    * @pre m_motion_star has been initialized.
    * @post If the device is not active, the bird format is updated to the
    *        new value.
    *
    * @param n  The new bird format to use.
    */
   void setBirdFormat(const unsigned int format);

   /**
    *  Get the bird format currently in use.
    *
    * @pre m_motion_star has been initialized.
    * @post The current bird format value is returned to the caller.
    *
    * @return An unsigned integer value for the bird format.
    */
   unsigned int getBirdFormat (void) const
   {
      return m_motion_star.getBirdFormat();
   }

   /**
    *  Set the number of birds connected to the flock.
    *
    * @pre m_motion_star has been initialized.
    * @post If the device is not active, the number of birds is updated to
    *        the new value.
    *
    * @param n  The new value for the number of birds in use.
    */
   void setNumBirds(const unsigned int n);

   /**
    *  Get the number of bird currently connected to the flock.
    *
    * @pre m_motion_star has been initialized.
    * @post The current number of connected birds is returned to the caller.
    *
    * @return An unsigned integer value for the number of connected birds.
    */
   unsigned int getNumBirds (void) const
   {
      return m_motion_star.getNumBirds();
   }

   /**
    *  Set the run mode for the device.
    *
    * @pre m_motion_star has been initialized.
    * @post If the device is not active, the run mode is set to the given
    *        value.
    *
    * @param i  The new value for the run mode.
    */
   void setRunMode(const unsigned int i);

   /**
    *  Get the current run mode for the device.
    *
    * @pre m_motion_star has been initialized.
    * @post The current run mode is returned to the caller.
    *
    * @return An integer value for run mode.
    */
   int getRunMode (void) const
   {
      return m_motion_star.getRunMode();
   }

   /**
    *  Set the report rate for the birds.
    *
    * @pre m_motion_star has been initialized.
    * @post If the device is not active, the bird report rate is set to the
    *        given byte value.
    *
    * @param rate  The new value for the bird report rate.
    */
   void setReportRate(const unsigned char rate);

   /**
    *  Get the current report rate for the device.
    *
    * @pre m_motion_star has been initialized.
    * @post The current report rate is returned to the caller.
    *
    * @return An unsigned character value for report rate.
    */
   unsigned char getReportRate (void) const
   {
      return m_motion_star.getReportRate();
   }

   /**
    *  Set the data measurement rate for the chassis.
    *
    * @pre m_motion_star has been initialized.
    * @post If the device is not active, the chassis data measurement rate
    *        is set to the given double value.
    *
    * @param rate  The new value for the measurement rate.
    */
   void setMeasurementRate(const double rate);

   /**
    *  Get the current data measurement rate for the chassis.
    *
    * @pre m_motion_star has been initialized.
    * @post The current data measurement rate is returned to the caller.
    *
    * @return A double-precision floating-point value for data measurement
    *           rate.
    */
   double getMeasurementRate (void) const
   {
      return m_motion_star.getMeasurementRate();
   }

   /**
    *  Get the x position of the i'th reciever.
    *
    * @pre m_motion_star has been initialized.
    * @post The x position for the given bird number is returned to the
    *        caller.
    *
    * @return A floating-point value for the current x position of the
    *           given bird.
    */
   float getXPos (const unsigned int i) const
   {
      return m_motion_star.getXPos(i);
   }

   /**
    *  Get the y position of the i'th reciever.
    *
    * @pre m_motion_star has been initialized.
    * @post The y position for the given bird number is returned to the
    *        caller.
    *
    * @return A floating-point value for the current y position of the
    *           given bird.
    */
   float getYPos (const unsigned int i) const
   {
      return m_motion_star.getYPos(i);
   }

   /**
    *  Get the z position of the i'th reciever.
    *
    * @pre m_motion_star has been initialized.
    * @post The z position for the given bird number is returned to the
    *        caller.
    *
    * @return A floating-point value for the current z position of the
    *           given bird.
    */
   float getZPos (const unsigned int i) const
   {
      return m_motion_star.getZPos(i);
   }

   /**
    *  Get the z rotation of the i'th reciever.
    *
    * @pre m_motion_star has been initialized.
    * @post The z rotation for the given bird number is returned to the
    *        caller.
    *
    * @return A floating-point value for the current z rotation of the
    *           given bird.
    */
   float getZRot (const unsigned int i) const
   {
      return m_motion_star.getZRot(i);
   }

   /**
    *  Get the y rotation of the i'th reciever.
    *
    * @pre m_motion_star has been initialized.
    * @post The y rotation for the given bird number is returned to the
    *        caller.
    *
    * @return A floating-point value for the current y rotation of the
    *           given bird.
    */
   float getYRot (const unsigned int i) const
   {
      return m_motion_star.getYRot(i);
   }

   /**
    *  Get the x rotation of the i'th reciever.
    *
    * @pre m_motion_star has been initialized.
    * @post The x rotation for the given bird number is returned to the
    *        caller.
    *
    * @return A floating-point value for the current x rotation of the
    *           given bird.
    */
   float getXRot (const unsigned int i) const
   {
      return m_motion_star.getXRot(i);
   }

private:
   /**
    *  Unimplemented!
    */
   void positionCorrect(float& x, float& y, float& z);

   /**
    *  Unimplemented!
    */
   void initCorrectionTable(const char*);

   vpr::Thread*         m_my_thread;   /**<  The thread doing the flock sampling */
   MotionStarStandalone m_motion_star; /**<  Actual MotionStar device driver */

    //PositionData*       mData;
};


};

#endif   /* _GADGET_ASCENSION_MOTION_STAR_H_ */
