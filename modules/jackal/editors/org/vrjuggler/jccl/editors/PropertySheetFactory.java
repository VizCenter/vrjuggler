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
package org.vrjuggler.jccl.editors;

import java.awt.event.ActionEvent;
import java.awt.event.ActionListener;
import java.awt.Component;
import java.awt.Dimension;
import java.awt.Insets;
//import java.awt.Graphics;
import java.awt.Color;
import java.util.ArrayList;
import java.util.Iterator;
import java.util.List;
import javax.swing.*;
import java.beans.*;
import info.clearthought.layout.*;
import org.vrjuggler.jccl.editors.*;
import org.vrjuggler.jccl.config.*;
//import com.sun.java.swing.plaf.windows.WindowsTreeUI;

public class PropertySheetFactory extends PropertyComponent
{
   public PropertySheetFactory()
   {
      // Register the editors.
      PropertyEditorManager.registerEditor(Boolean.class, BooleanEditor.class);
      PropertyEditorManager.registerEditor(String.class, StringEditor.class);
      PropertyEditorManager.registerEditor(Integer.class, IntegerEditor.class);
      PropertyEditorManager.registerEditor(Float.class, FloatEditor.class);
   }
   
   /**
    * Gets the singleton instance of this class. This implementation is thread
    * safe.
    */
   public static PropertySheetFactory instance()
   {
      synchronized (PropertySheetFactory.class)
      {
         if ( mInstance == null )
         {
            mInstance = new PropertySheetFactory();
         }
      }
      return mInstance;
   }

   /**
    * The singleton instance of this class.
    *
    * @link
    * @shapeType PatternLink
    * @pattern Singleton
    * @supplierRole Singleton factory
    */
   /*# private PropertySheetFactory _propertySheetFactory; */
   private static PropertySheetFactory mInstance = null;

   public PropertySheet makeSheet(ConfigContext ctx, ConfigElement elm, Color color)
   {
      PropertySheet sheet = new PropertySheet();
      
      sheet.setConfigElement(elm);
      sheet.setColor(color);
      sheet.init();
      
      int row = 1;

      // Loop over all properties.
      ConfigDefinition def = elm.getDefinition();
      java.util.List prop_defs = def.getPropertyDefinitions();
      for(Iterator prop_def_itr = prop_defs.iterator() ; prop_def_itr.hasNext() ; )
      {
         PropertyDefinition prop_def = (PropertyDefinition)prop_def_itr.next();
         System.out.println("Property: " + prop_def.getToken());
         
         if (prop_def.isVariable() || (elm.getPropertyValueCount(prop_def.getToken()) > 1))
         {
            System.out.println("Variable Property List.");
            
            addVarList(sheet, ctx, elm, prop_def, row);
            ++row;
         }
         else
         {
            Object value = elm.getProperty(prop_def.getToken(), 0);
            
            if(value.getClass() == ConfigElement.class)
            {
               // Embedded Element.
               addEmbeddedElement(sheet, ctx, elm, value, prop_def, row);
               ++row;
            }
            else
            {
               // Normal Property.
               String label = prop_def.getPropertyValueDefinition(0).getLabel();
               addNormalEditor(sheet, ctx, elm, value, prop_def, label, row, 0);
               ++row;
            }
         }
      }
      return sheet;
   }
   public PropertySheet makeVarSheet(ConfigElement elm, ConfigContext ctx, PropertyDefinition prop_def, Color color)
   {
      PropertySheet sheet = new PropertySheet();

      System.out.println("Variable Property List.");
      List props = elm.getPropertyValues(prop_def.getToken());
      
      sheet.setConfigElement(elm);
      sheet.setColor(color);
      
      sheet.init();
      
      int row = 1;
      int list_number = 0;
      
      // Insert Add Icon
      ClassLoader loader = getClass().getClassLoader();
      Icon add_icon = new ImageIcon(loader.getResource("org/vrjuggler/jccl/editors/images/New16.gif"));
      Icon remove_icon = new ImageIcon(loader.getResource("org/vrjuggler/jccl/editors/images/Delete16.gif"));

      JButton add_button = new JButton();   
      
      // If we have a variable list then create the buttons to add new values.
      if(prop_def.isVariable())
      {
         add_button.setIcon(add_icon);
         add_button.setMargin(new Insets(0,0,0,0));
         add_button.setBorderPainted(false);
         add_button.setFocusPainted(false);
         add_button.setContentAreaFilled(false);

         final ConfigElement temp_elm = elm;
         final PropertyDefinition temp_prop_def = prop_def;
         
         add_button.addActionListener(new ActionListener()
         {
            public void actionPerformed(ActionEvent evt)
            {
               if(ConfigElement.class == temp_prop_def.getType())
               {
                  addNewEmbeddedElement(temp_elm, temp_prop_def);
               }
               else
               {
                  addNewNormalEditor(temp_elm, temp_prop_def);
               }
            }
         });
         
         TableLayoutConstraints c3 = new TableLayoutConstraints(0, row, 2, row, TableLayout.CENTER, TableLayout.TOP);
         sheet.add(add_button, c3);
         ++row;
      }
      
      for(Iterator itr = props.iterator() ; itr.hasNext() ; )
      {
         Object value = itr.next();

         if(value.getClass() == ConfigElement.class)
         {
            // Embedded Element.
            addEmbeddedElement(sheet, ctx, elm, value, prop_def, row);
         }
         else // List of normal values.
         {
            String label = null;
            // If it is variable then there will only be one label for the
            // property. Otherwise there will be one for each value.
            if(prop_def.isVariable())
            {
               label = prop_def.getPropertyValueDefinition(0).getLabel();
            }
            else
            {
               label = prop_def.getPropertyValueDefinition(list_number).getLabel();
            }
            
            addNormalEditor(sheet, ctx, elm, value, prop_def, label, row, list_number);
            ++list_number;
         }
         ++row;
      }
      return sheet;
   }

