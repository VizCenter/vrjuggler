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

import java.io.DataInputStream;
import java.io.IOException;
import java.util.ArrayList;
import java.util.Iterator;
import java.util.List;
import org.jdom.Element;
import org.jdom.Attribute;


/** A ConfigChunk is a logical collection of configuration information.
 *  A vjConfigChunk stores a number of vjPropertys that describe
 *  the configuration of a particular component of the system.
 *  It has an associated vjChunkDesc which describes its type
 *  and the vjPropertys that belong to it.
 *
 *  @see ChunkDesc
 *  @version $Revision$
 */
public class ConfigChunk
{
   public final static String EMBEDDED_SEPARATOR = "/";

   public ConfigChunk (Element root)
   {
      mDomElement = root;
      desc = ChunkFactory.getChunkDescByToken(root.getName());

      if ( null == desc )
      {
         System.err.println("WARNING: Unknown chunk type '" + root.getName() +
                            "' encountered");
      }
   }

   public ConfigChunk (ConfigChunk srcChunk)
   {
      desc = srcChunk.desc;
      mDomElement = (Element) srcChunk.mDomElement.clone();
   }

   public ConfigChunk (ChunkDesc d)
   {
      desc = d;
      mDomElement = new Element(d.getToken());

      Iterator i = d.getPropertyDescs().iterator();
      PropertyDesc prop_desc;
      String prop_desc_type;

      while ( i.hasNext() )
      {
         prop_desc = (PropertyDesc) i.next();
         prop_desc_type = prop_desc.getToken();

         if ( prop_desc.hasVariableNumberOfValues() )
         {
            this.setProperty(prop_desc_type, 0, prop_desc.getDefaultValue(0));
         }
         else
         {
            int max_props = prop_desc.getNumAllowed();

            for ( int j = 0; j < max_props; ++j )
            {
               this.setProperty(prop_desc_type, j,
                                prop_desc.getDefaultValue(j));
            }
         }
      }
   }

   /**
    * Returns the symbolic, human-friendly name of this chunk.  If this chunk
    * does not have a name when this method is invoked, it is assigned here.
    */
   public final String getName ()
   {
      Attribute name_attr = mDomElement.getAttribute("name");

      // If this chunk does not yet have a name, we have to set one now.
      if ( null == name_attr )
      {
         name_attr = new Attribute("name", this.getDescToken());
      }

      return name_attr.getValue();
   }

   public final void setName (String s)
   {
      // We assign s to name and fullName to keep them in sync.  The call to
      // validateEmbeddedChunkNames() will deal with updating the fully
      // qualified names of embedded chunks.
      mDomElement.setAttribute("name", s);
   }

   /**
    * Returns the fully qualified, unique name of thsi chunk.  This will be
    * different from the value returned by getName() if this chunk is a child
    * of another chunk.  In that case, the full name will be the path to this
    * chunk based on the chunk hierarchy and the property tokens.
    */
   public final String getFullName ()
   {
      Element chunk_parent, prop_parent;
      String full_name = this.getName();

      chunk_parent = mDomElement;

      while ( (prop_parent = chunk_parent.getParent()) != null &&
              ! prop_parent.getName().equals(ConfigTokens.chunk_db_TOKEN) )
      {
         full_name = prop_parent.getName() + EMBEDDED_SEPARATOR + full_name;
         chunk_parent = prop_parent.getParent();
         full_name = chunk_parent.getAttribute("name").getValue() +
                     EMBEDDED_SEPARATOR + full_name;
      }

      return full_name;
   }

   //-------- Stuff for dealing with embedded chunk names ---------------

