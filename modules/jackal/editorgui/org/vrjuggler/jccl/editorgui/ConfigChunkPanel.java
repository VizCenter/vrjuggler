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
package org.vrjuggler.jccl.editorgui;

import org.vrjuggler.jccl.config.ConfigChunk;
import org.vrjuggler.jccl.config.ConfigChunkDB;
import org.vrjuggler.jccl.vjcontrol.ui.EditorPanel;

/** A VjComponent that displays and edits ConfigChunks.
 *  Interface for any panel that registers with the componentfactory as
 *  a display panel for ConfigChunks.  In addition to implementing this
 *  interface, the new display class must also be derived from JComponent,
 *  and must have a constructor that takes no arguments.
 *
 *  @version $Revision$
 */
public interface ConfigChunkPanel extends EditorPanel {


    /** Set the ConfigChunk and DB to display.
     *  This should generally be set immediately after instantiating
     *  the ConfigChunkPanel.
     *  It's perfectly reasonable to pass null for _chunkdb, as it is
     *  only used to check matches().
     */
    public void setChunk (ConfigChunk _chunk, ConfigChunkDB _chunkdb);



    /** Returns the name of the chunk panel
     *  Typically returns the name of the chunk.
     */
    public String getName ();



    /** Returns the chunkdb passed via setChunk (may be null!).
     */
    public ConfigChunkDB getChunkDB ();

    /** Returns the chunk loaded for editing by the panel. */
    public ConfigChunk getChunk();

    /** Returns the original name of the edited chunk. */
    public String getOrigChunkName();

    /** Commits any and all changes the user made to the loaded chunk. */
    public void commit() throws ConfigUIException;
}
