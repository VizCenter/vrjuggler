/***************** <Tweek heading BEGIN do not edit this line> ****************
 * Tweek
 *
 * -----------------------------------------------------------------
 * File:          $RCSfile$
 * Date modified: $Date$
 * Version:       $Revision$
 * -----------------------------------------------------------------
 ***************** <Tweek heading END do not edit this line> *****************/

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
 *************** <auto-copyright.pl END do not edit this line> ***************/

package org.vrjuggler.tweek.gui;

import java.awt.*;
import java.awt.event.*;
import javax.swing.*;
import javax.swing.border.*;
import javax.swing.event.DocumentEvent;
import javax.swing.event.DocumentListener;
import javax.swing.event.ListSelectionEvent;
import javax.swing.event.ListSelectionListener;
import javax.swing.table.DefaultTableModel;
import org.vrjuggler.tweek.beans.BeanRegistry;
import org.vrjuggler.tweek.beans.TweekBean;
import org.vrjuggler.tweek.services.EnvironmentService;
import org.vrjuggler.tweek.services.GlobalPreferencesService;
import org.vrjuggler.tweek.net.corba.CorbaService;
import tweek.SubjectManagerPackage.SubjectMgrInfoItem;


/**
 * A modal dialog box used to make a connection with a remote ORB.  The Tweek
 * CORBA Service created as a result of the connection is made available to
 * external code.
 *
 * @since 1.0
 */
public class ConnectionDialog extends JDialog
{
   public ConnectionDialog (Frame owner, String title)
   {
      super(owner, title);

      try
      {
         jbInit();
      }
      catch(Exception e)
      {
         e.printStackTrace();
      }

      mNSHostField.getDocument().addDocumentListener(new AddressFieldValidator());
      mNSPortField.getDocument().addDocumentListener(new AddressFieldValidator());

      this.mSubjectMgrList.addListSelectionListener(new SubjectMgrListSelectionListener());

      // Set defaults for the Naming Service host and the port number.
      GlobalPreferencesService prefs =
         (GlobalPreferencesService) BeanRegistry.instance().getBean("GlobalPreferences");
      mNSHostField.setText(prefs.getDefaultCorbaHost());
      mNSPortField.setText(String.valueOf(prefs.getDefaultCorbaPort()));

      // Add an input validator for the port number field.
      mNSPortField.setInputVerifier(new InputVerifier()
         {
            public boolean verify(JComponent input)
            {
               JTextField tf = (JTextField) input;
               return validatePortNumber(tf.getText());
            }
         });

      this.setModal(true);
      this.setLocationRelativeTo(owner);
   }

   public void display ()
   {
      this.pack();
      this.setVisible(true);
   }

   public int getStatus ()
   {
      return status;
   }

   public String getNameServiceHost ()
   {
      return nameServiceHost;
   }

   public int getNameServicePort ()
   {
      return nameServicePort;
   }

   public String getNamingSubcontext ()
   {
      return namingSubcontext;
   }

   public CorbaService getCorbaService()
   {
      return corbaService;
   }

   public static final int OK_OPTION     = JOptionPane.OK_OPTION;
   public static final int CANCEL_OPTION = JOptionPane.CANCEL_OPTION;
   public static final int CLOSED_OPTION = JOptionPane.CLOSED_OPTION;

   protected void processWindowEvent (WindowEvent e)
   {
      super.processWindowEvent(e);

      if (e.getID() == WindowEvent.WINDOW_CLOSING)
      {
         status = CLOSED_OPTION;
      }
   }

