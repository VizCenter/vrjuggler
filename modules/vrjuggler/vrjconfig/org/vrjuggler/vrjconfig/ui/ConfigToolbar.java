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
package org.vrjuggler.vrjconfig.ui;

import java.awt.*;
import java.awt.event.*;
import java.io.*;
import java.util.ArrayList;
import java.util.Iterator;
import java.util.Stack;
import javax.swing.*;
import org.vrjuggler.tweek.services.EnvironmentService;
import org.vrjuggler.tweek.services.EnvironmentServiceProxy;
import org.vrjuggler.tweek.services.GlobalPreferencesServiceProxy;
import org.vrjuggler.tweek.services.GlobalPreferencesService;

import org.vrjuggler.jccl.config.*;
import org.vrjuggler.jccl.config.event.ConfigContextEvent;
import org.vrjuggler.jccl.config.event.ConfigContextListener;
import org.vrjuggler.jccl.rtrc.*;
import org.vrjuggler.vrjconfig.PopupButton;
import org.vrjuggler.vrjconfig.VrjConfigConstants;

/**
 * A specialized toolbar that pays attention to the ConfigManager.
 */
public class ConfigToolbar
   extends JComponent
   implements VrjConfigConstants
{
   public ConfigToolbar()
   {
      try
      {
         jbInit();
      }
      catch (Exception e)
      {
         e.printStackTrace();
      }

      // Try to get icons for the toolbar buttons
      try
      {
         ClassLoader loader = getClass().getClassLoader();
         newBtn.setIcon(new ImageIcon(loader.getResource("org/vrjuggler/vrjconfig/images/newchunk.gif")));
         openBtn.setIcon(new ImageIcon(loader.getResource("org/vrjuggler/vrjconfig/images/open.gif")));
         saveAllBtn.setIcon(new ImageIcon(loader.getResource("org/vrjuggler/vrjconfig/images/saveall.gif")));
         copyBtn.setIcon(new ImageIcon(loader.getResource("org/vrjuggler/vrjconfig/images/Copy16.gif")));
         pasteBtn.setIcon(new ImageIcon(loader.getResource("org/vrjuggler/vrjconfig/images/Paste16.gif")));

         RTRCBtn.setIcon(new ImageIcon(loader.getResource("org/vrjuggler/vrjconfig/images/vrjuggler.gif")));
      }
      catch (Exception e)
      {
         // Ack! No icons. Use text labels instead
         newBtn.setText("New");
         openBtn.setText("Open");
         saveAllBtn.setText("Save All");
         copyBtn.setText("Copy");
         pasteBtn.setText("Paste");
         RTRCBtn.setText("RTRC");
      }

      try
      {
         GlobalPreferencesService prefs = new GlobalPreferencesServiceProxy();

         // Using the global user preferences from Tweek, set the start
         // directory for fileChooser.
         File f = new File(prefs.getChooserStartDir());
         fileChooser.setCurrentDirectory(f);
      }
      catch(Exception ex)
      {
         System.err.println("ConfigToolbar(): WARNING: Failed to set file chooser start directory: " +
                            ex.getMessage());
      }
   }

   public void addToToolbar(Component comp)
   {
      toolbar.add(comp);
   }

   public void setConfigContext(ConfigContext ctx)
   {
      this.context = ctx;
   }

   public ConfigContext getConfigContext()
   {
      return context;
   }

   public File getCurrentDirectory()
   {
      return fileChooser.getCurrentDirectory();
   }

   public void addActionListener(ActionListener listener)
   {
      listenerList.add(ActionListener.class, listener);
   }

   public void removeActionListener(ActionListener listener)
   {
      listenerList.remove(ActionListener.class, listener);
   }

   protected void fireAction(String command)
   {
      ActionEvent evt = null;
      Object[] listeners = listenerList.getListenerList();
      for (int i=listeners.length-2; i>=0; i-=2)
      {
         if (listeners[i] == ActionListener.class)
         {
            if (evt == null)
            {
               evt = new ActionEvent(this, 0, command);
            }
            ((ActionListener)listeners[i+1]).actionPerformed(evt);
         }
      }
   }

   /**
    * Creates the default configuration context used as the base of all
    * configurations.
    */
   private ConfigContext createDefaultConfigContext()
   {
      return new ConfigContext();
   }

   private boolean openInContext(String filename, ConfigContext ctx)
   {
      try
      {
         String file = expandEnvVars(filename);
         FileDataSource data_source = FileDataSource.open(file, getBroker().getRepository());
         getBroker().add(file, data_source);
         ctx.add(file);
      }
      catch (IOException ioe)
      {
         ioe.printStackTrace();
         return false;
      }
      return true;
   }

   /**
    * Programmatically does a new action in a new ConfigContext.
    */
   public boolean doNew()
   {
      NewConfigDialog new_dlg = new NewConfigDialog(fileChooser.getCurrentDirectory());
      int option = new_dlg.showDialog(getParentFrame());
      if (option == NewConfigDialog.APPROVE_OPTION)
      {
         // Open all the included files first
         ConfigContext ctx = new ConfigContext();
         // TODO: Add support for jconf includes.
         /*
         for (Iterator itr = new_dlg.getIncludes().iterator(); itr.hasNext(); )
         {
            if (! openInContext((String)itr.next(), ctx))
            {
               return false;
            }
         }
         */

         // Create the new config file
         try
         {
            File new_file = new File(new_dlg.getDirectory(),
                                     new_dlg.getFileName());
            String new_filename = new_file.getAbsolutePath();
            FileDataSource data_source = FileDataSource.create(new_filename, getBroker().getRepository());
            getBroker().add(new_filename, data_source);
            ctx.add(new_filename);
         }
         catch (IOException ioe)
         {
            ioe.printStackTrace();
            return false;
         }

         setConfigContext(ctx);
         fireAction("New");
         return true;
      }
      return false;
   }

   /**
    * Programmatically does a new action in the given ConfigContext.
    */
   public boolean doNew(ConfigContext ctx)
   {
      // Create a new data source and add it to the broker
      try
      {
         // Get the new filename from the user
         String filename = askUserForNewConfigFile();
         if (filename == null)
         {
            // User cancelled
            return false;
         }

         // Add in the new data source
         FileDataSource new_data_source = FileDataSource.create(filename, getBroker().getRepository());
         getBroker().add(filename, new_data_source);
         ctx.add(filename);
         setConfigContext(ctx);

         return true;
      }
      catch (IOException ioe)
      {
         ioe.printStackTrace();
      }

      return false;
   }

   /**
    * Programmatically does an open action into a new context.
    */
   public boolean doOpen()
   {
      boolean result = doOpen(createDefaultConfigContext());
      if (result)
      {
         fireAction("Open");
      }
      return result;
   }

   public boolean doRTRC()
   {
      boolean result = doRTRC(createDefaultConfigContext());
      if (result)
      {
         //XXX: "RTRC"
         fireAction("Open");
      }


      return result;
   }


   /**
    * Programmatically executes an open action into the given context.
    */
   public boolean doOpen(ConfigContext ctx)
   {
      int result = fileChooser.showOpenDialog(getParentFrame());
      if (result == JFileChooser.APPROVE_OPTION)
      {
         try
         {
            // Make sure the selected file actually exists
            File file = fileChooser.getSelectedFile();
            if (! file.exists())
            {
               JOptionPane.showMessageDialog(getParentFrame(),
                                             "You must open an existing file.",
                                             "Error",
                                             JOptionPane.ERROR_MESSAGE);
               return false;
            }

            ConfigBroker broker = new ConfigBrokerProxy();

            // We want to automatically follow include directives. Keep track of
            // all the URLs on a stack and read them one at a time in the order
            // that we come across them
            Stack urls = new Stack();
            urls.push(file);
            while (! urls.isEmpty())
            {
               // Expand env vars in the URL
               File res_file = (File)urls.pop();
               String res_name = expandEnvVars(res_file.getAbsolutePath());
               System.out.println("Opening included resource: "+res_name);

               FileDataSource data_source = FileDataSource.open(res_name, getBroker().getRepository());
               broker.add(res_name, data_source);
               ctx.add(res_name);

               // Look through the elements in the newly loaded file and see if
               // any of them are include directives
               java.util.List includes = data_source.getIncludes();
               for (Iterator itr = includes.iterator(); itr.hasNext(); )
               {
                  // Make sure the file reference it created relative to the
                  // current file
                  urls.push(new File(res_file.getParentFile().getAbsolutePath(),
                                     (String)itr.next()));
               }
            }

            setConfigContext(ctx);
            return true;
         }
         catch (IOException ioe)
         {
            JOptionPane.showMessageDialog(getParentFrame(), ioe.getMessage(),
                                          "Error", JOptionPane.ERROR_MESSAGE);
            ioe.printStackTrace();
         }
      }

      return false;
   }

   /**
    * Programmatically executes an open action on configuration files
    * passed on the command line.
    */
   public boolean doOpenCmdArgs(ConfigContext ctx)
   {
      try
      {
         EnvironmentServiceProxy env_service = new EnvironmentServiceProxy();
         
         String[] args = env_service.getCommandLineArgs();
         if (null == args)
         {
            return false;
         }
         for (int i = 0 ; i < args.length ; ++i)
         {
            // Check if this part of the path is a valid directory we can read
            String file_name = args[i];
            File file = new File(file_name);
            if (file.exists() && file.isFile() && file.canRead() && file_name.endsWith(".jconf"))
            {
               System.out.println("Found configuration file in args: " + file);

               ConfigBroker broker = new ConfigBrokerProxy();

               // We want to automatically follow include directives. Keep track of
               // all the URLs on a stack and read them one at a time in the order
               // that we come across them
               Stack urls = new Stack();
               urls.push(file);
               while (! urls.isEmpty())
               {
                  // Expand env vars in the URL
                  File res_file = (File)urls.pop();
                  String res_name = expandEnvVars(res_file.getAbsolutePath());
                  System.out.println("Opening included resource: "+res_name);

                  FileDataSource data_source = FileDataSource.open(res_name, getBroker().getRepository());
                  broker.add(res_name, data_source);
                  ctx.add(res_name);

                  // Look through the elements in the newly loaded file and see if
                  // any of them are include directives
                  java.util.List includes = data_source.getIncludes();
                  for (Iterator itr = includes.iterator(); itr.hasNext(); )
                  {
                     // Make sure the file reference it created relative to the
                     // current file
                     urls.push(new File(res_file.getParentFile().getAbsolutePath(),
                                        (String)itr.next()));
                  }
               }
            }
         }
         setConfigContext(ctx);
         fireAction("Open");
         return true;
      }
      catch(Exception ex)
      {
         System.out.println(ex);
         ex.printStackTrace();
      }

      return false;
   }



   /**
    * Adds a RTRCDataSource into the given context.
    */
   public boolean doRTRC(ConfigContext ctx)
   {
      try
      {
         ConfigBroker broker = new ConfigBrokerProxy();
         RTRCDataSourceBroker RTRCBroker = new RTRCDataSourceBrokerProxy();
         ConnectionDialog dialog =
            new ConnectionDialog((Frame) getParentFrame(),
                                 "Remote Run-Time Reconfiguration Connections");
         dialog.show();

         if ( dialog.getStatus() == ConnectionDialog.OK_OPTION )
         {
            RTRCDataSource data_src = dialog.getDataSource();

            broker.add(data_src.toString(), data_src);
            ctx.add(data_src.toString());

            setConfigContext(ctx);
            return true;
         }
      }
      catch (Exception ioe)
      {
         JOptionPane.showMessageDialog(getParentFrame(), ioe.getMessage(),
                                       "Error", JOptionPane.ERROR_MESSAGE);
         ioe.printStackTrace();
      }

      return false;
   }
   
   /**
    * Programmatically execute a save action.
    */
   public boolean doSaveAll()
   {
      // Send a SaveAll action to all ConfigIFrames.
      fireAction("SaveAll");
      return true;
   }


   /**
    * Programmatically execute a close action.
    */
   public boolean doClose()
   {
      return true;
   }

   /**
    * Asks the user for a new config filename. If the user does not pick a
    * unique name, they are scolded and told to try again.
    */
   private String askUserForNewConfigFile()
   {
      boolean cancelled = false;
      String name = null;
      do
      {
         int result = fileChooser.showSaveDialog(getParentFrame());
         if (result == JFileChooser.APPROVE_OPTION)
         {
            File file = fileChooser.getSelectedFile();
            String filename = file.getAbsolutePath();
            if (! getBroker().containsDataSource(filename))
            {
               name = filename;
            }
            else
            {
               JOptionPane.showMessageDialog(getParentFrame(),
                                             "That resource is already open",
                                             "Oops!",
                                             JOptionPane.ERROR_MESSAGE);
            }
         }
         else
         {
            // The user cancelled the new
            cancelled = true;
         }
      }
      while (name == null && !cancelled);

      return name;
   }

   /**
    * Gets a handle to the configuration broker service.
    */
   private ConfigBroker getBroker()
   {
      return new ConfigBrokerProxy();
   }

   /**
    * Returns a copy of the given string with all environment variables
    * expanded.
    */
   private String expandEnvVars(String str)
   {
      return mEnvService.expandEnvVars(str);
   }

   /**
    * JBuilder auto-generated GUI code.
    */
   private void jbInit()
      throws Exception
   {
      this.setLayout(new BorderLayout());
      titleLbl.setBackground(UIManager.getColor("textHighlight"));
      titleLbl.setFont(new java.awt.Font("Serif", 1, 18));
      titleLbl.setForeground(Color.black);
      titleLbl.setBorder(BorderFactory.createRaisedBevelBorder());
      titleLbl.setOpaque(true);
      titleLbl.setHorizontalAlignment(SwingConstants.RIGHT);
      titleLbl.setText("VRJConfig");
      toolbar.setBorder(BorderFactory.createEtchedBorder());
      toolbar.setFloatable(false);
      newBtn.setToolTipText("New Configuration");
      newBtn.setActionCommand("New");
      newBtn.setFocusPainted(false);
      openBtn.setToolTipText("Open Configuration");
      openBtn.setActionCommand("Open");
      openBtn.setFocusPainted(false);
      RTRCBtn.setToolTipText("Run Time ReConfiguration");
      RTRCBtn.setActionCommand("RTRC");
      RTRCBtn.setFocusPainted(false);
      
      saveAllBtn.setEnabled(true);
      saveAllBtn.setToolTipText("Save All Open Configurations");
      saveAllBtn.setActionCommand("SaveAll");
      saveAllBtn.setFocusPainted(false);

      copyBtn.setEnabled(true);
      copyBtn.setToolTipText("Copy Config Element");
      copyBtn.setActionCommand("copy");
      copyBtn.setFocusPainted(false);
      pasteBtn.setEnabled(true);
      pasteBtn.setToolTipText("Paste Config Element");
      pasteBtn.setActionCommand("paste");
      pasteBtn.setFocusPainted(false);
      
      newBtn.addActionListener(new ActionListener()
      {
         public void actionPerformed(ActionEvent evt)
         {
            doNew();
         }
      });
      openBtn.addActionListener(new ActionListener()
      {
         public void actionPerformed(ActionEvent evt)
         {
            doOpen();
         }
      });
      saveAllBtn.addActionListener(new ActionListener()
      {
         public void actionPerformed(ActionEvent evt)
         {
            doSaveAll();
         }
      });
      copyBtn.addActionListener(new ActionListener()
      {
         public void actionPerformed(ActionEvent evt)
         {
            fireAction(evt.getActionCommand());
         }
      });
      pasteBtn.addActionListener(new ActionListener()
      {
         public void actionPerformed(ActionEvent evt)
         {
            fireAction(evt.getActionCommand());
         }
      });
      RTRCBtn.addActionListener(new ActionListener()
      {
         public void actionPerformed(ActionEvent evt)
         {
            doRTRC();
         }
      });
      this.add(titleLbl, BorderLayout.NORTH);
      this.add(toolbar, BorderLayout.CENTER);
      toolbar.add(newBtn, null);
      toolbar.add(openBtn, null);
      toolbar.add(RTRCBtn, null);
      toolbar.add(saveAllBtn, null);
      toolbar.addSeparator();
      toolbar.add(copyBtn, null);
      toolbar.add(pasteBtn, null);
      toolbar.addSeparator();
      toolbar.add(Box.createHorizontalGlue(), null);

      fileChooser.setFileSelectionMode(JFileChooser.FILES_ONLY);
      fileChooser.setFileHidingEnabled(false);
      fileChooser.setAcceptAllFileFilterUsed(false);
      fileChooser.setFileFilter(new ConfigFileFilter());
      fileChooser.setFileView(new ConfigFileView());
   }

   /**
    * Pragmatically executs a sava as.
    */
   public boolean doSaveAs()
   {
      try
      {
         fileChooser.setDialogTitle("Save As...");
         fileChooser.setDialogType(JFileChooser.SAVE_DIALOG);
          
         ConfigBroker broker = new ConfigBrokerProxy();
         ArrayList removed_sources = new ArrayList();
         ArrayList added_sources = new ArrayList();
         for (Iterator itr = context.getResources().iterator(); itr.hasNext(); )
         {
            String old_name = (String)itr.next();
            DataSource current_resource = broker.get( old_name );
            if (! current_resource.isReadOnly())
            {
               ///Where is os.path.split when you need it?
               String[] paths = old_name.split(File.separator);
               int length = java.lang.reflect.Array.getLength(paths);
               String title_name = paths[length - 1];
               fileChooser.setDialogTitle(title_name + " -- Save As...");
               int result = fileChooser.showSaveDialog(getParentFrame());
               if (result == JFileChooser.APPROVE_OPTION)
               {
                  /// XXX:  This is kind of ghetto; the only way to "rename" a resource is to remove it
                  ///       and then add it in again with a new name.
                  String new_name = fileChooser.getSelectedFile().getAbsolutePath();
                  ///       JFileChooser implements File Filters, but if the user types in a name, the JFile Chooser
                  ///       does NOT automatically add the selected file extension to the name.  Go figure.
                  if ( !new_name.matches(".*\\.jconf") )
                  {
                     new_name = new_name + ".jconf";
                  }
                  if (!old_name.equals(new_name))
                  {
                     java.io.File file = new File(new_name);
                     if (file.exists())
                     {
                        int result_existing = JOptionPane.showConfirmDialog(null, 
                           "File already exists, do you want to overwrite it?", "File exists!", 
                           JOptionPane.YES_NO_OPTION, JOptionPane.WARNING_MESSAGE);
                        
                        if(JOptionPane.YES_OPTION == result_existing)
                        {
                           file.delete();
                           /// We have to buffer the adds and removes to avoid ConcurrentModificationExceptions.
                           removed_sources.add(old_name);
                           added_sources.add(new_name);
                        }
                     }
                   }
                }
             }
          }
          for (Iterator itr = removed_sources.iterator(); itr.hasNext(); )
          {
             String cur = (String)itr.next();
             context.remove(cur);
             getBroker().remove(cur);
          }
          for (Iterator itr = added_sources.iterator(); itr.hasNext(); )
          {
             String cur = (String)itr.next();
             FileDataSource new_resource = FileDataSource.create(cur, getBroker().getRepository());
             getBroker().add(cur, new_resource);
             context.add(cur);
             new_resource.commit();
          }
          return true;
      }
      catch(IOException ioe)
      {
         JOptionPane.showMessageDialog(getParentFrame(), ioe.getMessage(),
                                       "Error", JOptionPane.ERROR_MESSAGE);
         ioe.printStackTrace();
      }
      return false;
   }

   private Container getParentFrame()
   {
      if ( null == mParentFrame )
      {
         mParentFrame = SwingUtilities.getAncestorOfClass(Frame.class, this);
      }

      return mParentFrame;
   }
   
   // JBuilder GUI variables
   private JLabel titleLbl = new JLabel();
   private JToolBar toolbar = new JToolBar();
   private JButton newBtn = new JButton();
   private JButton openBtn = new JButton();
   private JButton saveAllBtn = new JButton();
   private JButton copyBtn = new JButton();
   private JButton pasteBtn = new JButton();
   private JButton RTRCBtn = new JButton();
   private JFileChooser fileChooser = new JFileChooser();
   private ConfigContext context = new ConfigContext();
   private EnvironmentService mEnvService = new EnvironmentServiceProxy();
   private Container mParentFrame = null;
}
