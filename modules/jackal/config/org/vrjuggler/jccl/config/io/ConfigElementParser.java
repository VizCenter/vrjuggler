/*************** <auto-copyright.pl BEGIN do not edit this line> **************
 *
 * VR Juggler is (C) Copyright 1998-2003 by Iowa State University
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
package org.vrjuggler.jccl.config.io;

import java.io.*;
import java.util.*;
import javax.swing.JOptionPane;
import javax.xml.transform.*;
import org.jdom.*;
import org.jdom.transform.*;
import org.jdom.output.*;
import org.vrjuggler.tweek.services.EnvironmentService;

import org.vrjuggler.jccl.config.*;

/**
 * This object knows how to create a configuration element from the node in a
 * DOM tree that describes it.
 */
public class ConfigElementParser
   implements ConfigIOConstants
{
   /**
    * Creates a new parser for configuration element XML nodes in which the
    * given configuration definition repository is used to lookup the required
    * configuration definitions.
    *
    * @param repos         the repository in which to lookup config definitions
    * @param searchPath    the path in which to look for config definitions when
    *                      they cannot be found in the repository
    */
   public ConfigElementParser(ConfigDefinitionRepository repos/*, List searchPath*/)
   {
      mDefinitionRepos = repos;
      //mSearchPath = searchPath;
   }

   /**
    * Parses the given XML DOM element for the configuration element defined
    * therein. If the element requires a configuration definition that is not in
    * this parser's repository and it can be found, the definition will be added
    * to the repository.
    *
    * @param root          the XML DOM element to parse
    */
   public ConfigElement parse(Element root)
      throws ParseException
   {
      // Get the name of the definition this element depends on
      String token = root.getName();

      // Get the definition version this element depends on
      String version_str = root.getAttributeValue(VERSION);
      if (version_str == null)
      {
         throw new MissingAttributeException(VERSION);
      }
      int our_version;
      try
      {
         our_version = Integer.parseInt(version_str);
      }
      catch (NumberFormatException nfe)
      {
         throw new ParseException("Version string is not a number: "+version_str);
      }

      // Get the name of this configuration element
      String name = root.getAttributeValue(NAME);
      if (name == null)
      {
         throw new MissingAttributeException(NAME);
      }

      int newest_version_number;
      try
      {
         newest_version_number =
            mDefinitionRepos.getNewestVersionNumber(token).intValue();
      }
      catch (DefinitionLookupException dle)
      {
         throw new ParseException(dle.getMessage());
      }
      
      // - If we have an old version
      //   - For i=our_version ; i < newest_version_num ; i++
      //     - Transform from version i to i+1
      if(newest_version_number > our_version)
      {

         JOptionPane.showMessageDialog(null, 
               "ConfigElement \"" + name + "\", of type \"" + token + "\", is an old version.\n"
               + " We are updating your configuration to the new ConfigElement format.\n" 
               + " You must save your configuration in order for these changes to become permanent.");
         
         // We must iterate over all versions between ours and the newest
         // transforming for each new version.
         for(int ver_num = our_version ; ver_num < newest_version_number ; ++ver_num)
         {
            // Get the XSLT from the next versions definition file.
            Element xslt_element = mDefinitionRepos.get(token,ver_num + 1).getXsltElement();
            if(null != xslt_element)
            {
               // Transform the element to the next version.
               root = transformElement(root, xslt_element);
            }
            else
            {
               throw new ParseException("Could not find XSLT transformation for: "
                                        + token + " Version: " + (ver_num+1));
            }
         }

         // Use the following to debug the transformations by outputting the
         // current state of root.

         /*
         System.out.println("Element:");
         XMLOutputter outp = new XMLOutputter();
         outp.setTextNormalize(true);
         outp.setIndent("  ");
         outp.setNewlines(true);
         try
         {
            outp.output(root, System.out); 
         }
         catch(Exception ex)
         {
            System.out.println("Somthing bad happened...");
         }
         */
      }

      // Get the definition for this configuration element
      ConfigDefinition def = getDefinition(token, newest_version_number);
      if (def == null)
      {
         throw new ParseException("Could not find definition: "+token+" version "+newest_version_number);
      }

      // Get all the properties for this element
      Map props = parseProperties(root, def);

      // Create the configuration element
      return new ConfigElement(def, name, props);
   }


   private static Element transformElement(Element element_node, Element xslt_element)
      throws ParseException
   {
      JDOMSource xslt_source = new JDOMSource(xslt_element);
      JDOMSource source = new JDOMSource(element_node);

      try
      {
         Transformer transformer = TransformerFactory.newInstance()
            .newTransformer(xslt_source);
     
         JDOMResult result = new JDOMResult();
         transformer.transform(source, result);
         Element result_elm = null;
         java.util.List temp_list = result.getResult();
         
         if(temp_list.size() > 0)
         {
            result_elm = (Element)temp_list.get(0);
            return result_elm;
         }
      }
      catch(Exception ex)
      {
         throw new ParseException("Could not transform using XSLT.");
      }
      return null;
   }

   /**
    * Parses all the property XML DOM nodes in the given configuration element
    * root node using the given configuration definition as a reference.
    */
   private Map parseProperties(Element root, ConfigDefinition def)
      throws ParseException
   {
      Map props = new TreeMap();
      
      // Process each property defined in the configuration definition
      List prop_defs = def.getPropertyDefinitions();
      for (Iterator itr = prop_defs.iterator(); itr.hasNext(); )
      {
         PropertyDefinition prop_def = (PropertyDefinition)itr.next();
         String prop_token = prop_def.getToken();

         // Process the property values for this property
         props.put(prop_token,
                   parsePropertyValues(root.getChildren(prop_token, CFG_NS), prop_def));
      }

      return props;
   }

   /**
    * Parses all the property values that reference the given property
    * definition out of the list of XML DOM property nodes.
    */
   private List parsePropertyValues(List elements, PropertyDefinition propDef)
      throws ParseException
   {
      List values = new ArrayList();

      // Make sure that a property that is not variable has a number of values
      // equals to those in the definition
      boolean variable = propDef.isVariable();
      if (!variable && (elements.size() != propDef.getPropertyValueDefinitionCount()))
      {
         throw new ParseException("Invalid number of property values for the property "+propDef.getToken());
      }

      // Go through each property value XML element and parse it
      for (Iterator itr = elements.iterator(); itr.hasNext(); )
      {
         Element elt = (Element)itr.next();
         values.add(parsePropertyValue(elt, propDef));
      }

      return values;
   }

   /**
    * Parses an individual property value XML DOM node out of the given element
    * using the given property definition as a reference.
    */
   private Object parsePropertyValue(Element elt, PropertyDefinition propDef)
      throws ParseException
   {
      String valueStr = elt.getTextTrim();

      // If the value matches an enumeration, return the corresponding value
      if (propDef.getEnums().containsKey(valueStr))
      {
         return propDef.getEnums().get(valueStr);
      }

      // If the property type is not a configuration element, convert it to
      // that value type.
      if (propDef.getType() != ConfigElement.class)
      {
         return convertValue(valueStr, propDef.getType());
      }

      // The property type must be a configuration element, so recurse on it
      Element config_element_node = (Element)elt.getChildren().get(0);
      return parse(config_element_node);
   }

   /**
    * Converts the given value from a string to the given value type class.
    */
   private Object convertValue(String valueStr, Class type)
      throws ParseException
   {
      try
      {
         if (type == Boolean.class)
         {
            // If the input string is "1", change it to "true" so that the
            // Boolean constructor will handle it correctly.
            if ( valueStr.equals("1") )
            {
               valueStr = "true";
            }

            return new Boolean(valueStr);
         }
         else if (type == Integer.class)
         {
            return new Integer(valueStr);
         }
         else if (type == Float.class)
         {
            return new Float(valueStr);
         }
         else if (type == String.class)
         {
            return valueStr;
         }
         else if (type == ConfigElementPointer.class)
         {
            return new ConfigElementPointer(valueStr);
         }
         else
         {
            throw new ParseException("Unknown valuetype: "+type);
         }
      }
      catch (NumberFormatException nfe)
      {
         throw new ParseException("Invalid value: "+valueStr);
      }
   }

   /**
    * Given the token and version of the configuration definition to retrieve,
    * the appropriate definition is retrieved. If the definition is not in the
    * repository, then the search path is used to try to find the required
    * configuration definition file. If it is found and parsed correctly, the
    * definitions contained therein are added to the repository.
    */
   private ConfigDefinition getDefinition(String token, int version)
   {
      ConfigDefinition def = mDefinitionRepos.get(token, version);

      // If the definition is already in the repository, return it immediately
      if (def != null)
      {
         return def;
      }

      // Go looking for the definition
      /*
      File def_file = findDefinition(token);
      if (def_file != null)
      {
         try
         {
            // Get the definitions in the file
            ConfigDefinitionReader reader = new ConfigDefinitionReader(def_file);
            List defs = reader.readDefinition();
            // Make sure the file has the right definition version
            for (Iterator itr = defs.iterator(); itr.hasNext(); )
            {
               def = (ConfigDefinition)itr.next();
               if (def.getVersion() == version)
               {
                  // For now, only add in the definition that we explicitly
                  // want so that we don't stomp over the other definitions
                  // that are already in the repository
                  mDefinitionRepos.add(def);
                  return def;
               }
            }
         }
         catch (IOException ioe)
         {
            ioe.printStackTrace();
         }
         catch (ParseException pe)
         {
            pe.printStackTrace();
         }
      }
      */
      return null;
   }

   /**
    * Given the token for a configuration definition, this method attempts to
    * find a file named "token".jdef in this parser's search path.
    */
   /*private File findDefinition(String token)
   {
      String filename = token + ".jdef";
      for (Iterator itr = mSearchPath.iterator(); itr.hasNext(); )
      {
         // Get the next directory
         String dir = (String)itr.next();
         dir = EnvironmentService.expandEnvVars(dir);
         File file = new File(dir, filename);

         // If the file exists, return it
         if (file.exists())
         {
            return file;
         }
      }

      return null;
   }*/

   private static final String CONFIGURATION_ELEMENTS = "configuration_elements";
   private static final String NAME                   = "name";
   private static final String VERSION                = "version";

   private ConfigDefinitionRepository mDefinitionRepos;

   //private List mSearchPath;
}