   /**
    * Adds a delete icon next to the given property. When the Icon is clicked it
    * will remove the property from the ConfigElement.
    */
   private void addDeleteButton(PropertySheet sheet, ConfigElement elm, PropertyDefinition prop_def, Object value, int row)
   {
      ClassLoader loader = getClass().getClassLoader();
      Icon remove_icon = new ImageIcon(loader.getResource("org/vrjuggler/jccl/editors/images/Delete16.gif"));
      
      JButton remove_button = new JButton();
      remove_button.setIcon(remove_icon);
      remove_button.setMargin(new Insets(0,0,0,0));
      remove_button.setBorderPainted(false);
      remove_button.setFocusPainted(false);
      remove_button.setContentAreaFilled(false);
      
      // Verify that the property is variable.
      if(prop_def.isVariable())
      {
         remove_button.setEnabled(true);
        
         final Object temp_value = value;
         final String temp_string = prop_def.getToken();
         final ConfigElement temp_elm = elm;
         
         remove_button.addActionListener(new ActionListener()
         {
            public void actionPerformed(ActionEvent evt)
            {
               PropertyComponent temp = (PropertyComponent)((Component)evt.getSource()).getParent();
               temp_elm.removeProperty(temp_string, temp_value);
               
               if(temp.getLayout() instanceof TableLayout)
               {
                  TableLayout tl = (TableLayout)temp.getLayout();
                  // Get the row that this panel is in.
                  TableLayoutConstraints tlc = tl.getConstraints((Component)evt.getSource());
                  int row = tlc.row1;
                  temp.remove((Component)evt.getSource()); 
                  tl.deleteRow(row);
               }

               temp.revalidate();
               temp.repaint();
            }
         });
      }
      else
      {
         remove_button.setEnabled(false);
      }
      TableLayoutConstraints c4 = new TableLayoutConstraints(2, row, 2, row, TableLayout.LEFT, TableLayout.TOP);
      sheet.add(remove_button, c4);
   }
   
   private void addVarList(PropertySheet sheet, ConfigContext ctx, ConfigElement elm, PropertyDefinition prop_def, int row)
   {
      // Use the same color for the list panel.
      VarListPanel editor_list = new VarListPanel(ctx, elm, prop_def, sheet.getColor());
      
      ((TableLayout)sheet.getLayout()).insertRow(row, TableLayout.PREFERRED);
      
      TableLayoutConstraints c = new TableLayoutConstraints(0, row, 1, row, TableLayout.FULL, TableLayout.FULL);
      sheet.add(editor_list, c);

      revalidate();
      repaint();
   }
   
