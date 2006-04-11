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



package VjConfig;

import java.util.Vector;
import java.io.DataInputStream;
import java.io.IOException;
import VjConfig.ConfigStreamTokenizer;
import VjConfig.ChunkDesc;



public class ConfigChunk {

    public Vector    props;
    public String    name;
    public ChunkDesc desc;



    public ConfigChunk (ConfigChunk c) {
	desc = c.desc;
	name = c.name;
	props = new Vector();
	for (int i = 0; i < c.props.size(); i++)
	    props.addElement (new Property ((Property)c.props.elementAt(i)));
    }



    public ConfigChunk (ChunkDesc d) {
	desc = d;
	name = "";
	props = new Vector();
	/* we start at one here cuz we don't include name, which is always
	 * the first element in a propertydesc
	 */
	for (int i = 1; i < d.props.size(); i++)
	    props.addElement (new Property((PropertyDesc)d.props.elementAt(i)));
    }



    public ConfigChunk (ChunkDesc d, String n) {
	desc = d;
	name = n;
	props = new Vector();
	for (int i = 1; i < d.props.size(); i++)
	    props.addElement (new Property((PropertyDesc)d.props.elementAt(i)));
    }



    public String getName() {
	return name;
    }

    public void setName (String s) {
	name = s;
    }

    public String getDescName() {
	return desc.name;
    }

    public String getDescToken() {
	return desc.token;
    }



    public boolean equals (ConfigChunk c) {
	Property p1, p2;
	if (!name.equals(c.name))
	    return false;
	if (!desc.token.equals(c.desc.token))
	    return false;

	/* This next part is O(n^2) <sigh> */
	for (int i = 0; i < props.size(); i++) {
	    p1 = (Property) props.elementAt(i);
	    p2 = c.getProperty(p1.token);
	    if (!p1.equals(p2))
		return false;
	}
	return true;
    }



    public Property getProperty(String n) {
	Property p;
	for (int i = 0; i < props.size(); i++) {
	    p = (Property)props.elementAt(i);
	    if (p.name.equalsIgnoreCase(n))
		return p;
	}
	return null;
    }



    public void setPropertyFromToken (String tok, VarValue val, int i) {
	// sets the ith value in property tok to val
	Property p = getPropertyFromToken (tok);
	if (p != null)
	    p.setValue (val, i);
    }



    public Property getPropertyFromToken(String n) {
	Property p;
	for (int i = 0; i < props.size(); i++) {
	    p = (Property)props.elementAt(i);
	    if (p.desc.token.equalsIgnoreCase(n))
		return p;
	}
	return null;
    }



    public boolean read (ConfigStreamTokenizer st) {
	try {
	    while (true) {
		st.nextToken();
		if (st.ttype == ConfigStreamTokenizer.TT_EOF)
		    break;
		if ((st.ttype != ConfigStreamTokenizer.TT_WORD) && 
		    (st.ttype != '"'))
		    continue;
		if (st.sval.equalsIgnoreCase("end"))
		    break;

		/* begin reading property info out of s */
		if (st.sval.equalsIgnoreCase("name")) {
		    st.nextToken();
		    if (st.ttype == '{') {
			st.nextToken();
			name = st.sval;
			st.nextToken();
		    }
		    else
			name = st.sval;
		}
		else {
		    Property p = getPropertyFromToken(st.sval);
		    if (p != null) {
			p.read(st);
		    }
		    else {
			System.err.println ("Error: ConfigChunk.Read() - no such property "
					    + st.sval);
		    }
		}
		/* finished reading property info from s */
	    }
	    return true;
	}
	catch (IOException io) {
	    System.err.println ("IO Error in ConfigChunk.read()");
	    return false;
	}
    }  



    public String toString () {
	return toString ("");
    }



    public String toString (String pad) {
	String s = pad + desc.token + "\n";
	s += pad + "  Name \"" + name + "\"\n";
	for (int i = 0; i < props.size(); i++)
	    s += ((Property)props.elementAt(i)).toString(pad + "  ") + "\n";
	s += pad + "  end\n";
	return s;
    }


}