   /**
    * Retrieves the named embedded chunk from this chunk.
    *
    * @note Expects self's own name/prop pair at start of pathname.
    * @note
    */
   public ConfigChunk getEmbeddedChunk (String pathname)
   {
      ConfigChunk ch = null;

      // note: a nonrecursive version of this is probably worth it
      //       for the performance increase.  this is yicky.
      try
      {
         Element root = mDomElement;

         while ( ConfigChunk.hasSeparator(pathname) && root != null )
         {
            String prop_name = ConfigChunk.getFirstNameComponent(pathname);

            // Get the child element matching the current property name.
            Element property = root.getChild(prop_name);
            PropertyDesc property_desc = desc.getPropertyDesc(prop_name);

            // At this point, we (hopefully) have a valid child of prop_name
            // and the corresponding property description.  The next phase of
            // work will set root to the next child chunk element in pathname.
            // If it doesn't, then we're done or something went wrong.
            root = null;

            // If the path element is valid, we can continue.
            if ( null != property && null != property_desc &&
                 property_desc.getValType() == ValType.EMBEDDEDCHUNK )
            {
               // Chop off the part of the path that is prop_name.
               pathname = ConfigChunk.getNameRemainder(pathname);

               // Get the name of the chunk from the newly truncated
               // pathname string.
               String next_chunk_name =
                  ConfigChunk.getFirstNameComponent(pathname);

               // Chop off the part of the path that is next_chunk_name.
               pathname = ConfigChunk.getNameRemainder(pathname);

               // Now we have to search the children of property for the
               // next chunk name in the path.
               Iterator i = property.getChildren().iterator();

               while ( i.hasNext() )
               {
                  Element child = (Element) i.next();

                  // Hooray!  We found the next child chunk in the path.
                  // We're done with this phase.
                  if ( child.getAttribute("name").equals(next_chunk_name) )
                  {
                     root = child;
                     break;
                  }
               }
            }
         }

         // If we have a valid root element and its name attribute matches
         // what is left of pathname, then we found our chunk!
         if ( root != null && root.getAttribute("name").equals(pathname) )
         {
            ch = new ConfigChunk(root);
         }
      }
      catch (Exception e)
      {
         /* Do nothing.  We'll end up returning null. */ ;
      }

      return ch;
   }

   public static final String getLastNameComponent (String chunkName)
   {
      int i = chunkName.lastIndexOf(EMBEDDED_SEPARATOR);
      return ((i < 0) ? chunkName
                      : chunkName.substring(i + EMBEDDED_SEPARATOR.length()));
   }

   /**
    * This is a helper function for classes wanting to deal with fully
    * qualified chunk names.
    */
   public static final String getFirstNameComponent(String _name)
   {
      int i = _name.indexOf(EMBEDDED_SEPARATOR);
      return(i < 0) ? "" : _name.substring(0, i);
   }

   /* Returns everything after the first divider in a name.
    * If there is no separator, returns name.
    */
   public final static String getNameRemainder (String chunkName)
   {
      int i = chunkName.indexOf(EMBEDDED_SEPARATOR);
      return (i < 0) ? chunkName
                     : chunkName.substring(i + EMBEDDED_SEPARATOR.length());
   }

   public final static boolean hasSeparator (String chunkName)
   {
      return (chunkName.indexOf(EMBEDDED_SEPARATOR) >= 0);
   }

   public final ChunkDesc getDesc()
   {
      return desc;
   }

   public final PropertyDesc getPropertyDesc (String propertyToken)
   {
      return desc.getPropertyDesc(propertyToken);
   }

   public final String getDescName()
   {
      return desc.getName();
   }

   /**
    * Returns the type of this chunk using its chunk description.  The type of
    * this chunk is defined by its description's token/type identifier.  If
    * this chunk has no description, null is returned.
    *
    * @return A string containing this chunk's type or null if this chunk has
    *         no description.
    */
   public final String getDescToken()
   {
      String desc_token = null;

      if ( null != desc )
      {
         desc_token = desc.getToken();
      }

      return desc_token;
   }

   public final String getDescHelp()
   {
      return desc.getHelp();
   }

