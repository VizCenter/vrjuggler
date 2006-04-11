/*************** <auto-copyright.pl BEGIN do not edit this line> **************
 *
 * VR Juggler is (C) Copyright 1998-2003 by Iowa State University
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

#ifndef _GADGET_INPUT_MIXER_H_
#define _GADGET_INPUT_MIXER_H_

#include <gadget/gadgetConfig.h>
#include <vpr/vpr.h>
#include <vpr/IO/ObjectReader.h>
#include <vpr/IO/ObjectWriter.h>
#include <vpr/IO/SerializableObject.h>

#include <boost/type_traits.hpp>
#include <boost/static_assert.hpp>

namespace gadget
{

/** Placeholder class.
* This class is provides a generic implementation of the virtual
* methods of a device.
*
* This class is meant to be used to create "place holder" devices
* that don't actually function on their own but instead just
* look like a device with a given interface.
* This can be useful for devices that are progmatically set
* by systems such as the remote input manager
*/
template <class ParentType>
class InputPlaceHolder  : public ParentType
{
public:
   virtual bool sample()
    {return(0);}

   virtual bool startSampling()
    {return(0);}

   virtual bool stopSampling()
    {return(0);}

   virtual void updateData()
    {;}

   /**
    * Invokes the global scope delete operator.  This is required for proper
    * releasing of memory in DLLs on Win32.
    */
   void operator delete(void* p)
   {
      ::operator delete(p);
   }

protected:
   /**
    * Deletes this object.  This is an implementation of the pure virtual
    * gadget::Input::destroy() method.
    */
   virtual void destroy()
   {
      delete this;
   }
};


/**
* Class for mixin in base classes of input devices.
* All devices should derive from a mixed type like below based on their types.
*
* This allows us to implement any custom functions that are needed
* such as writeObject and readObject.
*
* InputMixer<InputMixer<InputMixer<Input,Digital>,Analog>,Position>
*/
template <class ComposedParent, class NewParent>
class InputMixer : public ComposedParent, public NewParent
{
private:
   // Make sure that we are using serializable objects
   BOOST_STATIC_ASSERT((::boost::is_base_and_derived<vpr::SerializableObject,NewParent>::value));

public:
   /** Type of the placeholder object that can be used to represent us */
   typedef InputPlaceHolder< InputMixer<ComposedParent, NewParent> > MixedPlaceholderType;

   vpr::ReturnStatus writeObject(vpr::ObjectWriter* writer)
   {
      writer->beginTag(InputMixer<ComposedParent,NewParent>::getBaseType());
      ComposedParent::writeObject(writer);
      NewParent::writeObject(writer);
      writer->endTag();
      return(vpr::ReturnStatus::Succeed);
   }

   vpr::ReturnStatus readObject(vpr::ObjectReader* reader)
   {
      reader->beginTag(InputMixer<ComposedParent,NewParent>::getBaseType());
      ComposedParent::readObject(reader);
      NewParent::readObject(reader);
      reader->endTag();
      return(vpr::ReturnStatus::Succeed);
   }

   std::string getBaseType()
   {
     return(ComposedParent::getBaseType() + NewParent::getBaseType());    //Input,Digital,Analog,Position, NEED THIS TOO
   }
};

} // end namespace

#endif   /* _GADGET_INPUT_MIXER_H_ */

