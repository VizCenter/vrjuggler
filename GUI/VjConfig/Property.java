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



package VjConfig;

import java.util.Vector;
import java.io.StreamTokenizer;
import java.util.NoSuchElementException;
import java.io.IOException;
import VjConfig.PropertyDesc;
import VjConfig.ChunkFactory;

public class Property {

    public String name;
    public String token;
    private int num;
    public PropertyDesc desc;
    ValType valtype;
    public Vector vals;
    public ChunkDesc embeddesc; // used only for t_embeddedchunk


    public Property (Property p) {
	name = p.name;
	token = p.token;
	num = p.num;
	desc = p.desc;
	embeddesc = p.embeddesc;
	valtype = new ValType (p.valtype);
	vals = new Vector();
	for (int i = 0; i < p.vals.size(); i++)
	    vals.addElement (new VarValue ((VarValue)p.vals.elementAt(i)));
    }



    public Property (PropertyDesc d) {
	
	desc = d;
	name = d.name;
	token = d.token;
	valtype = d.valtype;
	num = d.num;
	vals = new Vector();
	if (valtype.equals (ValType.t_embeddedchunk)) {
	    embeddesc = ChunkFactory.getChunkDescByToken(d.getEnumAtIndex(0).str);
	    if (embeddesc == null) {
		System.err.println ("Big Messup in Property Constructor!!!");
		embeddesc = new ChunkDesc ();
	    }
	}
	else {
	    embeddesc = null;
	}
	for (int i = 0; i < num; i++)
	    vals.addElement (createVarValue(i));

    }



    private VarValue createVarValue (int i) {
	// if i == -1, we're just tacking onto the end
	if (i == -1)
	    i = vals.size();
	if (valtype.equals (ValType.t_embeddedchunk)) {
	    ConfigChunk ch = ChunkFactory.createChunk (embeddesc);
	    if (desc.valuelabels.size() > i)
		ch.setName (((DescEnum)desc.valuelabels.elementAt(i)).str);
	    else
		ch.setName (desc.name + " " + i);
	    return new VarValue (ch);
	}
	else
	    return new VarValue (valtype);
    }


    public String getName () {
	return desc.name;
    }

    public String getToken () {
	return desc.token;
    }


    public void setValue (VarValue s, int v) {
	if (!valtype.equals (s.getValType()))
	    return;
	if (v < 0)
	    return;
	if (num == -1) {
	    while (v >= vals.size())
		vals.addElement (createVarValue(-1));
	}
	else if (v >= num)
	    return;
	((VarValue)vals.elementAt(v)).set(s);
    }



    public VarValue getValue (int i) {
	if (i < 0 || i >= vals.size())
	    return null;
	return (VarValue)vals.elementAt(i);
    }



    public int getNum () {
	return vals.size();
    }



    public boolean equals (Property p) {
	/* note - this does a total value comparison, not just checking names
	 * The exception to this is that right now I only check the name of the
	 * chunkdesc
	 */
	VarValue v1,v2;

	if (!name.equals(p.name))
	    return false;
	if (num != p.num)
	    return false;
	if (!desc.name.equals(p.desc.name))
	    return false;
	if (!valtype.equals(p.valtype))
	    return false;
	if (vals.size() != p.vals.size())
	    return false;
	for (int i = 0; i < vals.size(); i++) {
	    v1 = (VarValue)vals.elementAt(i);
	    v2 = (VarValue)p.vals.elementAt(i);
	    if (!v1.equals(v2))
		return false;
	}
	return true;
    }



    public String toString(String pad) {
	VarValue v;
	String s = pad + desc.token + " { ";
	for (int i = 0; i < vals.size(); i++) {
	    v = (VarValue)vals.elementAt(i);

	    if (valtype.equals (ValType.t_embeddedchunk)) {
		s += "\n" + v.getEmbeddedChunk().toString(pad + "    ");
	    }
	    else {
		s += "\"" + desc.getEnumString(v) + "\"";
	    }

	    s+= " ";
	}
	s += "}";
	return s;
    }



    public boolean read (ConfigStreamTokenizer st) throws IOException {
	/* the idea here is that st holds the string, less the property name
	 * which was already read by ConfigChunk.read()
	 */
	VarValue v;
	int n = 0;
	try {
	    int t = st.nextToken();
	    String token = st.sval;
	    
	    if (st.ttype == '{') {
		/* multiple entries */
		vals.removeAllElements();
		while ((v = buildVarValue (st)) != null) 
		    vals.addElement(v);
		if (st.ttype != '}') {
		    System.out.println ("ERROR: expected '}'");
		}
	    }
	    else {
		/* one entry */
		v = buildVarValue(st);
		vals.removeAllElements();
		vals.addElement(v);
	    }
	    if (num != -1) {
		/* are we a proper # of values? */
		while (vals.size() < num)
		    vals.addElement (new VarValue(valtype));
	    }
	    return true;
	}
	catch (IOException e) {
	    /* Either a streamtokenizer error or unexpected eof */
	    System.err.println ("Error in Property.read()");
	    return false;
	}
    }



    private VarValue buildVarValue (ConfigStreamTokenizer st) {
	/* here we build a token into an appopriate VarValue; includes
	 * type resolution and enumeration lookup 
	 */
	try {
	    st.nextToken();
	}
	catch (IOException e) {
	    return null;
	}

	if (st.ttype == '}')
	    return null;

	if (valtype.equals (ValType.t_embeddedchunk)) {
	    if (embeddesc != null) {
		ConfigChunk c = ChunkFactory.createChunk (embeddesc);
		c.read(st);
		return new VarValue (c);
	    }
	    return null;
	}
	else try {
	    return (desc.getEnumValue(st.sval));
	}
	catch (NoSuchElementException e) {
	    VarValue v = createVarValue(-1);
	    v.set(st.sval);
	    return v;
	}
    }



//     public boolean containsValue (String s) {
// 	int i;
// 	String s2;

// 	if (!valtype.equals(ValType.t_string)) {
// 	    for (i = 0; i < vals.size(); i++) {
// 		s2 = ((VarValue)vals.elementAt(i)).getString();
// 		if (s2.equalsIgnoreCase(s))
// 		    return true;
// 	    }
// 	}
// 	return false;
//     }



}



