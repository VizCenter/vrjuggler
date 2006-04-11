/*
 * VRJuggler
 *   Copyright (C) 1997,1998,1999,2000
 *   Iowa State University Research Foundation, Inc.
 *   All Rights Reserved
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
 */



package VjGUI.chunkdesc;

import javax.swing.*;
import javax.swing.border.*;
import java.awt.*;
import java.awt.event.*;
import java.util.Vector;
import VjGUI.*;
import VjConfig.*;



public class DescEnumFrame extends JFrame 
    implements ActionListener, WindowListener {

    ValType pdtype; // current type of the associated propertydescpanel
    JPanel panel;
    JPanel sppanel;  /* sppanel is the panel inside the scrollpane...*/
    JButton insertbutton;
    JButton removebutton;
    JButton okbutton;
    JButton cancelbutton;
    Vector elempanels; // vector of DescEnumElemPanels
    Vector data;  // vector of DescEnum
    public boolean closed;
    JPanel buttonspanel;
    boolean varnumbers;
    int numvals;


    /* 
     * ARGS: _varnumbers - if true, show insert & remove buttons
     * ARGS: _numval - # of values to show (only matters if varnumbers false)
     */
    public DescEnumFrame (PropertyDescPanel p,
			  Vector _data,
			  String pdn,
			  ValType pdt,
			  boolean _varnumbers,
			  int _numval) {

	super(pdn);
	
	closed = false;
	pdtype = pdt;

	buttonspanel = new JPanel();
	//buttonspanel.setLayout (new GridLayout (1, 4));
	//buttonspanel.setLayout (new BoxLayout (buttonspanel, BoxLayout.X_AXIS));

	data = _data;
	varnumbers = _varnumbers;
	numvals = _numval;

	elempanels = new Vector();
	panel = new JPanel(new BorderLayout (10,2));
	panel.setBorder (new EmptyBorder (10, 5, 5, 5));
	getContentPane().add(panel);
	
	sppanel = new JPanel();
	sppanel.setLayout (new BoxLayout (sppanel, BoxLayout.Y_AXIS));

	JScrollPane sp = new JScrollPane(sppanel, 
					 JScrollPane.VERTICAL_SCROLLBAR_ALWAYS, 
					 JScrollPane.HORIZONTAL_SCROLLBAR_AS_NEEDED);
	sp.getVerticalScrollBar().setUnitIncrement(40);
	sp.getHorizontalScrollBar().setUnitIncrement(40);

	panel.add (sp, "Center");

	if (varnumbers) {
	    buttonspanel.setLayout (new GridLayout (1, 4));
	    insertbutton = new JButton ("Insert");
	    insertbutton.addActionListener(this);
	    buttonspanel.add (insertbutton);
	    removebutton = new JButton ("Remove");
	    removebutton.addActionListener(this);
	    buttonspanel.add (removebutton);
	}
	else {
	    buttonspanel.setLayout (new GridLayout (1, 2));
	    insertbutton = null;
	    removebutton = null;
	}
	okbutton = new JButton ("OK");
	okbutton.addActionListener(this);
	buttonspanel.add (okbutton);
	cancelbutton = new JButton ("Cancel");
	cancelbutton.addActionListener(this);
	buttonspanel.add (cancelbutton);
	panel.add (buttonspanel, "South");

	makeItems();
	addWindowListener(this);

	setFrameSize();
	setVisible(true);
    }



    public void setFrameSize() {
	Dimension d = sppanel.getPreferredSize();
	Dimension d2 = buttonspanel.getPreferredSize();
	d.width = Math.max (d.width+42, d2.width +20);
	d.width = Math.min (d.width, Core.screenWidth);
	if (varnumbers)
	    d.height = Math.min (300, Core.screenHeight);
	else
	    d.height = Math.min (d.height + d2.height + 45, Core.screenHeight);
	setSize(d);
    }



    public void closeFrame() {
	/* closes this frame & informs it's parent - doesn't save any data or
	 * anything.
	 */
	closed = true;
	dispose();
    }



    private int makeItems() {
	/* fills up the scrollpane with all the items in enums, which is a vector
	 * of DescEnum
	 */
	int i;
	int ypos = 0;
	DescEnumElemPanel p;
	for (i = 0; (i < data.size()) && (varnumbers || (i < numvals)); i++) {
	    p = new DescEnumElemPanel((DescEnum)data.elementAt(i),
				      pdtype);
	    elempanels.addElement(p);
	    sppanel.add(p);
	}
	if (!varnumbers) {
	    for (; i < numvals; i++) {
		p = new DescEnumElemPanel(pdtype);
		elempanels.addElement(p);
		sppanel.add(p);
	    }
	}
	sppanel.validate();
	return data.size();
    }



    public void actionPerformed (ActionEvent e) {
	DescEnumElemPanel p;
	int unused,j;
	float k;
	String s = null;
	if (e.getSource() == insertbutton) {
	    p = new DescEnumElemPanel(pdtype);
	    elempanels.addElement(p);
	    sppanel.add(p);

	    if (elempanels.size() == 1) { // make sure panel wide enough
		setFrameSize();
		Dimension d = getSize();
		Dimension d2 = sppanel.getPreferredSize();
		d.width = Math.max (d.width, d2.width + 42);
		d.width = Math.min (d.width, Core.screenWidth);
		setSize (d);
	    }
	    //setFrameSize();
	    validate();
	}
	else if (e.getSource() == removebutton) {
	    for (int i = 0; i < elempanels.size(); ) {
		p = (DescEnumElemPanel)elempanels.elementAt(i);
		if (p.getSelected()) {
		    sppanel.remove(p);
		    elempanels.removeElementAt(i);
		}
		else
		    i++;
	    }
	    // validate & repaint needed to get everything repositioned & redrawn
	    validate();
	    sppanel.repaint (sppanel.getBounds());
	}
	else if (e.getSource() == cancelbutton) {
	    closeFrame();
	}
	else if (e.getSource() == okbutton) {
	    unused = 0;
	    data.removeAllElements();
	    VarValue val;
	    
	    if (pdtype.equals (ValType.t_embeddedchunk) || pdtype.equals (ValType.t_chunk))
		val = new VarValue(new ValType(ValType.t_string));
	    else
		val = new VarValue (pdtype);

	    for (int i = 0; i < elempanels.size(); i++) {
		p = (DescEnumElemPanel)elempanels.elementAt(i);

		s = p.getName();
		val.set (p.getVal());
		//System.out.println ("name is " + s + " and val is " + val.toString());

		if (s == null || s.equals (""))
		    continue; 

		if (pdtype.equals (ValType.t_chunk) || pdtype.equals (ValType.t_embeddedchunk)) {
		    for (j = 0; j < Core.descdbs.size(); j++) {
			String s2 = ((ChunkDescDB)Core.descdbs.elementAt(j)).getTokenFromName (p.getName());
			if (s2 != null) {
			    s = s2;
			    val.set(s2);
			    break;
			}
		    }
		}

		if (pdtype.equals (ValType.t_int) || pdtype.equals (ValType.t_float)) {
		    if (p.getVal().equals("")) {
			val.set (unused++);
		    }
		    else
			unused = (unused > val.getInt())?(unused):(val.getInt()+1);
		}

		if (pdtype.equals (ValType.t_string) || pdtype.equals (ValType.t_chunk) || pdtype.equals (ValType.t_embeddedchunk))
		    if (p.getVal().equals(""))
			val.set (s);

		data.addElement (new DescEnum (s, val));
		
	    }
	    closeFrame();
	}
    }



    /* WindowListener Stuff */
    public void windowActivated(WindowEvent e) {}
    public void windowClosed(WindowEvent e) {}
    public void windowClosing(WindowEvent e) {
	closeFrame();
    }
    public void windowDeactivated(WindowEvent e) {}
    public void windowDeiconified(WindowEvent e) {}
    public void windowIconified(WindowEvent e) {}
    public void windowOpened(WindowEvent e) {}



}




