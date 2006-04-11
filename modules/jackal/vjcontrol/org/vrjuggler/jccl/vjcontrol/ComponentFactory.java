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

package org.vrjuggler.jccl.vjcontrol;

import java.beans.Beans;
import java.io.*;
import java.net.URL;
import java.net.URLClassLoader;
import java.util.HashMap;
import java.util.Vector;

import org.vrjuggler.jccl.config.*;

/** Factory object to create VjComponents and manage loading of .jars.
 *  Extensions to VjControl can be added in the form of external .jar
 *  archives, which can be loaded via the ComponentFactory.  Each
 *  extension .jar should contain at least one class that implements
 *  the VjComponent interface, or some subinterface of it.  A number of
 *  basic interfaces are declared in the VjGUI.interfaces package.
 *  <p>
 *  The .jar archive must have a contents.config file in its base directory.
 *  contents.config is a standard VR Juggler-style config file.  It should
 *  contain a VjComponent ConfigChunk for every class that implements
 *  VjComponent.  It should also contain ConfigChunks that define instances
 *  of those classes.  For example:
 *  <pre>
 *              vjc_genericinstance
 *                 Name "Connection"
 *                 ClassName { "VjGUI.ConnectionPane" }
 *                 Dependencies { "ControlUI Module" "Network Module" }
 *                 ParentComp { "ControlUI Module" }
 *                 end
 *  </pre>
 *  The vjc_genericinstance and vjc_component ConfigChunks are defined in
 *  juggler/GUI/VjFiles/vjcontrol.dsc.  Additional component chunkdescs can
 *  be included in a contents.desc file in the root or the extension .jar.
 *  <p>
 *   Once all the dependencies listed
 *  in the chunk are satisfied, the chunk is passed to the component listed
 *  as ParentComp, which is responsible for deciding what to do with it.
 *  The obvious choice is to immediately instantiate a copy of ClassName
 *  via ComponentFactory.createComponent(), but that may not always be
 *  the correct action.  For example, when the ControlUIModule sees a
 *  ConfigChunk for a ConfigChunkPanel component, it stores that information
 *  away and creates components on-demand.
 *  <p>
 *  In any event, after the new component is created, the parent will
 *  pass the instantiation ConfigChunk along via the configure() method.
 *  The new component should use this at least to set its component name.
 *  Note that a component may use some ConfigChunk other than
 *  vjc_genericinstance for instantiation, if it needs additional
 *  configuration information, so long as it contains a superset of the
 *  properties in vjc_genericinstance.  In general, VjControl interprets
 *  any ConfigChunks in contents.config that aren't vjc_component as
 *  requests to instantiate a new instance of a component.
 *  <p>
 *  ComponentFactory uses a single ClassLoader to load all extension .jars.
 *  A VjComponent (or any other class in an extension .jar) can directly
 *  instantiate any other class in any other extension .jar.  In contrast,
 *  classes defined in the base VjControl .jar file (loaded at application
 *  start) cannot directly instantiate any class in an extension .jar and
 *  would need to use ComponentFactory's createComponentMethod instead.
 *  <p>
 *  Objects whose classes were defined in separate .jar files should still
 *  be able to access each other (e.g. by getting references via
 *  Core.getRegisteredObject()).
 *
 *  @see VjComponent
 */
public class ComponentFactory
{
   /** Contains all objects which are listening for ComponentFactoryEvents. */
   public Vector component_factory_targets;

   public VjClassLoader loader;

   /** Constructor */
   public ComponentFactory ()
   {
      component_factory_targets = new Vector ();
      loader = new VjClassLoader(new URL[0]);
   }

   /** Loads a new extension .jar archive.
    *  The named .jar file is loaded.  Its contents.config is parsed,
    *  new VjComponent classes are registered, and instantiation
    *  ConfigChunks are placed in Core's pending configuration queue.
    *  @param _jar_file_name Name of a URL to load.  To load a file,
    *                        use a string like "file:path/name.jar"
    */
   public void registerJarURL (String jar_file_url)
   {
      // need to mangle filename;
      if( !jar_file_url.startsWith ("jar:") )
      {
         jar_file_url = "jar:" + jar_file_url;
      }

      if( !jar_file_url.endsWith ("!/") )
      {
         jar_file_url = jar_file_url + "!/";
      }

      try
      {
         System.out.println ("Loading jar file: " + jar_file_url);
         loader.addURL (new URL(jar_file_url));

         // need to get the description file out of the jar.
         // pity i don't think the manifest is versatile enough
         // for what i want

         try
         {
            // Get an optional chunkdescs file "contents.desc"
            URL descdb_url = new URL (jar_file_url + "contents.desc");
            ChunkDescDB descdb = new ChunkDescDB();
            descdb.build(descdb_url.openStream());
            Core.descdb.addAll (descdb);
         }
         catch( FileNotFoundException e1 )
         {
            // that's fine, we don't care if it ain't there.
         }
         catch( Exception e2 )
         {
            Core.consoleErrorMessage ("ComponentFactory",
                                      "Error reading contents.desc in "
                                      + jar_file_url);
            // e2.printStackTrace();
         }

         // Get the required contents file "contents.config"
         URL chunkdb_url = new URL (jar_file_url + "contents.config");

         ConfigChunkDB chunkdb = new ConfigChunkDB();
         chunkdb.build(chunkdb_url.openStream());
         /* now we need to analyze the read-in chunks and deal with 'em */

         for ( int i = 0; i < chunkdb.size(); ++i )
         {
            registerComponent (chunkdb.get(i));
         }

      }
      catch( Exception e )
      {
         System.out.println (e);
      }
   }

