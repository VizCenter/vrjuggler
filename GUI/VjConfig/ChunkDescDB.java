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


/* corresponds to ChunkDescDB.C/.h */

package VjConfig;

import java.util.Vector;
import java.io.*;

import VjConfig.DescDBEvent;
import VjConfig.DescDBListener;

public class ChunkDescDB {

    public String name;
    public File file;

    private Vector targets;
    private Vector descs;

    public ChunkDescDB () {
	name = "Unnamed";
	file = new File ("");
	targets = new Vector();
	descs = new Vector();
    }


    public void setName(String _name) {
	name = _name;
    }


    public String getName () {
	return name;
    }


    public void setFile (File _file) {
	file = _file;
    }


    public int size () {
	return descs.size();
    }


    public ChunkDesc elementAt (int i) {
	return (ChunkDesc)descs.elementAt(i);
    }


    public ChunkDescDB diff (ChunkDescDB d) {
	/* builds a new ChunkDescDB that's sort of the difference of its
	 * arguments - the returned db contains every chunk in d that isn't
	 * in self or differs from the same-named chunk in self 
	 */
	ChunkDesc c1, c2;
	ChunkDescDB newdb = new ChunkDescDB();
	
	for (int i = 0; i < d.descs.size(); i++) {
	    c1 = (ChunkDesc)d.descs.elementAt(i);
	    c2 = getByToken (c1.token);
	    if ((c2 == null) || (!c1.equals(c2)))
		newdb.descs.addElement(c1);
	}
	return newdb;
    }


    public void replace (ChunkDesc oldc, ChunkDesc newc) {
	int i = descs.indexOf (oldc);
	if (i >= 0) {
	    DescDBEvent e = new DescDBEvent (this, DescDBEvent.REPLACE, oldc, newc);
	    descs.setElementAt (newc, i);
	    notifyDescDBTargets (e);
	}
	else
	    addElement (newc);
    }
    
    public void removeAll () {
	descs.removeAllElements();
	notifyDescDBTargets (new DescDBEvent (this, DescDBEvent.REMOVEALL, null, null));
    }

    public boolean remove(String tok) {
	for (int i = 0; i < descs.size(); i++) {
	    ChunkDesc d = (ChunkDesc)descs.elementAt(i);
	    if (d.token.equals(tok)) {
		DescDBEvent e = new DescDBEvent (this, DescDBEvent.REMOVE, d, null);
		descs.removeElementAt(i);
		notifyDescDBTargets (e);
		return true;
	    }
	}
	return false;
    }


    public boolean removeByName(String tok) {
	for (int i = 0; i < descs.size(); i++) {
	    ChunkDesc d = (ChunkDesc)descs.elementAt(i);
	    if (d.name.equalsIgnoreCase(tok)) {
		DescDBEvent e = new DescDBEvent (this, DescDBEvent.REMOVE, d, null);
		descs.removeElementAt(i);
		notifyDescDBTargets (e);
		return true;
	    }
	}
	return false;
    }
    
    public Vector getTokenBegins (String tok) {
	ChunkDesc d;
	Vector v = new Vector();
	for (int i = 0; i < descs.size(); i++) {
	    d = (ChunkDesc)descs.elementAt(i);
	    if (d.token.startsWith (tok))
		v.addElement(d);
	}
	return v;
    }





    public ChunkDesc getByToken (String tok) {
	for (int i = 0; i < descs.size(); i++) {
	    if (((ChunkDesc)descs.elementAt(i)).token.equalsIgnoreCase(tok)) {
		return (ChunkDesc)descs.elementAt(i);
	    }
	}
	return null;
    }
    


    public ChunkDesc getByName (String tok) {
	for (int i = 0; i < descs.size(); i++)
	    if (((ChunkDesc)descs.elementAt(i)).name.equalsIgnoreCase(tok))
		return (ChunkDesc)descs.elementAt(i);
	return null;
    }



    public String getTokenFromName (String n) {
	for (int i = 0; i < descs.size(); i++)
	    if (((ChunkDesc)descs.elementAt(i)).name.equalsIgnoreCase(n))
		return ((ChunkDesc)descs.elementAt(i)).token;
	return null;
    }



    public String getNameFromToken (String n) {
	for (int i = 0; i < descs.size(); i++)
	    if (((ChunkDesc)descs.elementAt(i)).token.equalsIgnoreCase(n))
		return ((ChunkDesc)descs.elementAt(i)).name;
	return null;
    }



    public void addElement (ChunkDesc d) {
	remove (d.token);
	descs.addElement(d);
	DescDBEvent e = new DescDBEvent (this, DescDBEvent.INSERT, null, d);
	notifyDescDBTargets (e);
    }


    public void addElements (Vector v) {
	for (int i = 0; i < v.size(); i++)
	    addElement ((ChunkDesc)v.elementAt(i));
    }


    public void addElements (ChunkDescDB db) {
	addElements (db.descs);
    }

    public boolean read (ConfigStreamTokenizer st) {
	String name;
	ChunkDesc c;

	try {
	    while (true) {
		st.nextToken();
		if (st.sval.equalsIgnoreCase ("end"))
		    return true;
		if (!st.sval.equalsIgnoreCase("chunk"))
		    throw new IOException();
		st.nextToken();
		name = st.sval;
		//System.err.println ("Reading chunkDesc: " + name);
		c = new ChunkDesc (name);
		c.read(st);
		addElement(c);
	    }

	}
	catch (IOException io) {
	    System.err.println ("IO Error in ChunkDescDB.read()");
	    return false;
	}
    }

    public String fileRep () {
	String s = "";
	ChunkDesc d;
	for (int i = 0; i < descs.size(); i++) {
	    d = (ChunkDesc)descs.elementAt(i);
	    s += d.toString();
	}
	s += "End\n";
	return s;
    }



    /******************** DescDB Target Stuff *********************/

    public synchronized void addDescDBListener (DescDBListener l) {
	synchronized (targets) {
	    targets.addElement (l);
	}
    }

    public void removeDescDBListener (DescDBListener l) {
	synchronized (targets) {
	    targets.removeElement (l);
	}
    }

    protected void notifyDescDBTargets (DescDBEvent e) {
	Vector l;
	synchronized (targets) {
	    l = (Vector) targets.clone();
	}
	for (int i = 0; i < l.size(); i++) {
	    DescDBListener lis = (DescDBListener)l.elementAt (i);
            int a = e.getAction();
	    if (a == e.INSERT)
		lis.addDesc (e);
	    else if (a == e.REMOVE)
		lis.removeDesc (e);
	    else if (a == e.REPLACE)
		lis.replaceDesc (e);
	    else if (a == e.REMOVEALL)
		lis.removeAllDescs (e);
	}
    }

}