   /** This is helpful for the GUI. */
   public List getEmbeddedChunks()
   {
      List chunks = new ArrayList();

      Iterator i = this.getDesc().getPropertyDescs().iterator();
      String full_name = this.getName();

      while ( i.hasNext() )
      {
         PropertyDesc prop_desc = (PropertyDesc) i.next();

         if ( prop_desc.getValType() == ValType.EMBEDDEDCHUNK )
         {
            String prop_type = prop_desc.getToken();
            int prop_count = this.getPropertyCount(prop_type);
            ConfigChunk emb_chunk;

            for ( int j = 0; j < prop_count; ++j )
            {
               emb_chunk = this.getProperty(prop_type, j).getEmbeddedChunk();

               // Do not return null child chunks.
               if ( null != emb_chunk )
               {
                  chunks.add(emb_chunk);
               }
            }
         }
      }

      return chunks;
   }

   public boolean equals(ConfigChunk c)
   {
      return this.mDomElement == c.mDomElement;
   }

   /**
    * Returns the named property value in propStorage if the property exists
    * in this chunk.
    */
   public VarValue getProperty (String propType)
   {
      return getProperty(propType, 0);
   }

   /**
    * Gets the value of the given property type at the specified index.
    *
    * @param propType A property type that matches those defined in this
    *                 chunk's description.
    * @param index    The indexed property matching the named type.
    *
    * @return A valid VarValue object is returned if the named property type
    *         exists within this chunk and the index is within the allowed
    *         range.  Otherwise, null is returned.
    *
    * @note The rest of the code expects this method to return null if the
    *       index is valid but not within the range of currently defined
    *       values.  This seems incorrect, but it may be too hard to change
    *       every place.  Either that, or determining the difference between
    *       an "empty" VarValue object and a default value may be too hard.
    */
   public VarValue getProperty (String propType, int index)
   {
      VarValue value = null;

      // Get the property description for the given property type.
      PropertyDesc prop_desc = desc.getPropertyDesc(propType);

      // Invalid property type!  We have to return null.
      if ( null == prop_desc )
      {
         System.err.println("WARNING: Invalid property type '" + propType +
                            "' requested in chunk of type '" +
                            desc.getToken() + "'");
      }
      // The property type is valid, so we can look up the indexed value.
      else
      {
         // JDOM will return an empty list if there are no children matching
         // propType.
         List props = mDomElement.getChildren(propType);

         // Extract the information relevant for the given property type.
         ValType val_type = prop_desc.getValType();

         // Verify that the index is valid.
         // XXX: Remove the test for the size of props to return a default
         // value rather than null when the index is valid but just doesn't
         // have a value yet.
         if ( props.size() > 0 &&
              (index < prop_desc.getNumAllowed() ||
               prop_desc.hasVariableNumberOfValues()) )
         {
            // Ensure that the index actually falls within the loaded list of
            // property values.
            if ( index < props.size() )
            {
               // Look up the property in the list.
               Element prop = (Element) props.get(index);
               String prop_val = prop.getTextTrim();
               value = new VarValue(val_type);

               // A child chunk property.
               if ( ValType.EMBEDDEDCHUNK == val_type )
               {
                  // This gets the child of prop that is the embedded chunk.
                  // Using the description enumeration string seems like a
                  // strange way to get it, but that's what the API has right
                  // now.
                  String chunk_type = prop_desc.getEnumAtIndex(0).str;

                  Element child = prop.getChild(chunk_type);

                  // If we have a child chunk, we can return it.  Otherwise,
                  // we return an empty VarValue object.
                  if ( null != child )
                  {
                     ConfigChunk child_chunk = new ConfigChunk(child);
                     value.set(child_chunk);
                  }
               }
               // All other property types.
               else
               {
                  value.set(prop_desc.getEnumValue(prop_val));
               }
            }
            // The index is outside the range of currently defined property
            // values.  In this case, we fall back on the default value.
            // Thank goodness for those!
            else
            {
               value = prop_desc.getDefaultValue(index);
            }
         }
      }

      return value;
   }

   public int getPropertyCount (String propType)
   {
      List children = mDomElement.getChildren(propType);
      int count = 0;

      if ( null != children )
      {
         count = children.size();
      }

      return count;
   }

