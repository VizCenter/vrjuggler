/*************** <auto-copyright.pl BEGIN do not edit this line> **************
 *
 * VR Juggler is (C) Copyright 1998-2010 by Iowa State University
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
 * Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 *
 *************** <auto-copyright.pl END do not edit this line> ***************/

/* Generated by Together */

package org.vrjuggler.tweek.beans;

import org.vrjuggler.tweek.services.EnvironmentService;
import org.vrjuggler.tweek.services.EnvironmentServiceProxy;

import org.jdom.Document;
import org.jdom.Element;
import org.jdom.JDOMException;
import org.jdom.input.SAXBuilder;

import java.io.File;
import java.util.ArrayList;
import java.util.Iterator;
import java.util.List;
import java.util.StringTokenizer;

/**
 * The sole purpose of this class is to find XML files in a particular path list
 * and examine those files for Tweek beans. Only XML files that contain a
 * "beanlist" document qualify as valid, loadable XML files.
 */
public class XMLBeanFinder
{
   public XMLBeanFinder()
   {
      this(false);
   }

   public XMLBeanFinder(boolean validate)
   {
      mValidate = validate;
   }

   /**
    * Searches the given path for XML files describing Tweek beans. When such
    * files are found, they are parsed for beans. The list of found beans is
    * returned.
    *
    * @param path    the path in which to search for beans. multiple paths may
    *                be concatenated together using the filesytem dependent path
    *                separator character (i.e. Unix=':')
    *
    * @return  the list of found beans
    *
    * @throws BeanPathException  if one of the paths is invalid
    */
   public List find (String path)
      throws BeanPathException
   {
      List beans = new ArrayList();

      // Break up the path into individual files and directories
      List files = new ArrayList();
      StringTokenizer tokenizer = new StringTokenizer( path, File.pathSeparator );
      while ( tokenizer.hasMoreTokens() )
      {
         String token = tokenizer.nextToken();

         // if the current path is a directory, search it for xml files
         File file = new File( token );
         if ( file.isDirectory() )
         {
            FileFinder fileFinder = new FileFinder( "xml", path );
            for ( int i = 0; i < fileFinder.getFileCount(); i++ )
            {
               files.add( fileFinder.getFile( i ) );
            }
         }
         // otherwise, just add the file directly
         else
         {
            files.add(file);
         }
      }

      SAXBuilder builder = new SAXBuilder(mValidate);
      builder.setErrorHandler(new BestSAXChecker());
      File xml_file;

      // Check each XML file for beans
      for ( Iterator itr = files.iterator(); itr.hasNext(); )
      {
         xml_file = (File) itr.next();

         if ( xml_file.canRead() )
         {
            try
            {
               Document document = builder.build(xml_file);
               Element root = document.getRootElement();

               // If the root element is a <beantree>, proceed.  Otherwise, this
               // XML document is not one about which we care.
               if ( root.getName().equals( "beanlist" ) )
               {
                  beans.addAll( manageChildren( root.getChildren() ) );
               }
            }
            catch (Exception e)
            {
               e.printStackTrace();
            }
         }
         else
         {
            System.err.println("WARNING: Could not read '" +
                               xml_file.getAbsolutePath() + "'");
         }
      }
      return beans;
   }

   /**
    * Iterates over the given list of XML Elements (children of some parent
    * Element) and processes each.
    *
    * @return  the list of found beans
    */
   protected List manageChildren (List children)
   {
      List beans = new ArrayList();
      Iterator i = children.iterator();

      while ( i.hasNext() )
      {
         Element child = (Element) i.next();
         String name   = child.getName();

         // The current child is a <guipanel></guipanel> block.
         if ( name.equals("guipanel") )
         {
            beans.add( handleGuiPanel(child) );
         }
         // The current child is a <service></service> block.
         else if ( name.equals("service") )
         {
            beans.add( handleService(child) );
         }
         // The current child is a <viewer></viewer> block.
         else if ( name.equals("viewer") )
         {
            beans.add( handleViewer(child) );
         }
         // The current child is a <generic></generic> block.
         else if ( name.equals("generic") )
         {
            beans.add( handleGeneric(child) );
         }
         else
         {
            System.err.println("WARNING: Unknown child type <" + name + ">");
         }
      }
      return beans;
   }

   /**
    * Processes a &lt;guipanel&gt;&lt;/guipanel&gt; block of the beanlist document.
    *
    * @return  the bean described in this block
    */
   protected TweekBean handleGuiPanel (Element bean)
   {
      BeanAttributes attrs = BeanAttributes.parseXMLElement( bean );

      EnvironmentService env = new EnvironmentServiceProxy();

      // The rest are specific to <guipanel>.
      String icon_path = null;
      String tool_tip  = null;
      List tree_path = new ArrayList();

      List children = bean.getChildren();
      Iterator i    = children.iterator();

      while ( i.hasNext() )
      {
         Element e = (Element) i.next();

         // <tree /> element.
         if ( e.getName().equals("tree") )
         {
            tree_path.add(e.getAttribute("path").getValue());
         }
         // <icon /> element.
         else if ( e.getName().equals("icon") )
         {
            icon_path = env.expandEnvVars(e.getAttribute("source").getValue());
            tool_tip  = e.getAttribute("tooltip").getValue();
         }
      }

      return new PanelBean( attrs, icon_path, tool_tip, tree_path );
   }

   /**
    * Processes a &lt;service&gt;&lt;/service&gt; block of the beanlist document.
    *
    * @return  the bean described in this block
    */
   protected TweekBean handleService (Element service)
   {
      BeanAttributes attrs = BeanAttributes.parseXMLElement( service );
      return new ServiceBean(attrs);
   }

   /**
    * Processes a &lt;viewer&gt;&lt;/viewer&gt; block of the beanlist document.
    *
    * @return  the bean described in this block
    */
   protected TweekBean handleViewer (Element viewer)
   {
      BeanAttributes attrs = BeanAttributes.parseXMLElement( viewer );
      return new ViewerBean(attrs);
   }

   /**
    * Processes a &lt;generic&gt;&lt;/generic&gt; block of the beanlist document.
    *
    * @return  the bean described in this block
    */
   protected TweekBean handleGeneric (Element gen_bean)
   {
      BeanAttributes attrs = BeanAttributes.parseXMLElement( gen_bean );
      return new GenericBean(attrs);
   }

   private boolean mValidate = false;
}
