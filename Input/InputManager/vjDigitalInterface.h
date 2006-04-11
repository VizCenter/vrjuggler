/*************** <auto-copyright.pl BEGIN do not edit this line> **************
 *
 * VR Juggler is (C) Copyright 1998, 1999, 2000, 2001, 2002
 *   by Iowa State University
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


#ifndef _VJ_DIGITAL_INTERFACE_H_
#define _VJ_DIGITAL_INTERFACE_H_

#include <Kernel/vjKernel.h>
#include <Math/vjMatrix.h>
#include <Input/InputManager/vjInputManager.h>
#include <Input/InputManager/vjDigitalProxy.h>
#include <Input/InputManager/vjDeviceInterface.h>

//: Provides a simplified user interface to a digital device
//
// See also: vjDeviceInterface
//!PUBLIC_API:
class vjDigitalInterface : public vjDeviceInterface
{
protected:
   vjDigitalInterface(vjDigitalInterface& other) {;}

public:
   vjDigitalInterface() : mDigProxy(NULL)
   {;}

   vjDigitalProxy* operator->()
   { return mDigProxy; }

   vjDigitalProxy& operator*()
   { return *(mDigProxy); }

   virtual void refresh()
   {
      vjDeviceInterface::refresh();
      if(mProxyIndex != -1)
         mDigProxy = vjKernel::instance()->getInputManager()->getDigProxy(mProxyIndex);
      else
         mDigProxy = vjKernel::instance()->getInputManager()->getDummyDigProxy();
   }

private:
   vjDigitalProxy* mDigProxy;     // The proxy that is being wrapped
};

#endif
