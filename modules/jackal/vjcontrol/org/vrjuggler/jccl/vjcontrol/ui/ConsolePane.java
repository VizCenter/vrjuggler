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
package org.vrjuggler.jccl.vjcontrol.ui;

import java.awt.*;
import java.awt.event.*;
import javax.swing.*;
import javax.swing.text.*;

import org.vrjuggler.jccl.vjcontrol.*;
import org.vrjuggler.jccl.config.ConfigChunk;

/** PlugPanel to display error and status messages.
 * 
 *  @version $Revision$
 */
public class ConsolePane extends JPanel implements PlugPanel, LogMessageListener {

    protected String component_name;
    protected ConfigChunk component_chunk;

    JTextArea textarea;
    JScrollPane sp;
    StringBuffer tmpbuffer;

    public ConsolePane () {
        component_name = "Unconfigured ConsolePane";
        component_chunk = null;
        textarea = null;
        tmpbuffer = new StringBuffer (256);

//  	setLayout (new BorderLayout (5, 5));
//  	textarea = new JTextArea ("");
//  	textarea.setEditable (false);
//  	add (sp = new JScrollPane (textarea), "Center");

    }



    public void logMessage (LogMessageEvent e) {
	switch (e.getStyle()) {
	case LogMessageEvent.TEMPORARY_MESSAGE:
	    break;
	case LogMessageEvent.PERMANENT_MESSAGE:
            if (textarea == null)
                tmpbuffer.append ("(" + e.getSourceName() + "): " + e.getMessage() + "\n");
            else
                textarea.append ("(" + e.getSourceName() + "): " + e.getMessage() + "\n");
	    break;
	case LogMessageEvent.PERMANENT_ERROR:
            if (textarea == null)
                tmpbuffer.append ("ERROR (" + e.getSourceName() + "): " + e.getMessage() + "\n");
            else
                textarea.append ("ERROR (" + e.getSourceName() + "): " + e.getMessage() + "\n");
	    break;
	}
    }


    /********************** PlugPanel Stuff **************************/

    public String getComponentName () {
        return component_name;
    }


    public void setComponentName (String _name) {
        component_name = _name;
    }


    public ImageIcon getComponentIcon () {
        return null;
    }


    public void setConfiguration (ConfigChunk ch) throws VjComponentException {
        component_chunk = ch;
        component_name = ch.getName();
    }

    
    public ConfigChunk getConfiguration () {
        return component_chunk;
    }


    public void initialize () throws VjComponentException {
	Core.addLogMessageListener (this);
    }


    public VjComponent addConfig (ConfigChunk ch) throws VjComponentException {
        throw new VjComponentException (component_name + " does not support child component: " + ch.getName());
    }


    public boolean removeConfig (String name) {
        return false;
    }


    public JComponent getUIComponent() {
        return this;
    }

    public boolean initUIComponent() {
        if (textarea == null) {
            setLayout (new BorderLayout (5, 5));
            textarea = new JTextArea (tmpbuffer.toString());
            textarea.setEditable (false);
            add (sp = new JScrollPane (textarea), "Center");
        }
        return true;
    }

    public void destroy () {
        Core.removeLogMessageListener(this);
    }


    public void rebuildDisplay () {
    }


}
