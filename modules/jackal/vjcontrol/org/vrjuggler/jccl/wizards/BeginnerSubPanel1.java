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
package org.vrjuggler.jccl.wizards;

import java.io.*;
import javax.swing.*;
import org.vrjuggler.jccl.config.*;
import org.vrjuggler.jccl.editorgui.ConfigUIHelper;

public class BeginnerSubPanel1 extends JPanel implements WizardSubPanel {

    public BeginnerSubPanel1 () {
        super();
    }

    //----------------------- WizardSubPanel Stuff --------------------------

    public void setConfigUIHelper (ConfigUIHelper _uihelper_module) {
    }

    public JComponent getUIComponent () {
        return this;
    }

    public boolean initUIComponent () {
        return true;
    }

    public File getHelpFile () {
        return null;
    }

    public boolean checkState () {
        return true;
    }

    public boolean setInitialDB (ConfigChunkDB db) {
        return true;
    }

    public ConfigChunkDB getResultDB () {
        return new ConfigChunkDB();
    }

    public void reset() {
        ;
    }
}
