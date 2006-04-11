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
package org.vrjuggler.jccl.config;

import java.io.*;

public interface ConfigIOHandler
{
   public void readConfigChunkDB(File file, ConfigChunkDB db,
                                 ConfigIOStatus iostatus);

   public void readConfigChunkDB(InputStream in, ConfigChunkDB db,
                                 ConfigIOStatus iostatus);

   public void writeConfigChunkDB(DataOutputStream out, ConfigChunkDB db)
      throws IOException;

   public void writeConfigChunkDB(File file, ConfigChunkDB db)
      throws IOException;

   public void readChunkDescDB(File file, ChunkDescDB db,
                               ConfigIOStatus iostatus);

   public void readChunkDescDB(InputStream in, ChunkDescDB db,
                               ConfigIOStatus iostatus);

   public void writeChunkDescDB(DataOutputStream out, ChunkDescDB db)
      throws IOException;

   public void writeChunkDescDB(File file, ChunkDescDB db)
      throws IOException;
}
