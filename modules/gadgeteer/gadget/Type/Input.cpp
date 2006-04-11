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

#include <gadget/gadgetConfig.h>

#include <vpr/Thread/Thread.h>
#include <vpr/Util/Assert.h>

#include <gadget/Type/Input.h>
#include <jccl/Config/ConfigChunk.h>


namespace gadget
{

Input::Input()
 : mPort(""),
   mInstName(""),
   mPortId(0),
   mThread(NULL),
   mActive(0),
   mBaudRate(0)
{
}

Input::~Input()
{
}

bool Input::config( jccl::ConfigChunkPtr c)
{
  if((!mPort.empty()) && (!mInstName.empty()))
  {
     // ASSERT: We have already been configured
     //         this prevents config from being called multiple times (once for each derived class)
     //         ie. Digital, Analog, etc
     return true;
  }

  mPort     = c->getProperty<std::string>("port");
  mInstName = c->getFullName();
  mBaudRate = c->getProperty<int>("baud");

  return true;
}


void Input::setPort(const std::string& serialPort)
{
if (mThread != NULL) {
     std::cerr << "Cannot change the serial Port while active\n";
     return;
  }
  mPort = serialPort;
}

std::string Input::getPort()
{
  if (mPort.empty()) return std::string("No port");
  return mPort;
}

void Input::setBaudRate(int baud)
{
  if (mThread != NULL)
     mBaudRate = baud;
}


};
