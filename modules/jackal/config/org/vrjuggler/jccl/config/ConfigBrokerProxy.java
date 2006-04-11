/*************** <auto-copyright.pl BEGIN do not edit this line> **************
 *
 * VR Juggler is (C) Copyright 1998-2005 by Iowa State University
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

import java.io.*;
import java.util.*;
import org.vrjuggler.tweek.beans.BeanRegistry;
import org.vrjuggler.tweek.beans.TweekBean;
import org.vrjuggler.jccl.config.event.ConfigListener;
import org.vrjuggler.jccl.config.event.ConfigBrokerListener;

/**
 * Imeplementation of the ConfigBroker interface that marshalls requests off to
 * the registered ConfigBroker Tweek service.
 *
 * @see ConfigBroker
 */
public class ConfigBrokerProxy
   implements ConfigBroker
{
   /**
    * Creates a new ConfigBroker proxy to the ConfigBroker Tweek service.
    *
    * @throws Exception    if the ConfigBroker service could not be found
    */
   public ConfigBrokerProxy()
      throws RuntimeException
   {
      TweekBean bean = BeanRegistry.instance().getBean("ConfigBroker");
      if (bean == null)
      {
         throw new RuntimeException("Could not find ConfigBroker service");
      }
      if (bean.getBean() instanceof ConfigBroker)
      {
         mBroker = (ConfigBroker)bean.getBean();
      }
      else
      {
         throw new RuntimeException("The bean registered as ConfigBroker is not a ConfigBroker implementation.");
      }
   }

   /**
    * Adds the given resources in the given data source to this mBroker.
    *
    * @param name          the unique name to assign to the resource
    * @param dataSource    the data source from which to retrieve the data
    */
   public void add(String name, DataSource dataSource)
   {
      mBroker.add(name, dataSource);
   }

   /**
    * Removes the resources in the given data source from this mBroker.
    *
    * @param name          the name of the data source to remove
    *
    * @return  the data source that was removed
    */
   public DataSource remove(String name)
   {
      return mBroker.remove(name);
   }

   /**
    * Gets the data source associated with the given name in this mBroker.
    *
    * @param name          the unique name assigned to the data source
    *
    * @return  the DataSource if one exists; null if no DataSource has that name
    */
   public DataSource get(String name)
   {
      return mBroker.get(name);
   }

   /**
    * Tests if the data source with the given name is being managed by this
    * mBroker.
    *
    * @param name    the name of the data source to check
    *
    * @return  true if the data source resource is in this broker;
    *          false otherwise
    */
   public boolean containsDataSource(String name)
   {
      return mBroker.containsDataSource(name);
   }

   /**
    * Adds the given configuration element to the current context. If the
    * context contains more than one resource, a dialog will prompt the user
    * for which resource they wish to add the element to.
    *
    * @param context    the context in which to add the element
    * @param elt        the configuration element to add
    *
    * @return  true if the addition was successful, false otherwise
    */
   public boolean add(ConfigContext context, ConfigElement elt)
   {
      return mBroker.add(context, elt);
   }

   /**
    * Removes the given configuration element from the current context. If the
    * element appears in more than one resource in the context, a dialog will
    * prompt the user for which resource they wish to remove the element from.
    * If the element does not appear in any resource in the context, this method
    * will return false.
    *
    * @param context    the context from which to remove the element
    * @param elt        the element to remove
    *
    * @return  true if the removal was successful, false if the user cancelled
    *          the removal or the element does not exist in any resource
    */
   public boolean remove(ConfigContext context, ConfigElement elt)
   {
      return mBroker.remove(context, elt);
   }

   /**
    * Gets a list of all the configuration elements within the given context.
    *
    * @param context    the context from which to retrieve elements
    *
    * @return  a list of the elements
    */
   public List getElements(ConfigContext context)
   {
      return mBroker.getElements(context);
   }
   
   /**
    * Gets a list of all the configuration elements within the given context
    * includeing embedded elements.
    *
    * @param context    the context from which to retrieve elements
    *
    * @return  a list of the configuration elements
    */
   public List getElementsIncludingEmbedded(ConfigContext context)
   {
      return mBroker.getElementsIncludingEmbedded(context);
   }

   /**
    * Gets a list of all the configuration elements within the given
    * resource.
    *
    * @param resource   the name of the resource in which to get elements
    *
    * @return  a list of the elements in the resource if it has any
    */
   public List getElementsIn(String resource)
   {
      return mBroker.getElementsIn(resource);
   }

   /**
    * Gets a list of the names all the resources currently being managed by this
    * mBroker.
    *
    * @return  a list of the resource names
    */
   public List getResourceNames()
   {
      return mBroker.getResourceNames();
   }

   /**
    * Gets the repository in which configuration definitions are stored.
    *
    * @return  the repository of config definitions
    */
   public ConfigDefinitionRepository getRepository()
   {
      return mBroker.getRepository();
   }

   /**
    * Adds the given listener to receive config events from this mBroker.
    */
   public void addConfigListener(ConfigListener listener)
   {
      mBroker.addConfigListener(listener);
   }

   /**
    * Removes the given listener from receiving config events from this mBroker.
    */
   public void removeConfigListener(ConfigListener listener)
   {
      mBroker.removeConfigListener(listener);
   }

   /**
    * Adds the given listener to receive config broker events from this mBroker.
    */
   public void addConfigBrokerListener(ConfigBrokerListener listener)
   {
      mBroker.addConfigBrokerListener(listener);
   }

   /**
    * Removes the given resource listener from receiving confi broker events
    * from this mBroker.
    */
   public void removeConfigBrokerListener(ConfigBrokerListener listener)
   {
      mBroker.removeConfigBrokerListener(listener);
   }
   
   public void saveDefinition(ConfigDefinition def)
      throws IOException
           , org.vrjuggler.jccl.config.io.ParseException
   {
      mBroker.saveDefinition(def);
   }

   /**
    * The config broker instance this object is proxy to.
    */
   private ConfigBroker mBroker;
}
