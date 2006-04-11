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
package org.vrjuggler.vrjconfig.wizard.newdevice;

import java.awt.BorderLayout;
import java.awt.Component;
import java.awt.event.*;
import java.io.IOException;
import java.util.Enumeration;
import java.util.Iterator;
import java.util.List;
import java.util.Map;
import java.util.TreeSet;
import javax.swing.*;
import javax.swing.tree.*;

import org.vrjuggler.jccl.config.*;
import org.vrjuggler.tweek.wizard.*;

public class SelectDefinitionPanel extends JPanel
{
   private JPanel mDirectionsPanel = new JPanel();
   private JLabel mTitleLbl = new JLabel();
   private JLabel mDirectionsLbl = new JLabel();
   private JPanel mTopPanel = new JPanel();
   private JLabel mDeviceIcon = new JLabel();
   private JList mDefinitionList = new JList();
   private JScrollPane mDefinitionListScrollPane = new JScrollPane();
   private Map mWhiteBoard;

   public SelectDefinitionPanel()
   {
      try
      {
         createListModel();
         jbInit();
         
         DefinitionListRenderer renderer2 = new DefinitionListRenderer();
         mDefinitionList.setCellRenderer(renderer2);
      }
      catch(Exception e)
      {
         e.printStackTrace();
      }

      try
      {
         ClassLoader loader = getClass().getClassLoader();
         mDeviceIcon.setIcon(new ImageIcon(loader.getResource("org/vrjuggler/vrjconfig/images/devices64.png")));
      }
      catch(NullPointerException exp)
      {
         mDeviceIcon.setText("");
      }
   }

   public class DefinitionInfo
   {
      private ConfigDefinition mDef;

      public DefinitionInfo(ConfigDefinition def)
      {
         mDef = def;
      }

      public String toString()
      {
         return(mDef.getName());
      }
   }

   public ConfigDefinition getSelectedDefinition()
   {
      return(((DefinitionInfo)mDefinitionList.getSelectedValue()).mDef);
   }

   public void createListModel()
   {
      DefaultListModel list_model = new DefaultListModel();

      ConfigDefinitionRepository repos = getBroker().getRepository();
      
      TreeSet all_defs = new TreeSet();
      
      List pos_defs = repos.getSubDefinitions("positional_device");
      List dig_defs = repos.getSubDefinitions("digital_device");
      List ana_defs = repos.getSubDefinitions("analog_device");

      all_defs.addAll(pos_defs);
      all_defs.addAll(dig_defs);
      all_defs.addAll(ana_defs);
      
      for(Iterator itr = all_defs.iterator() ; itr.hasNext() ; )
      {
         ConfigDefinition def = repos.get((String)itr.next());
         list_model.addElement(new DefinitionInfo(def));
      }
      
      mDefinitionList.setModel(list_model);
   }

   private void jbInit() throws Exception
   {
      mTitleLbl.setFont(new java.awt.Font("Serif", 1, 20));
      mTitleLbl.setHorizontalAlignment(SwingConstants.LEFT);
      mTitleLbl.setText("Add New Input Device Wizard");
      mDirectionsLbl.setText("Select the type of input device you want to add.");

      mDirectionsPanel.add(mTitleLbl, null);
      mDirectionsPanel.add(mDirectionsLbl, null);

      mTopPanel.setLayout(new BorderLayout());
      mTopPanel.setBorder(BorderFactory.createEtchedBorder());
      mTopPanel.add(mDirectionsPanel, BorderLayout.CENTER);
      mTopPanel.add(mDeviceIcon, BorderLayout.EAST);

      mDefinitionListScrollPane.getViewport().add(mDefinitionList, null);
      mDefinitionListScrollPane.setBorder(BorderFactory.createLoweredBevelBorder());
      mDefinitionList.setSelectionMode(ListSelectionModel.SINGLE_SELECTION);
 
      setLayout(new BorderLayout());
      add(mTopPanel, BorderLayout.NORTH);
      add(mDefinitionListScrollPane, BorderLayout.CENTER);
   }

   public void init(Map whiteboard)
   {
      mWhiteBoard = whiteboard;
   }

   /** Reference to the ConfigBroker used in this object. */
   private ConfigBroker mBroker = null;
   
   /**
    * Gets a handle to the configuration broker.
    */
   private ConfigBroker getBroker()
   {
      if (mBroker == null)
      {
         synchronized (this)
         {
            if (mBroker == null)
            {
               mBroker = new ConfigBrokerProxy();
            }
         }
      }
      return mBroker;
   }

   /**
    * Custom ListCellRenderer for proxies.
    */
   public class DefinitionListRenderer extends DefaultListCellRenderer
   {
      /** Icon used for position nodes. */
      private Icon mPositionalIcon = null;

      /** Icon used for position nodes. */
      private Icon mDigitalIcon = null;

      /** Icon used for position nodes. */
      private Icon mAnalogIcon = null;

      /**
       * Constructor.
       */
      public DefinitionListRenderer()
      {
         ClassLoader loader = getClass().getClassLoader();
        
         // Load the icons.
         mPositionalIcon = new ImageIcon(loader.getResource("org/vrjuggler/vrjconfig/wizard/newdevice/images/positional_devices16.png"));
         mDigitalIcon = new ImageIcon(loader.getResource("org/vrjuggler/vrjconfig/wizard/newdevice/images/digital_devices16.png"));
         mAnalogIcon = new ImageIcon(loader.getResource("org/vrjuggler/vrjconfig/wizard/newdevice/images/analog_devices16.png"));
      }
      
      /**
       * Creates new TreeNodeRenderer for given value.
       *
       * @param list     JList which is asking for renderer.
       * @param value    Value of node being renderer.
       * @param selected Is the value selected.
       * @param hasFocus Does the value have focus.
       */
      public Component getListCellRendererComponent(JList list, Object value,
                                                    int index, boolean selected,
                                                    boolean hasFocus)
      {
         //DefaultMutableTreeNode node = (DefaultMutableTreeNode)value;
       
         // NOTE: DefaultTreeCellRenderer extends JLabel and returns 
         // itself, if we call the following method to set everything up
         // we can then set the Icon to whatever we want and return this.
         // Get the default renderer for the passed value because we know that the UserObject is
         // either a ProxyType or a DeviceUnit which both implement the toString method.
         super.getListCellRendererComponent(list, value, index, selected, hasFocus);

         /*
         DefaultMutableTreeNode parent = (DefaultMutableTreeNode)node.getParent();
         
         // Use the appropriate Icon.
         if(null != parent && parent.getUserObject() instanceof String)
         {
            String label = (String)parent.getUserObject();

            // Set the correct Icon for this node in the tree.
            if(label.equals("Positional Device"))
            {
               setIcon(mPositionalIcon);
            }
            else if(label.equals("Digital Device"))
            {
               setIcon(mDigitalIcon);
            }
            else if(label.equals("Analog Device"))
            {
               setIcon(mAnalogIcon);
            }
         }
         */
         return this;
      }
   }

   
}
