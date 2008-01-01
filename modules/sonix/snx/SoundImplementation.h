/****************** <SNX heading BEGIN do not edit this line> *****************
 *
 * sonix
 *
 * Original Authors:
 *   Kevin Meinert, Carolina Cruz-Neira
 *
 ****************** <SNX heading END do not edit this line> ******************/

/*************** <auto-copyright.pl BEGIN do not edit this line> **************
 *
 * VR Juggler is (C) Copyright 1998-2008 by Iowa State University
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
 *************** <auto-copyright.pl END do not edit this line> ***************/

/* Generated by Together */

#ifndef SNXSOUNDIMPLEMENTATION_H
#define SNXSOUNDIMPLEMENTATION_H

#include <snx/snxConfig.h>

#include <string>
#include <map>
#include <boost/concept_check.hpp>
#include <gmtl/Matrix.h>
#include <vpr/Util/Assert.h>

#include <snx/SoundInfo.h>
#include <snx/SoundAPIInfo.h>
#include <snx/ISoundImplementation.h>

namespace snx
{

/** \class SoundImplementation SoundImplementation.h snx/SoundImplementation.h
 *
 * Sound implementation.
 */
class SNX_CLASS_API SoundImplementation : public ISoundImplementation
{
public:
   /** Default constructor. */
   SoundImplementation()
      : ISoundImplementation()
      , mName( "unknown" )
      , mSoundAPIInfo()
      , mSounds()
      , mListenerPos()
   {
   }

   /**
    * Every implementation can return a new copy of itself.
    */
   virtual void clone( ISoundImplementation* &newCopy ) = 0;

   /** Destructor. */
   virtual ~SoundImplementation()
   {
      // make sure the API has gracefully exited.
      this->shutdownAPI();
   }

   /**
    * Copies the current state of the system from the given API into this
    * object.  This does not perform any binding.  That must be done
    * separately.
    *
    * @param si The object from which the state will be copied into this
    *           object.
    */
   void copy( const SoundImplementation& si );

public:

   /**
    * Triggers a sound.
    *
    * @pre alias does not have to be associated with a loaded sound.
    * @post If alias is associated with a loaded sound, then the loaded sound
    *       is triggered.  If it isn't, then nothing happens.
    *
    * @param alias  Alias of the sound to trigger.
    * @param repeat The number of times to play.  Use -1 to repeat forever.
    */
   virtual void trigger(const std::string& alias, const int repeat = 1)
   {
      vprASSERT(this->isStarted() == true && "must call startAPI prior to this function");

      this->lookup( alias ).repeat = repeat;
      this->lookup( alias ).repeatCountdown = repeat;
   }

   /**
    * Is the named sound currently playing?
    *
    * @param alias The alias of the sound to query.
    */
   virtual bool isPlaying( const std::string& alias )
   {
      boost::ignore_unused_variable_warning(alias);
      return false;
   }

   /**
    * Specifies whether the named sound retriggers from beginning when
    * triggered while playing.  In other words, when the named sound is already
    * playing and trigger() is called, does the sound restart from the
    * beginning?
    *
    * @param alias The alias of the sound to change.
    * @param onOff A Boolean value enabling or disabling retriggering.
    */
   virtual void setRetriggerable( const std::string& alias, bool onOff )
   {
      this->lookup( alias ).retriggerable = onOff;
   }

   /**
    * Is the named sound retriggerable?
    *
    * @param alias The alias of the sound to query.
    */
   virtual bool isRetriggerable( const std::string& alias )
   {
      return bool( this->lookup( alias ).retriggerable == true );
   }

   /**
    * Stops the named sound.
    *
    * @param alias Alias of the sound to be stopped.
    */
   virtual void stop( const std::string& alias )
   {
      vprASSERT(this->isStarted() == true && "must call startAPI prior to this function");
      this->lookup( alias ).repeatCountdown = 0;
   }

   /**
    * Pauses the named sound.  Use unpause() to return playback from where the
    * sound was paused.
    *
    * @param alias The alias of the sound to pause.
    */
   virtual void pause( const std::string& alias )
   {
      this->stop( alias );
   }