   private void jbInit() throws Exception
   {
      mNSConnectBorder = new TitledBorder(new EtchedBorder(EtchedBorder.RAISED,Color.white,new Color(142, 142, 142)),"Naming Service Connection");
      mSubjectMgrBorder = new TitledBorder(new EtchedBorder(EtchedBorder.RAISED,Color.white,new Color(142, 142, 142)),"Subject Manager Choice");
      mNSConnectPanel.setLayout(mNSConnectLayout);
      mNSConnectPanel.setBorder(mNSConnectBorder);
      mNSConnectPanel.setMinimumSize(new Dimension(450, 175));

      mNSHostLabel.setHorizontalAlignment(SwingConstants.TRAILING);
      mNSHostLabel.setText("Naming Service Host");
      mNSHostField.setMinimumSize(new Dimension(80, 17));
      mNSHostField.setPreferredSize(new Dimension(180, 17));
      mNSHostField.addActionListener(new java.awt.event.ActionListener()
      {
         public void actionPerformed(ActionEvent e)
         {
            validateNetworkAddress(e);
         }
      });
      mNSPortField.addActionListener(new java.awt.event.ActionListener()
      {
         public void actionPerformed(ActionEvent e)
         {
            validateNetworkAddress(e);
         }
      });
      mSubjectMgrPanel.setBorder(mSubjectMgrBorder);
      mSubjectMgrPanel.setMinimumSize(new Dimension(450, 200));
      mSubjectMgrPanel.setPreferredSize(new Dimension(450, 200));
      mSubjectMgrPanel.setLayout(mSubjectMgrLayout);
      mSubjectMgrSplitPane.setDividerSize(7);
      mNSConnectButton.setEnabled(false);
      mNSConnectButton.setText("Connect");
      mNSConnectButton.addActionListener(new java.awt.event.ActionListener()
      {
         public void actionPerformed(ActionEvent e)
         {
            namingServiceConnectAction(e);
         }
      });
      mOkayButton.setEnabled(false);
      mSubjectMgrList.setSelectionMode(ListSelectionModel.SINGLE_SELECTION);
      mButtonPanel.setMinimumSize(new Dimension(450, 33));
      mButtonPanel.setPreferredSize(new Dimension(450, 33));
      mSubjectMgrListPane.setMinimumSize(new Dimension(100, 90));
      mSubjectMgrListPane.setPreferredSize(new Dimension(180, 90));
      mSubjectMgrInfoPane.setMinimumSize(new Dimension(100, 90));
      mSubjectMgrInfoPane.setPreferredSize(new Dimension(180, 90));
      mNSPortLabel.setHorizontalAlignment(SwingConstants.TRAILING);
      mNamingContextLabel.setHorizontalAlignment(SwingConstants.TRAILING);
      mNSConnectPanel.add(mNSHostLabel,         new GridBagConstraints(0, 0, 1, 1, 0.0, 0.0
            ,GridBagConstraints.CENTER, GridBagConstraints.NONE, new Insets(0, 0, 0, 2), 47, 18));
      mNSConnectPanel.add(mNSHostField,       new GridBagConstraints(1, 0, 1, 1, 1.0, 0.0
            ,GridBagConstraints.WEST, GridBagConstraints.HORIZONTAL, new Insets(0, 0, 0, 0), 14, 6));

      mNSPortLabel.setText("Naming Service Port");
      mNSPortField.setMinimumSize(new Dimension(50, 17));
      mNSPortField.setPreferredSize(new Dimension(50, 17));
      mNSConnectPanel.add(mNSPortLabel,    new GridBagConstraints(0, 1, 1, 1, 0.0, 0.0
            ,GridBagConstraints.CENTER, GridBagConstraints.NONE, new Insets(0, 0, 0, 2), 51, 18));
      mNSConnectPanel.add(mNSPortField,            new GridBagConstraints(1, 1, 1, 1, 1.0, 0.0
            ,GridBagConstraints.WEST, GridBagConstraints.NONE, new Insets(0, 0, 0, 0), 10, 6));

      mNamingContextLabel.setText("Naming Subcontext");
      mNamingContextField.setMinimumSize(new Dimension(80, 17));
      mNamingContextField.setPreferredSize(new Dimension(150, 17));


      mOkayButton.setText("OK");
      mOkayButton.setMnemonic('O');
      mOkayButton.setSelected(true);
      mOkayButton.addActionListener(new ActionListener()
      {
         public void actionPerformed (ActionEvent e)
         {
            okButtonAction(e);
         }
      });

      mCancelButton.setText("Cancel");
      mCancelButton.setMnemonic('C');
      mCancelButton.addActionListener(new ActionListener()
      {
         public void actionPerformed (ActionEvent e)
         {
            cancelButtonAction(e);
         }
      });

      this.getContentPane().add(mNSConnectPanel,  BorderLayout.NORTH);

      mNSConnectPanel.add(mNamingContextLabel,   new GridBagConstraints(0, 2, 1, 1, 0.0, 0.0
            ,GridBagConstraints.WEST, GridBagConstraints.NONE, new Insets(0, 0, 0, 2), 56, 18));
      mNSConnectPanel.add(mNamingContextField,    new GridBagConstraints(1, 2, 1, 1, 1.0, 0.0
            ,GridBagConstraints.WEST, GridBagConstraints.HORIZONTAL, new Insets(0, 0, 0, 0), 44, 6));
      mNSConnectPanel.add(mNSConnectButton,               new GridBagConstraints(0, 3, 2, 1, 0.0, 0.0
            ,GridBagConstraints.CENTER, GridBagConstraints.NONE, new Insets(5, 0, 0, 0), 0, 0));

      this.getContentPane().add(mButtonPanel, BorderLayout.SOUTH);
      mButtonPanel.add(mOkayButton, null);
      mButtonPanel.add(mCancelButton, null);
      this.getContentPane().add(mSubjectMgrPanel, BorderLayout.CENTER);
      mSubjectMgrPanel.add(mSubjectMgrSplitPane,  BorderLayout.CENTER);
      mSubjectMgrSplitPane.add(mSubjectMgrListPane, JSplitPane.LEFT);
      mSubjectMgrSplitPane.add(mSubjectMgrInfoPane, JSplitPane.RIGHT);
   }

