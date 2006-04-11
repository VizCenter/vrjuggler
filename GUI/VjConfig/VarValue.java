/*
 *  File:	    $RCSfile$
 *  Date modified:  $Date$
 *  Version:	    $Revision$
 *
 *
 *                                VR Juggler
 *                                    by
 *                              Allen Bierbaum
 *                             Christopher Just
 *                            Carolina Cruz-Neira
 *                               Albert Baker
 *
 *                         Copyright  - 1997,1998,1999
 *                Iowa State University Research Foundation, Inc.
 *                            All Rights Reserved
 */



package VjConfig;

import VjConfig.*;
import java.lang.Cloneable;

public class VarValue {
  /* of course, this can't do any of the leet things the C++ varvalue
   * class does, because Sun would track us down and kill us for doing
   * things like that in Java ;)
   */

    private String         strval;
    private int            intval;
    private float          floatval;
    private boolean        boolval;
    private ConfigChunk    embeddedchunkval;

    private ValType        valtype;



    public VarValue (VarValue v) {
	valtype = v.valtype;
	strval = v.strval;
	intval = v.intval;
	floatval = v.floatval;
	boolval = v.boolval;
	if (v.embeddedchunkval == null)
	    embeddedchunkval = null;
	else
	    embeddedchunkval = new ConfigChunk (v.embeddedchunkval);
    }



    public VarValue (ValType t) {
	valtype = t;
	strval = "";
	embeddedchunkval = null;
    }

    public VarValue (int v) {
	valtype = new ValType(ValType.t_int);
	intval = v;
    }

    public VarValue (float v) {
	valtype = new ValType (ValType.t_float);
	floatval = v;
    }

    public VarValue (boolean v) {
	valtype = new ValType (ValType.t_bool);
	boolval = v;
    }

    public VarValue (String v) {
	valtype = new ValType (ValType.t_string);
	strval = v;
    }

    public VarValue (ConfigChunk v) {
	valtype = new ValType (ValType.t_embeddedchunk);
	embeddedchunkval = v;
    }



    public boolean equals(VarValue v) {
	if (!valtype.equals(v.valtype))
	    return false;
	else if (valtype.equals (ValType.t_int))
	    return (intval == v.intval);
	else if (valtype.equals (ValType.t_float))
	    return (floatval == v.floatval);
	else if (valtype.equals (ValType.t_bool))
	    return (boolval == v.boolval);
	else if (valtype.equals (ValType.t_embeddedchunk))
	    return (embeddedchunkval.equals (v.embeddedchunkval));
	else
	    return (strval.equals(v.strval));
    }



    public String toString() {
	String s = null;
	if (valtype.equals(ValType.t_bool)) {
	    s = (boolval)?"True":"False";
	}
	else if (valtype.equals(ValType.t_string)) {
	    s = strval;
	}
	else if (valtype.equals(ValType.t_chunk)) {
	    s = strval;
	}
	else if (valtype.equals(ValType.t_int)) {
	    s = String.valueOf(intval);
	}
	else if (valtype.equals(ValType.t_float)) {
	    s = String.valueOf(floatval);
	}
	else if (valtype.equals (ValType.t_embeddedchunk)) {
	    s = embeddedchunkval.toString();
	}
	return s;
    }
    


    public ValType getValType () {
	return valtype;
    }



    /* I've decided life will be a lot easier if set is a bit smarter about
     * converting strings into different things
     */
    public void set (String s) {
	try {
	    if (valtype.equals (ValType.t_string))
		strval = s;
	    else if (valtype.equals (ValType.t_chunk))
		strval = s;
	    else if (valtype.equals (ValType.t_bool))
		boolval = s.equalsIgnoreCase("true")? true:false;
	    else if (valtype.equals (ValType.t_int)) {
		intval = 0;
		intval = Integer.parseInt(s);
	    }
	    else if (valtype.equals (ValType.t_float)) {
		floatval = 0;
		floatval = new Float(s).floatValue();
		/* why doesn't Float have something analogous to 
		 * Integer.parseInt()? 
		 */
	    }
	    else if (valtype.equals (ValType.t_embeddedchunk)) {
		embeddedchunkval = null;
		throw (new VarValueException ("Error assigning string to " +
					      "embeddedchunk VarValue"));
	    }
	}
	catch (NumberFormatException e) {
	    /* just so it's defined: if an error occurred, we set the numeric
	     * values to 0
	     */
	    intval = 0;
	    floatval = 0;
	    throw (new VarValueException ("Error assigning '" + s 
					  + "' to VarValue"));
	}
    }



    public void set (boolean s) {
	if (valtype.equals(ValType.t_bool))
	    boolval = s;
	else
	    throw (new VarValueException ("Error assigning to Boolean VarValue"));
    }


    public void set (int s) {
	if (valtype.equals(ValType.t_int))
	    intval = s;
	else
	    throw (new VarValueException ("Error assigning to Integer VarValue"));
    }


    public void set (float s) {
	if (valtype.equals(ValType.t_float))
	    floatval = s;
	else
	    throw (new VarValueException ("Error assigning to Float VarValue"));
    }


    public void set (ConfigChunk ch) {
	if (valtype.equals (ValType.t_embeddedchunk))
	    embeddedchunkval = ch;
	else
	    throw (new VarValueException ("Error assigning to EmbeddedChunk VarValue"));
    }


    public void set (VarValue s) {
	/* so we can copy one varvalue into another */
	if (!valtype.equals(s.getValType()))
	    throw (new VarValueException ("Incompatible types assigning one VarValue to another."));
	boolval = s.boolval;
	intval = s.intval;
	floatval = s.floatval;
	strval = s.strval;
	embeddedchunkval = s.embeddedchunkval;
    }


    public int getInt() {
	if (valtype.equals(ValType.t_int))
	    return intval;
	else
	    throw (new VarValueException ("Error in getInt()"));
    }


    public float getFloat() {
	if (valtype.equals(ValType.t_float))
	    return floatval;
	else
	    throw (new VarValueException ("Error in getFloat()"));
    }


    public boolean getBool() {
	if (valtype.equals(ValType.t_bool))
	    return boolval;
	else
	    throw (new VarValueException ("Error in getBool()"));
    }


    public String getString() {
	if (valtype.equals(ValType.t_string) || valtype.equals(ValType.t_chunk))
	    return strval;
	else
	    throw (new VarValueException ("Error in getInt()"));
    }


    public ConfigChunk getEmbeddedChunk() {
	if (valtype.equals (ValType.t_embeddedchunk))
	    return embeddedchunkval;
	else
	    throw (new VarValueException ("Error in getEmbeddedChunk()"));
    }


    public Object get() {
	if (valtype.equals(ValType.t_string))
	    return strval;
	else if (valtype.equals(ValType.t_chunk))
	    return strval;
	else if (valtype.equals(ValType.t_int))
	    return new Integer(intval);
	else if (valtype.equals(ValType.t_float))
	    return new Float(floatval);
	else if (valtype.equals(ValType.t_bool))
	    return new Boolean(boolval);
	else if (valtype.equals (ValType.t_embeddedchunk))
	    return embeddedchunkval;
	else
	    throw (new VarValueException ("Error in get()"));
    }


}