   /**
    * Resumes playback of the named sound from a paused state.  This does
    * nothing if sound was not paused.
    *
    * @param alias The alias of the sound to unpause.
    */
   virtual void unpause( const std::string& alias )
   {
      this->trigger( alias, this->lookup( alias ).repeat );
   }

   /**
    * If the named sound is paused, then return true.
    *
    * @param alias The alias of the sound to query.
    */
   virtual bool isPaused( const std::string& alias )
   {
      boost::ignore_unused_variable_warning(alias);
      return false;
   }

   /**
    * Sets the named sound as either ambient or positional depending on the
    * value of ambient.  If the sound is ambient, it is attached to the
    * listener, and its volume does not change when the listener moves.  If
    * the sound is positional, the volume changes when the listener moves.
    *
    * @param ambient A flag identifying whether this sound is ambient (true)
    *                or positional (false).  This parameter is optional,
    *                and it defaults to false (the sound is positional).
    */
   virtual void setAmbient( const std::string& alias, bool ambient = false )
   {
      this->lookup( alias ).ambient = ambient;
   }

   /**
    * Is the named sound ambient?
    *
    * @param alias The alias of the sound to query.
    */
   virtual bool isAmbient( const std::string& alias )
   {
      return this->lookup( alias ).ambient;
   }

   /**
    * Alters the frequency of the named sound.
    *
    * @param alias  The alias of the sound to change.
    * @param amount The value that determines the pitch bend.  1.0 means
    *               that there is no change.  A value less than 1.0 is low;
    *               a value greather than 1.0 is high.
    */
   virtual void setPitchBend( const std::string& alias, float amount )
   {
      this->lookup( alias ).pitchbend = amount;
   }

   /**
    * Sets the effect volume of the named sound.  The value must be in the
    * range [0,1].
    *
    * @param alias  The alias of the sound to change.
    * @param amount The value of the volume.  It must be between 0.0 and
    *               1.0 inclusive.
    */
   virtual void setVolume( const std::string& alias, float amount )
   {
      this->lookup( alias ).volume = amount;
   }

   /**
    * Sets the effect cutoff of the named soudn.  The value must be in the
    * range [0,1].
    *
    * @param alias  The alias of the sound to change.
    * @param amount The value of the cutoff.  1.0 is no change; 0.0 is
    *               total cutoff.
    */
   virtual void setCutoff( const std::string& alias, float amount )
   {
      this->lookup( alias ).cutoff = amount;
   }

   /**
    * Set the named sound's 3D position.
    *
    * @param alias The alias of the sound to change.
    * @param x     The X coordinate of the sound in 3D OpenGL coordinates.
    * @param y     The Y coordinate of the sound in 3D OpenGL coordinates.
    * @param z     The Z coordinate of the sound in 3D OpenGL coordinates.
    */
   virtual void setPosition(const std::string& alias, float x, float y,
                            float z)
   {
      vprASSERT(this->isStarted() == true && "must call startAPI prior to this function");
      this->lookup( alias ).position[0] = x;
      this->lookup( alias ).position[1] = y;
      this->lookup( alias ).position[2] = z;
   }

   /**
    * Gets the named sound's 3D position.
    *
    * @param alias A name that has been associated with some sound data.
    * @param x     Storage for the X coordinate of the sound's position (in
    *              OpenGL coordinates).
    * @param y     Storage for the Y coordinate of the sound's position (in
    *              OpenGL coordinates).
    * @param z     Storage for the Z coordinate of the sound's position (in
    *              OpenGL coordinates).
    */
   virtual void getPosition(const std::string& alias, float& x, float& y,
                            float& z)
   {
      vprASSERT(this->isStarted() == true && "must call startAPI prior to this function");

      x = this->lookup( alias ).position[0];
      y = this->lookup( alias ).position[1];
      z = this->lookup( alias ).position[2];
   }

