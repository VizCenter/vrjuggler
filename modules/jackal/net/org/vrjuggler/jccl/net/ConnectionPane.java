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
package org.vrjuggler.jccl.net;

import java.awt.*;
import java.awt.event.*;
import javax.swing.*;

import org.vrjuggler.jccl.config.*;
import org.vrjuggler.jccl.vjcontrol.ui.ControlUIModule;
import org.vrjuggler.jccl.vjcontrol.ui.PlugPanel;
import org.vrjuggler.jccl.vjcontrol.*;

/** PlugPanel for managing connections to VR Juggler apps.
 *
 *  @version $Revision$
 */
public class ConnectionPane extends JPanel
    implements PlugPanel, ActionListener, NetControlListener {

    JButton connect_button;
    JButton disconnect_button;
    JTextField hostfield;
    JTextField portfield;
    JMenuItem updateactive_mi;
    protected String component_name;
    protected ConfigChunk component_chunk;
    protected NetworkModule network_module;
    protected ControlUIModule ui_module;
    protected boolean ui_initialized;

    public ConnectionPane () {
   super(new GridLayout (2, 1, 10, 10));

        component_name = "Unconfigured ConnectionPane";
        component_chunk = null;

        ui_module = null;
        network_module = null;

        ui_initialized = false;
    }


    protected void setRemoteAddress (String _host, int port) {
   hostfield.setText (_host);
   portfield.setText (Integer.toString(port));
    }


    public void actionPerformed (ActionEvent e) {
        System.out.println ("connection panel event called");
        int i;

        if (e.getSource() == connect_button) {
            network_module.disconnect(); // if we were connected, drop it
            int portnum = Integer.parseInt(portfield.getText());
       network_module.setRemoteHost (hostfield.getText(), portnum);
            network_module.connect ();
        }
        else if (e.getSource() == disconnect_button) {
            network_module.disconnect();
        }
        else if (e.getSource() == updateactive_mi) {
            network_module.requestUpdates();
        }
    }  // actionPerformed()


    /********************* NetControlListener Stuff ******************/
    public void openedConnection (NetControlEvent e) {;}
    public void closedConnection (NetControlEvent e) {;}
    public void addressChanged (NetControlEvent e) {
   setRemoteAddress (e.host, e.port);
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

        // get pointers to the modules we need.
        VarValue prop_val = ch.getProperty(VjComponentTokens.DEPENDENCIES);

        if ( prop_val != null )
        {
            int i;
            int n = ch.getPropertyCount(VjComponentTokens.DEPENDENCIES);
            String s;
            VjComponent c;
            for (i = 0; i < n; i++) {
                s = ch.getProperty(VjComponentTokens.DEPENDENCIES, i).toString();
                c = Core.getVjComponent (s);
                if (c != null) {
                    if (c instanceof ControlUIModule)
                        ui_module = (ControlUIModule)c;
                    if (c instanceof NetworkModule)
                        network_module = (NetworkModule)c;
                }
            }
        }
    }


    public void initialize () throws VjComponentException {
        if (ui_module == null || network_module == null)
            throw new VjComponentException (component_name + " initialized with unmet VjComponent dependencies.");

        updateactive_mi =
            ui_module.addMenuItem ("File/Update Active Configuration");
        updateactive_mi.addActionListener (this);
    }


    public ConfigChunk getConfiguration () {
        return component_chunk;
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
        if (!ui_initialized) {
            JPanel p1, p2;

            /*    GridLayout l = new GridLayout(2,1);
                  l.setVgap(10);
                  l.setHgap(10);
                  setLayout (l);
            */

            p1 = new JPanel();
            p2 = new JPanel();
            p1.add (new JLabel("Host Name:"));
            hostfield = new JTextField (30);
            p1.add(hostfield);
            p1.add (new JLabel("Port:"));
            portfield = new JTextField (5);
            p1.add(portfield);
            connect_button = new JButton("Connect");
            connect_button.addActionListener(this);
            p2.add(connect_button);
            disconnect_button = new JButton ("Disconnect");
            disconnect_button.addActionListener(this);
            p2.add (disconnect_button);
            add(p1);
            add(p2);

            connect_button.setToolTipText ("Connect or reconnect to remote host");
            disconnect_button.setToolTipText ("Disconnect from remote host");

            hostfield.setText (network_module.getHost());
            portfield.setText (Integer.toString(network_module.getPort()));

            network_module.addNetControlListener (this);

            ui_initialized = true;
        }
        return ui_initialized;
    }

    public void destroy () {
        if (ui_initialized)
            network_module.removeNetControlListener(this);
    }

    public void rebuildDisplay () {
    }


}