   /** Performs vjcontrol component load on the vjcontrol.jar file.
    *  Its contents.config is parsed,
    *  new VjComponent classes are registered, and instantiation
    *  ConfigChunks are placed in Core's pending configuration queue.
    */
   public void registerBaseJar ()
   {
      InputStream is;

      try
      {
         // Get an optional chunkdescs file "contents.desc"
         is = loader.getSystemResourceAsStream ("contents.desc");
         if( is != null )
         {
            ChunkDescDB descdb = new ChunkDescDB();
            descdb.build(is);
            Core.descdb.addAll (descdb);
         }
      }
      catch( Exception e1 )
      {
         Core.consoleErrorMessage ("ComponentFactory",
                                   "Error reading contents.desc in base .jar");
         // e1.printStackTrace();
      }

      try
      {
         // Get the required contents file "contents.config"
         is = loader.getSystemResourceAsStream ("contents.config");
         if( is != null )
         {
            ConfigChunkDB chunkdb = new ConfigChunkDB();
            chunkdb.build(is);

            /* now we need to analyze the read-in chunks */
            for ( int i = 0; i < chunkdb.size(); ++i )
            {
               registerComponent (chunkdb.get(i));
            }
         }
         else
         {
            Core.consoleErrorMessage ("ComponentFactory",
                                      "Base .jar had no 'contents.config'");
         }
      }
      catch( Exception e2 )
      {
         Core.consoleErrorMessage ("ComponentFactory",
                                   "Error reading contents.config in base .jar");
      }
   }

   /** Auxiliary function for registering JarFiles.
    *  @param ch A ConfigChunk from the .jar's contents.config.
    *  @param cl ClassLoader for the .jar.
    */
   protected void registerComponent (ConfigChunk ch)
   {
      try
      {
         String desctoken = ch.getDescToken();
         if ( desctoken.equals("vjc_Component") )
         {
            String class_name = ch.getName();
            System.out.println ("Registering component: " + class_name);
            //notifyComponentFactoryTargets (class_name, c);
         }
         else
         {
            // it's not a VjComponent chunk, so it's an actual
            // component instance chunk...
            Core.addPendingChunk (ch);
         }
      }
//         catch (ClassNotFoundException e) {
//             System.out.println (e);
//         }
      catch( Exception e )
      {
         System.out.println (e);
         e.printStackTrace();
      }
   }

   /** Create an instance of a VjComponent.
    *  @param class_name Fully-qualified name of a class that's been
    *                    registered with the ComponentFactory, e.g.
    *                    "VjGUI.components.coremodules.ConfigModule".
    *  @return A new instance of the named class, or null if an
    *          error occurred.
    */
   public VjComponent createComponent (String class_name)
   {
      try
      {
         //Class cl = loader.loadClass (class_name);
         //Object o = cl.newInstance();
         Object o = Beans.instantiate (loader, class_name);
         return(VjComponent)Beans.getInstanceOf (o, VjComponent.class);
      }
      catch( Exception e )
      {
         Core.consoleErrorMessage ("CreateComponent", "Failed: " + e.toString());
         e.printStackTrace();
         return null;
      }
   }

   /** Returns true if class_name is a VjComponent class we know about.
    *  @param class_name Fully-qualified name of a class that's been
    *                    registered with the ComponentFactory.
    *  @return True if class_name is listed in our registry of VjComponent
    *          classes, and false if it isn't.
    */
   public boolean isRegistered (String class_name)
   {
      //return (registered_classes.get(class_name) != null);
      try
      {
         loader.loadClass (class_name);
         return true;
      }
//          catch (java.lang.Error e1) {
//              return false;
//          }
      catch( Exception e )
      {
         return false;
      }
   }

   /** Checks if the named class supports an interface.
    *  @param class_name Fully-qualified name of a class that's been
    *                    registered with the ComponentFactory.
    *  @param inter The interface we're checking for.
    *  @return True if the class class_name implements the interface
    *          interface_name.
    *          False if the class doesn't implement it, or either the
    *          class or interface couldn't be found.
    */
   public boolean classSupportsInterface (String class_name,
                                          Class inter)
   {
      try
      {
         Class cl = loader.loadClass (class_name);
         return inter.isAssignableFrom (cl);
      }
      catch( Exception ex )
      {
         // any exception here is gonna be because we couldn't find the
         // class or interface, which implies failure.
         return false;
      }
   }


   //------------------ Component Factory Target Stuff ---------------------

   /** Adds a listener for ComponentFactoryEvents.
    *  The listener will receive an event notification whenever a new
    *  VjComponent class is registered.
    *  @deprecated
    */
   public void addComponentFactoryListener (ComponentFactoryListener l)
   {
      synchronized (component_factory_targets)
      {
         component_factory_targets.addElement (l);
      }
   }

   /** Removes a listener for ComponentFactoryEvents.
    *  @deprecated
    */
   public void removeComponentFactoryListener (ComponentFactoryListener l)
   {
      synchronized (component_factory_targets)
      {
         component_factory_targets.removeElement (l);
      }
   }

   /** Notifies listeners when a new VjComponent is registered. */
   protected void notifyComponentFactoryTargets (String _component_class_name,
                                                 Class _component_class)
   {
      Vector l;
      synchronized (component_factory_targets)
      {
         l = (Vector) component_factory_targets.clone();
      }

      ComponentFactoryEvent e =
      new ComponentFactoryEvent (this, _component_class_name,
                                 _component_class);

      for( int i = 0; i < l.size(); i++ )
      {
         ComponentFactoryListener lis =
         (ComponentFactoryListener)l.elementAt (i);
         lis.loadNewComponent (e);
      }
   }
}
