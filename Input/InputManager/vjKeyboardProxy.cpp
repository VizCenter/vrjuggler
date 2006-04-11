/*************** <auto-copyright.pl BEGIN do not edit this line> **************
 *
 * VR Juggler is (C) Copyright 1998, 1999, 2000 by Iowa State University
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


#include <vjConfig.h>
#include <Kernel/vjKernel.h>
#include <Input/InputManager/vjKeyboardProxy.h>

bool vjKeyboardProxy::config(vjConfigChunk* chunk)
{
   vjDEBUG_BEGIN(vjDBG_INPUT_MGR,3) << "------------------ KEYBOARD PROXY config()-----------------\n" << vjDEBUG_FLUSH;
   vjASSERT(((std::string)chunk->getType()) == "KeyboardProxy");

   int unitNum = chunk->getProperty("unit");
   std::string proxy_name = chunk->getProperty("name");
   std::string dev_name = chunk->getProperty("device");

   int proxy_num = vjKernel::instance()->getInputManager()->addKeyboardProxy(dev_name,unitNum,proxy_name,this);

   if ( proxy_num != -1)
   {
      vjDEBUG_END(vjDBG_INPUT_MGR,3) << "   KeyboardProxy config()'ed"
                                     << std::endl << vjDEBUG_FLUSH;
      return true;
   }
   else
   {
      vjDEBUG(vjDBG_INPUT_MGR,0) << "   KeyboardProxy config() failed"
                                 << std::endl << vjDEBUG_FLUSH;
      vjDEBUG_END(vjDBG_INPUT_MGR,3) << std::endl << vjDEBUG_FLUSH;
      return false;
   }
}