   public void addNormalEditor(PropertySheet sheet, ConfigContext ctx, ConfigElement elm,
                               Object value, PropertyDefinition prop_def, String label,
                               int row, int list_num)
   {
      PropertyEditorPanel editor = new PropertyEditorPanel(ctx, value, prop_def, elm, list_num, sheet.getColor());
      
      ((TableLayout)sheet.getLayout()).insertRow(row, TableLayout.PREFERRED);
   
      // Add both columns to this row.
      TableLayoutConstraints c = new TableLayoutConstraints(1, row, 1, row, TableLayout.FULL, TableLayout.FULL);
      sheet.add(editor, c);
      sheet.add(new JLabel(label), "0," + Integer.toString(row) + ",F,F");
      
      addDeleteButton(sheet, elm, prop_def, value, row);

      revalidate();
      repaint();
   }
   
   public void addEmbeddedElement(PropertySheet sheet, ConfigContext ctx, ConfigElement elm,
                                  Object value, PropertyDefinition prop_def, int row)
   {
      // Embedded Element
      // Adding a List
      EmbeddedElementPanel editor_list = new EmbeddedElementPanel(ctx, (ConfigElement)value, sheet.getColor());
      
      ((TableLayout)sheet.getLayout()).insertRow(row, TableLayout.PREFERRED);
      
      TableLayoutConstraints c = new TableLayoutConstraints(0, row, 1, row, TableLayout.FULL, TableLayout.FULL);
      sheet.add(editor_list, c);
      
      addDeleteButton(sheet, elm, prop_def, value, row);

      revalidate();
      repaint();
   }
   
   private void addNewNormalEditor(ConfigElement elm, PropertyDefinition prop_def)
   {
      // We know that we want the default value for the first
      // PropertyValueDefinition since this is a variable list.
      PropertyValueDefinition pvd = prop_def.getPropertyValueDefinition(0);
      Object default_value = null;
      
      // NOTE: This fixed a rather large bug that caused ConfigElementPointerEditors
      //       that were added at run time to all edit the same ConfigElementPointer.
      //       By default the PropertyValueDefinition returns a default value that
      //       is always the same, bad idea.
      if (prop_def.getType() == ConfigElementPointer.class)
      {
         default_value = new ConfigElementPointer("");
      }
      else
      {
         default_value = pvd.getDefaultValue();
      }
      
      elm.addProperty(prop_def.getToken(), default_value);
      
      //XXX: This should be detected through a listener in the Property sheet.
      // We select 2 here because we want to add it to the top of the list.
      //addNormalEditor(default_value, prop_def, pvd.getLabel(), 2,
      //      elm.getPropertyValueCount(prop_def.getToken()) - 1);
   }
   
   private void addNewEmbeddedElement(ConfigElement elm, PropertyDefinition prop_def)
   {
      Object new_value = null;
      
      // Pick which type of embedded element to add
      List string_allowed_types = prop_def.getAllowedAndDerivedTypes();
      List allowed_types = new ArrayList();
      
      ConfigBroker broker = new ConfigBrokerProxy();
      ConfigDefinitionRepository def_repos = broker.getRepository();
      
      for(Iterator itr = string_allowed_types.iterator() ; itr.hasNext() ; )
      {
         String type = (String)itr.next();
         ConfigDefinition def = def_repos.get(type);
         if(null != def)
         {
            allowed_types.add(def);
         }
      }
      
      // Ask the user to choose a base ConfigDefinition.
      ConfigDefinitionChooser chooser = new ConfigDefinitionChooser();
      chooser.setDefinitions(allowed_types);
      int result = chooser.showDialog(this);

      // If the user did not cancel their choice, make a new ConfigElement for
      // the chosen ConfigDefinition.
      if (result == ConfigDefinitionChooser.APPROVE_OPTION)
      {
         // Get a list of all known ConfigDefinitions
         java.util.List defs = def_repos.getAllLatest();
      
         ConfigElementFactory temp_factory = new ConfigElementFactory(defs);
         
         // TODO: Compute a unique name
         new_value = temp_factory.create("CHANGEME", chooser.getSelectedDefinition());
         
         elm.addProperty(prop_def.getToken(), new_value);
         //XXX: This should be detected through a listener in the Property sheet.
         // We select 2 here because we want to add it to the top of the list.
         //addEmbeddedElement(new_value, prop_def, 2);
      }
   }
}