   /**
    * Sets the position of the listener.
    *
    * @param mat A transformation matrix representing the position of the
    *            listener.
    */
   virtual void setListenerPosition( const gmtl::Matrix44f& mat )
   {
      vprASSERT(this->isStarted() == true && "must call startAPI prior to this function");
      mListenerPos = mat;
   }

   /**
    * Gets the position of the listener.
    *
    * @param mat Storage for returning the position of the listener.
    */
   virtual void getListenerPosition( gmtl::Matrix44f& mat )
   {
      vprASSERT(this->isStarted() == true && "must call startAPI prior to this function");
      mat = mListenerPos;
   }

   /**
    * Start the sound API, creating any contexts or other configurations at
    * startup.  This function should be called before using the other
    * functions in the class.
    *
    * @post Sound API is ready to go.
    * @return 1 if success, 0 otherwise.
    */
   virtual int startAPI() = 0;

   /**
    * Queries whether the API has been started.
    *
    * @return true if API has been started, false otherwise.
    */
   virtual bool isStarted() const = 0;

   /**
    * Kills the sound API, deallocating any sounds, etc.  This function could
    * be called any time.  The function could be called multiple times, so it
    * should be smart.
    *
    * @post The sound API is no longer running or available for use.
    */
   virtual void shutdownAPI() {}

   /**
    * Configures/reconfigures the sound API global settings.
    *
    * @param sai A description of the settings for this sound API.
    */
   virtual void configure( const snx::SoundAPIInfo& sai )
   {
      mSoundAPIInfo = sai;
   }

   /**
    * Configures/reconfigures the named sound by associating sound data with
    * the named sound.  Afterwards, the alias can be used to operate on sound
    * data.
    *
    * Configure: associate a name (alias) to the description if not already
    * done.
    *
    * Reconfigure: change properties of the sound to the description
    * provided.
    *
    * @pre Provide a SoundInfo which describes the sound.
    * @post This handle will point to loaded sound data.
    *
    * @param description An object that describes the sound for which this
    *                    object will be a handle.
    */
   virtual void configure(const std::string& alias,
                          const snx::SoundInfo& description);

   /**
    * Removes a configured sound.  Any future reference to the alias will not
    * cause an error, but it will not result in a rendered sound.
    *
    * @param alias The alias of the sound to be removed.
    */
   virtual void remove(const std::string& alias);

   /**
    * Call once per sound frame (which does not have to be same as the graphics
    * frame).
    *
    * @param timeElapsed The time elapsed since the last sound frame.
    */
   virtual void step(const float timeElapsed)
   {
      boost::ignore_unused_variable_warning(timeElapsed);
   }

   /**
    * Clears all associations.
    *
    * @post Any existing aliases will be stubbed. Sounds will be unbound.
    */
   virtual void clear()
   {
      this->unbindAll();
   }

   /**
    * Bind: load (or reload) all associated sounds.
    *
    * @post All sound associations are buffered by the sound API.
    */
   virtual void bindAll();

   /**
    * Unbind: unload/deallocate all associated sounds.
    *
    * @post All sound associations are unbuffered by the sound API.
    */
   virtual void unbindAll();

   /**
    * Loads/allocates the sound data this alias refers to the sound API.
    *
    * @post The sound API has the sound buffered.
    */
   virtual void bind( const std::string& alias ) = 0;

   /**
    * Unloads/deallocates the sound data this alias refers to in the sound API.
    *
    * @post The sound API no longer has the sound buffered.
    */
   virtual void unbind( const std::string& alias ) = 0;

   snx::SoundInfo& lookup( const std::string& alias )
   {
      return mSounds[alias];
   }

   void setName( const std::string& name )
   {
      mName = name;
   }

   std::string& name()
   {
      return mName;
   }

protected:
   /*
    * The name of this implementation.
    */
   std::string mName;

   snx::SoundAPIInfo mSoundAPIInfo;
   std::map<std::string, snx::SoundInfo> mSounds;

   gmtl::Matrix44f mListenerPos; /**< Position of the observer/listener. */
};

} // end namespace

#endif //SNXSOUNDIMPLEMENTATION_H