   /**
    * Handles the event generated by the user clicking the "Connect" button in
    * the Naming Service connection panel.
    */
   private void namingServiceConnectAction(ActionEvent e)
   {
      // Commit the information provided by the user in the data entry fields.
      commitConnectInfo();

      // Create a new CORBA service using the information provided by the user.
      CorbaService new_orb = new CorbaService(this.getNameServiceHost(),
                                              this.getNameServicePort(),
                                              this.getNamingSubcontext());

      TweekBean service = BeanRegistry.instance().getBean("Environment");

      try
      {
         // If we have the Tweek Environment Service, and we should, initialize
         // the new CORBA service with the command line arguments passed when
         // the Tweek GUI was started.
         if ( null != service )
         {
            EnvironmentService env_service = (EnvironmentService) service;
            new_orb.init(env_service.getCommandLineArgs());
         }
         else
         {
            new_orb.init(null);
         }

         // XXX: Is this the best time to do this?  We want to be sure that
         // anyone who calls getCorbaService() gets a valid reference back.  If
         // something goes wrong later with the Subject Manager choice, this
         // should be set to null.
         corbaService = new_orb;

         // XXX: Should we allow the user to make multiple connections from a
         // single dialog box?  Probably not...
         mNSConnectButton.setEnabled(false);

         // Create a new list model for the fresh list of Subject Managers.
         DefaultListModel mgr_list_model = new DefaultListModel();

         // Get the list of Subject Managers.  There had better be at least one!
         java.util.Iterator i = new_orb.getSubjectManagerList().iterator();
         tweek.SubjectManager cur_mgr;

         while ( i.hasNext() )
         {
            cur_mgr = (tweek.SubjectManager) i.next();
            mgr_list_model.addElement(new SubjectManagerWrapper(cur_mgr));
         }

         mSubjectMgrList.setModel(mgr_list_model);
      }
      // Something went wrong with the CORBA service initialization.
      catch (org.omg.CORBA.SystemException sys_ex)
      {
         // Disable the Naming Service connection button just to be safe.
         mNSConnectButton.setEnabled(false);
         JOptionPane.showMessageDialog(null, "CORBA System Exception: '" +
                                       sys_ex.getMessage() + "'",
                                       "CORBA System Exception",
                                       JOptionPane.ERROR_MESSAGE);

         // DEBUG
         sys_ex.printStackTrace();
      }
   }

   /**
    * Commits the user-specified information from the text fields to this
    * object's properties.  After calling this method, those properties can be
    * considered up to date and usable.
    */
   private void commitConnectInfo()
   {
      nameServiceHost  = mNSHostField.getText();
      nameServicePort  = Integer.parseInt(mNSPortField.getText());
      namingSubcontext = mNamingContextField.getText();
   }

   private void okButtonAction (ActionEvent e)
   {
      status = OK_OPTION;
      commit();
      setVisible(false);
   }

   /**
    * Sets the close status to CANCEL_OPTION and closes this dialog box.  If
    * an ORB is running, it is shut down.
    */
   private void cancelButtonAction (ActionEvent e)
   {
      // If we have an ORB running, we have to shut it down.
      if ( null != corbaService )
      {
         corbaService.shutdown(true);
         corbaService = null;
      }

      status = CANCEL_OPTION;
      setVisible(false);
   }

   private void commit()
   {
      if ( null != mSubjectManager )
      {
         corbaService.setSubjectManager(mSubjectManager);
      }
   }

   /**
    * Validates that the network address (hostname and port number) entered
    * by the user.  If the network address is valid, then the Naming Service
    * connection button is enabled.  Otherwise, it is disabled.
    */
   private void validateNetworkAddress(ActionEvent e)
   {
      if ( ! mNSHostField.getText().equals("") &&
           validatePortNumber(mNSPortField.getText()) )
      {
         mNSConnectButton.setEnabled(true);
      }
      else
      {
         mNSConnectButton.setEnabled(false);
      }
   }