   /**
    * Sets the value for the property of type propType at the given index.
    *
    * @pre value must not be null.
    *
    * @param propType The property description identifier/token for the
    *                 property being set.
    * @param index    The index within the named property type that will be
    *                 set.  If this index is not within the valid range, no
    *                 value is set.
    * @param value    The value to be set.  It must not be a null reference.
    *
    * @return A boolean value stating whether or not the set operation
    *         succeeded is returned to the caller.
    */
   public boolean setProperty (String propType, int index, VarValue value)
   {
      List props = mDomElement.getChildren(propType);
      boolean status = false;

      PropertyDesc prop_desc = desc.getPropertyDesc(propType);

      // Verify that we can set the property value at the given index.
      if ( index < prop_desc.getNumAllowed() ||
           prop_desc.hasVariableNumberOfValues() )
      {
         if ( null != props && index < props.size() )
         {
            Element prop = (Element) props.get(index);

            // The special case for embedded chunks is needed because we do
            // not want to insert them as text strings.
            if ( value.getValType() == ValType.EMBEDDEDCHUNK )
            {
               prop.removeChildren();
               prop.addContent(value.getEmbeddedChunk().getNode());
            }
            else
            {
               prop.setText(value.toString());
            }
         }
         // The property does not currently exist, so we'll add it.
         else
         {
            Element prop = new Element(propType);

            // The special case for embedded chunks is needed because we do
            // not want to insert them as text strings.
            if ( value.getValType() == ValType.EMBEDDEDCHUNK )
            {
               if ( value.getEmbeddedChunk() != null )
               {
                  prop.addContent(value.getEmbeddedChunk().getNode());
               }
            }
            else
            {
               prop.addContent(value.toString());
            }

            mDomElement.addContent(prop);
         }

         status = true;
      }

      return status;
   }

   /**
    * Removes the property of type propType at the given index.
    */
   public boolean removeProperty (String propType, int index)
   {
      List props = mDomElement.getChildren(propType);
      boolean status = false;

      PropertyDesc prop_desc = desc.getPropertyDesc(propType);

      if ( index < props.size() )
      {
         Element del_prop = (Element) props.get(index);
         mDomElement.removeContent(del_prop);
         status = true;
      }

      return status;
   }

   /** Returns a vector of ConfigChunk names this chunk depends on */
   public List getDependencyNames()
   {
      ArrayList results = new ArrayList();

      Iterator i= desc.getPropertyDescs().iterator();

      while ( i.hasNext() )
      {
         PropertyDesc prop_desc = (PropertyDesc) i.next();

         if ( prop_desc.getValType() == ValType.CHUNK )
         {
            // Get all the elements matching the current property description
            // token.
            Iterator j = mDomElement.getChildren(prop_desc.getToken()).iterator();

            while ( j.hasNext() )
            {
               String dep_text = ((Element) j.next()).getTextTrim();

               // JDOM will return an empty string rather than null if the
               // element has no text.
               if ( ! dep_text.equals("") )
               {
                  results.add(dep_text);
               }
            }
         }
         else if ( prop_desc.getValType() == ValType.EMBEDDEDCHUNK )
         {
            // Get all the elements matching the current property description
            // token.
            Iterator j = mDomElement.getChildren(prop_desc.getToken()).iterator();

            while ( j.hasNext() )
            {
               Element parent_prop = (Element) j.next();

               // This is needed to go to to next level in the hierarchy--that
               // is, to the position of the actual embedded chunk.
               Iterator k = parent_prop.getChildren().iterator();

               while ( k.hasNext() )
               {
                  ConfigChunk child_chunk = new ConfigChunk((Element) k.next());
                  results.addAll(child_chunk.getDependencyNames());
               }
            }
         }
      }

      return results;
   }

   public final String toString()
   {
      org.jdom.output.XMLOutputter outputter =
         new org.jdom.output.XMLOutputter("   ", true);
      outputter.setLineSeparator(System.getProperty("line.separator"));
      return outputter.outputString(mDomElement);
   }

   /**
    * Returns the DOM tree node for this chunk.  The constructors guarantee
    * that it is non-null.  It is package visible so that code in other
    * packages cannot abuse this implementation exposure.
    */
   Element getNode ()
   {
      return mDomElement;
   }

   private Element mDomElement = null;

   private ChunkDesc desc   = null;
}