   /**
    * Verifies that the given string is a valid port number.
    */
   private boolean validatePortNumber(String port)
   {
      boolean valid = false;

      try
      {
         int port_num = Integer.parseInt(port);

         if ( port_num > 0 && port_num < 65536 )
         {
            valid = true;
         }
      }
      catch (Exception e)
      {
         System.err.println("Invalid port number: " + port);
      }

      return valid;
   }

   /**
    * An implementation of DocumentListener used to validate the network
    * address entered for the CORBA Naming Service.
    */
   private class AddressFieldValidator implements DocumentListener
   {
      public void insertUpdate(DocumentEvent e)
      {
         validateNetworkAddress(null);
      }

      public void removeUpdate(DocumentEvent e)
      {
         validateNetworkAddress(null);
      }

      public void changedUpdate(DocumentEvent e)
      {
         validateNetworkAddress(null);
      }
   }

   /**
    * An implementation of ListSelectionListener for use with the list of
    * known Tweek Subject Manager references.  When a Subject Manager is
    * selected in the list, the table mSubjectMgrInfo is updated to display
    * information queried from that Subject Manager.
    */
   private class SubjectMgrListSelectionListener
      implements ListSelectionListener
   {
      public void valueChanged(ListSelectionEvent e)
      {
         // Pull the tweek.SubjectManager reference from the list.
         int index = mSubjectMgrList.getSelectedIndex();

         // No selection.
         if ( -1 == index )
         {
            mOkayButton.setEnabled(false);
         }
         // New selection.
         else
         {
            SubjectManagerWrapper mgr_wrapper =
               (SubjectManagerWrapper) mSubjectMgrList.getModel().getElementAt(index);
            mSubjectManager = mgr_wrapper.getSubjectManager();

            // Fill in the table model for the selected Subject Manager.
            Object[] column_names = new Object[]{"Property", "Value"};
            DefaultTableModel table_model = new DefaultTableModel();
            table_model.setColumnIdentifiers(column_names);

            try
            {
               SubjectMgrInfoItem[] subj_mgr_info = mSubjectManager.getInfo();

               for ( int i = 0; i < subj_mgr_info.length; ++i )
               {
                  table_model.addRow(new Object[]{subj_mgr_info[i].key,
                                                  subj_mgr_info[i].value});
               }

               mSubjectMgrInfo.setModel(table_model);
               mOkayButton.setEnabled(true);
            }
            catch (org.omg.CORBA.COMM_FAILURE commEx)
            {
               mOkayButton.setEnabled(false);
               String msg = "Invalid Subject Manager Selected";

               // If the exception has an error message, append it.
               if ( ! commEx.getMessage().equals("") )
               {
                  msg += ": '" + commEx.getMessage() + "'";
               }

               JOptionPane.showMessageDialog(null, msg,
                                             "CORBA Communication Exception",
                                             JOptionPane.ERROR_MESSAGE);
            }
         }
      }
   }

   // Attributes that may be queried by the class that instantiated us.
   private int          status;
   private String       nameServiceHost  = null;
   private int          nameServicePort  = 2809;
   private String       namingSubcontext = null;
   private CorbaService corbaService     = null;

   // Internal-use properties.
   private tweek.SubjectManager mSubjectManager = null;

   // GUI elements for the Naming Service connection panel.
   private TitledBorder  mNSConnectBorder;
   private JPanel        mNSConnectPanel     = new JPanel();
   private GridBagLayout mNSConnectLayout    = new GridBagLayout();
   private JLabel        mNSHostLabel        = new JLabel();
   private JTextField    mNSHostField        = new JTextField();
   private JLabel        mNSPortLabel        = new JLabel();
   private JTextField    mNSPortField        = new JTextField();
   private JLabel        mNamingContextLabel = new JLabel();
   private JTextField    mNamingContextField = new JTextField();

   // GUI elements for the Subject Manager panel.
   private TitledBorder mSubjectMgrBorder;
   private JPanel       mSubjectMgrPanel     = new JPanel();
   private BorderLayout mSubjectMgrLayout    = new BorderLayout();
   private JSplitPane   mSubjectMgrSplitPane = new JSplitPane();
   private JList        mSubjectMgrList      = new JList();
   private JScrollPane  mSubjectMgrListPane  = new JScrollPane(mSubjectMgrList);
   private JTable       mSubjectMgrInfo      = new JTable();
   private JScrollPane  mSubjectMgrInfoPane  = new JScrollPane(mSubjectMgrInfo);
   private JButton      mNSConnectButton     = new JButton();

   // GUI elements for the OK/Cancel button panel.
   private JPanel  mButtonPanel  = new JPanel();
   private JButton mOkayButton   = new JButton();
   private JButton mCancelButton = new JButton();
}
