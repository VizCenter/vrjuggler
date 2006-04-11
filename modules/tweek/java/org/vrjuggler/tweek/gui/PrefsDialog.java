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
import java.util.Iterator;
import java.util.List;
import javax.swing.*;
import javax.swing.border.*;
import javax.swing.event.TableModelEvent;
import javax.swing.event.TableModelListener;
import javax.swing.table.AbstractTableModel;
import org.vrjuggler.tweek.services.GlobalPreferencesService;


/**
 * @since 1.0
 */
public class PrefsDialog extends JDialog implements TableModelListener
{
   public PrefsDialog (Frame owner, String title,
                       GlobalPreferencesService prefs)
   {
      super(owner, title);

      mPrefs = prefs;

      userLevel        = mPrefs.getUserLevel();
      lookAndFeel      = mPrefs.getLookAndFeel();
      beanViewer       = mPrefs.getBeanViewer();
      windowWidth      = new Integer(mPrefs.getWindowWidth());
      windowHeight     = new Integer(mPrefs.getWindowHeight());
      chooserStartDir  = mPrefs.getChooserStartDir();
      chooserOpenStyle = mPrefs.getChooserOpenStyle();
      defaultCorbaHost = mPrefs.getDefaultCorbaHost();
      defaultCorbaPort = mPrefs.getDefaultCorbaPort();

      WindowSizeTableModel table_model = new WindowSizeTableModel(windowWidth,
                                                                  windowHeight);
      table_model.addTableModelListener(this);
      mWinSizeTable = new JTable(table_model);

      try
      {
         jbInit();
      }
      catch(Exception e)
      {
         e.printStackTrace();
      }

      this.configComboBoxes();

      mCorbaHostField.setText(String.valueOf(defaultCorbaHost));
      mCorbaPortField.setText(String.valueOf(defaultCorbaPort));

      switch ( chooserOpenStyle )
      {
         case GlobalPreferencesService.EMACS_CHOOSER:
            mEmacsStyleButton.setSelected(true);
            break;
         case GlobalPreferencesService.WINDOWS_CHOOSER:
         default:
            mWindowsStyleButton.setSelected(true);
            break;
      }

      this.setModal(true);
      this.setLocationRelativeTo(owner);
   }

   public void display ()
   {
      this.pack();
      this.setVisible(true);
   }

   public void setBeanViewer (String v)
   {
      mPrefs.setBeanViewer(v);
   }

   public int getStatus ()
   {
      return status;
   }

   public void tableChanged(TableModelEvent e)
   {
      WindowSizeTableModel model =
         (WindowSizeTableModel) e.getSource();

      switch (e.getColumn())
      {
         case 0:
            windowWidth = (Integer) model.getValueAt(0, 0);
            break;
         case 1:
            windowHeight = (Integer) model.getValueAt(0, 1);
            break;
      }
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
      mOkButton.setMnemonic('O');
      mOkButton.setText("OK");
      mOkButton.addActionListener(new ActionListener()
      {
         public void actionPerformed (ActionEvent e)
         {
            okButtonAction(e);
         }
      });

      mApplyButton.setMnemonic('A');
      mApplyButton.setText("Apply");
      mApplyButton.addActionListener(new ActionListener()
      {
         public void actionPerformed(ActionEvent e)
         {
            applyButtonAction(e);
         }
      });

      mCancelButton.setMnemonic('C');
      mCancelButton.setText("Cancel");
      mCancelButton.addActionListener(new ActionListener()
      {
         public void actionPerformed (ActionEvent e)
         {
            cancelButtonAction(e);
         }
      });

      mFileChooserPanel.setLayout(mFileChooserLayout);
      mFcConfigPanel.setLayout(mFcConfigLayout);
      mFcStartDirLabel.setMaximumSize(new Dimension(24, 13));
      mFcStartDirLabel.setMinimumSize(new Dimension(24, 13));
      mFcStartDirLabel.setPreferredSize(new Dimension(24, 13));
      mFcStartDirLabel.setHorizontalAlignment(SwingConstants.RIGHT);
      mFcStartDirLabel.setText("Start Directory");
      mFcStartDirBox.setMinimumSize(new Dimension(126, 10));
      mFcStartDirBox.setPreferredSize(new Dimension(130, 10));
      mFcStartDirBox.setEditable(true);
      mFcOpenStyleTitleLabel.setMaximumSize(new Dimension(24, 13));
      mFcOpenStyleTitleLabel.setMinimumSize(new Dimension(24, 13));
      mFcOpenStyleTitleLabel.setPreferredSize(new Dimension(24, 13));
      mFcOpenStyleTitleLabel.setHorizontalAlignment(SwingConstants.RIGHT);
      mFcOpenStyleTitleLabel.setText("Open Style");
      mEmacsStyleButton.setMinimumSize(new Dimension(109, 15));
      mEmacsStyleButton.setPreferredSize(new Dimension(109, 15));
      mEmacsStyleButton.setText("Emacs Style");
      mEmacsStyleButton.addActionListener(new ActionListener()
         {
            public void actionPerformed (ActionEvent e)
            {
               chooserOpenStyle = GlobalPreferencesService.EMACS_CHOOSER;
            }
         });

      mWindowsStyleButton.setMinimumSize(new Dimension(128, 15));
      mWindowsStyleButton.setPreferredSize(new Dimension(128, 15));
      mWindowsStyleButton.setText("Windows Style");
      mWindowsStyleButton.addActionListener(new ActionListener()
         {
            public void actionPerformed (ActionEvent e)
            {
               chooserOpenStyle = GlobalPreferencesService.WINDOWS_CHOOSER;
            }
         });

      mFcOpenStyleButtonPanel.setLayout(mFcOpenStyleButtonLayout);
      mFcOpenStyleButtonLayout.setColumns(1);
      mFcOpenStyleButtonLayout.setRows(0);
      mFcOpenStyleButtonPanel.setMinimumSize(new Dimension(128, 50));
      mFcOpenStyleButtonPanel.setPreferredSize(new Dimension(128, 50));
      mLazyInstanceButton.setSelected(mPrefs.getLazyPanelBeanInstantiation());
      mLazyInstanceButton.setText("Lazy Panel Bean Instantiaion");
      mLevelBox.setMaximumSize(new Dimension(130, 26));
      mLevelBox.setMinimumSize(new Dimension(70, 26));
      mLevelBox.setPreferredSize(new Dimension(70, 26));
      mViewerBox.setMaximumSize(new Dimension(32767, 26));
      mViewerBox.setMinimumSize(new Dimension(126, 26));
      mViewerBox.setPreferredSize(new Dimension(130, 26));
      mGeneralPanel.setToolTipText("General Tweek interface configuration");
      mGeneralPanel.setLayout(mGenLayout);
      mLevelLabel.setMinimumSize(new Dimension(24, 13));
      mLevelLabel.setPreferredSize(new Dimension(24, 13));
      mLevelLabel.setHorizontalAlignment(SwingConstants.RIGHT);
      mLevelLabel.setLabelFor(mLevelBox);
      mLevelLabel.setText("User Level");
      mViewerLabel.setMaximumSize(new Dimension(24, 13));
      mViewerLabel.setMinimumSize(new Dimension(24, 13));
      mViewerLabel.setPreferredSize(new Dimension(24, 13));
      mViewerLabel.setHorizontalAlignment(SwingConstants.RIGHT);
      mViewerLabel.setLabelFor(mViewerBox);
      mViewerLabel.setText("Bean Viewer");
      mGenConfigPanel.setLayout(mGenConfigLayout);
      mLafBox.setMaximumSize(new Dimension(32767, 26));
      mLafBox.setMinimumSize(new Dimension(126, 26));
      mLafBox.setPreferredSize(new Dimension(130, 26));
      mLafLabel.setMaximumSize(new Dimension(74, 13));
      mLafLabel.setMinimumSize(new Dimension(24, 13));
      mLafLabel.setPreferredSize(new Dimension(24, 13));
      mLafLabel.setHorizontalAlignment(SwingConstants.RIGHT);
      mLafLabel.setLabelFor(mLafBox);
      mLafLabel.setText("Look and Feel");
      mCorbaPortField.setMaximumSize(new Dimension(15, 17));
      mCorbaPortField.setMinimumSize(new Dimension(15, 17));
      mCorbaPortField.setPreferredSize(new Dimension(15, 17));
      mCorbaPortField.setToolTipText("The port number of the CORBA Naming Service");
      mCorbaPortField.addFocusListener(new java.awt.event.FocusAdapter()
      {
         public void focusLost(FocusEvent e)
         {
            corbaPortFieldChanged();
         }
      });
      mCorbaPortField.addActionListener(new java.awt.event.ActionListener()
      {
         public void actionPerformed(ActionEvent e)
         {
            corbaPortFieldChanged();
         }
      });
      mCorbaPortLabel.setMaximumSize(new Dimension(77, 13));
      mCorbaPortLabel.setMinimumSize(new Dimension(77, 13));
      mCorbaPortLabel.setPreferredSize(new Dimension(77, 13));
      mCorbaPortLabel.setHorizontalAlignment(SwingConstants.RIGHT);
      mCorbaPortLabel.setLabelFor(mCorbaPortField);
      mCorbaPortLabel.setText("Port Number");
      mCorbaPanel.setLayout(mCorbaLayout);
      mCorbaHostField.setMaximumSize(new Dimension(120, 17));
      mCorbaHostField.setMinimumSize(new Dimension(85, 17));
      mCorbaHostField.setPreferredSize(new Dimension(85, 17));
      mCorbaHostField.setToolTipText("The hostname for the CORBA Naming Service");
      mCorbaHostField.addFocusListener(new java.awt.event.FocusAdapter()
      {
         public void focusLost(FocusEvent e)
         {
            corbaHostFieldChanged();
         }
      });
      mCorbaHostField.addActionListener(new java.awt.event.ActionListener()
      {
         public void actionPerformed(ActionEvent e)
         {
            corbaHostFieldChanged();
         }
      });
      mCorbaHostLabel.setHorizontalAlignment(SwingConstants.RIGHT);
      mCorbaHostLabel.setLabelFor(mCorbaHostField);
      mCorbaHostLabel.setText("Host Name");

      mWinSizeLabel.setHorizontalAlignment(SwingConstants.RIGHT);
      mWinSizeLabel.setText("Window Size");
      mWinSizeTable.setCellSelectionEnabled(true);
      mWinSizeTable.setRowSelectionAllowed(false);
      mWinSizeTablePane.setMinimumSize(new Dimension(150, 45));
      mWinSizeTablePane.setPreferredSize(new Dimension(150, 45));
      mButtonPanel.add(mOkButton, null);
      mButtonPanel.add(mApplyButton, null);
      mButtonPanel.add(mCancelButton, null);
      this.getContentPane().add(mContentPane,  BorderLayout.CENTER);
      mContentPane.add(mGeneralPanel,   "General");
      mGeneralPanel.add(mGenConfigPanel, BorderLayout.CENTER);
      mGenConfigPanel.add(mLevelLabel,  new GridBagConstraints(0, 1, 1, 1, 0.0, 0.0
            ,GridBagConstraints.WEST, GridBagConstraints.NONE, new Insets(0, 0, 0, 3), 95, 23));
      mGenConfigPanel.add(mLevelBox,      new GridBagConstraints(1, 1, 1, 1, 1.0, 0.0
            ,GridBagConstraints.WEST, GridBagConstraints.NONE, new Insets(0, 0, 0, 0), 0, 0));
      mGenConfigPanel.add(mLafLabel,  new GridBagConstraints(0, 2, 1, 1, 0.0, 0.0
            ,GridBagConstraints.WEST, GridBagConstraints.NONE, new Insets(0, 0, 0, 3), 95, 23));
      mGenConfigPanel.add(mLafBox,    new GridBagConstraints(1, 2, 1, 1, 1.0, 0.0
            ,GridBagConstraints.CENTER, GridBagConstraints.HORIZONTAL, new Insets(0, 0, 0, 0), 0, 0));
      mGenConfigPanel.add(mViewerLabel,  new GridBagConstraints(0, 3, 1, 1, 0.0, 0.0
            ,GridBagConstraints.WEST, GridBagConstraints.NONE, new Insets(0, 0, 0, 3), 95, 23));
      mGenConfigPanel.add(mViewerBox,   new GridBagConstraints(1, 3, 1, 1, 1.0, 0.0
            ,GridBagConstraints.CENTER, GridBagConstraints.HORIZONTAL, new Insets(0, 0, 0, 0), 64, 0));
      mGenConfigPanel.add(mLazyInstanceButton,  new GridBagConstraints(1, 0, 1, 1, 0.0, 0.0
            ,GridBagConstraints.WEST, GridBagConstraints.NONE, new Insets(0, 0, 0, 0), 0, 0));
      mGenConfigPanel.add(mWinSizeLabel,    new GridBagConstraints(0, 4, 1, 1, 0.0, 0.0
            ,GridBagConstraints.EAST, GridBagConstraints.NONE, new Insets(0, 0, 0, 3), 0, 23));
      mContentPane.add(mCorbaPanel,   "CORBA");
      mCorbaPanel.add(mCorbaHostLabel,         new GridBagConstraints(0, 0, 1, 1, 0.0, 0.0
            ,GridBagConstraints.WEST, GridBagConstraints.NONE, new Insets(0, 0, 0, 0), 40, 9));
      mCorbaPanel.add(mCorbaHostField,            new GridBagConstraints(1, 0, 1, 1, 1.0, 0.0
            ,GridBagConstraints.WEST, GridBagConstraints.HORIZONTAL, new Insets(0, 0, 0, 4), 112, 9));
      mCorbaPanel.add(mCorbaPortLabel,             new GridBagConstraints(0, 1, 1, 1, 0.0, 0.0
            ,GridBagConstraints.WEST, GridBagConstraints.NONE, new Insets(4, 0, 1, 2), 40, 9));
      mCorbaPanel.add(mCorbaPortField,                new GridBagConstraints(1, 1, 1, 1, 1.0, 0.0
            ,GridBagConstraints.WEST, GridBagConstraints.NONE, new Insets(4, 0, 1, 1), 45, 9));

      this.getContentPane().add(mButtonPanel, BorderLayout.SOUTH);
      mFileChooserPanel.add(mFcConfigPanel, BorderLayout.CENTER);
      mFcConfigPanel.add(mFcStartDirLabel,       new GridBagConstraints(0, 0, 1, 1, 0.0, 0.0
            ,GridBagConstraints.WEST, GridBagConstraints.NONE, new Insets(0, 0, 0, 0), 95, 23));
      mFcConfigPanel.add(mFcStartDirBox,        new GridBagConstraints(1, 0, 1, 1, 1.0, 0.0
            ,GridBagConstraints.CENTER, GridBagConstraints.HORIZONTAL, new Insets(0, 0, 0, 0), 64, 14));
      mFcConfigPanel.add(mFcOpenStyleTitleLabel,             new GridBagConstraints(0, 1, 1, 1, 0.0, 0.0
            ,GridBagConstraints.WEST, GridBagConstraints.NONE, new Insets(0, 0, 0, 3), 95, 23));
      mFcConfigPanel.add(mFcOpenStyleButtonPanel,           new GridBagConstraints(1, 1, 1, 1, 1.0, 1.0
            ,GridBagConstraints.CENTER, GridBagConstraints.HORIZONTAL, new Insets(0, 0, 0, 0), 66, 0));
      mFcOpenStyleButtonPanel.add(mWindowsStyleButton, null);
      mFcOpenStyleButtonPanel.add(mEmacsStyleButton, null);
      mOpenStyleButtonGroup.add(mWindowsStyleButton);
      mOpenStyleButtonGroup.add(mEmacsStyleButton);
      mContentPane.add(mFileChooserPanel,  "File Chooser");
      mWinSizeTablePane.getViewport().add(mWinSizeTable);
      mGenConfigPanel.add(mWinSizeTablePane,           new GridBagConstraints(1, 4, 1, 1, 0.0, 0.0
            ,GridBagConstraints.CENTER, GridBagConstraints.HORIZONTAL, new Insets(0, 0, 0, 0), 0, 0));
   }

   /**
    * Adds items to all the combo boxes.  In some cases, other modifications
    * to a combo box will be made.  These can include setting the default
    * selected value and/or adding action listeners.
    */
   private void configComboBoxes ()
   {
      // ----------------------------------------------------------------------
      // Add all the known Bean viewers to mViewerBox.
      java.util.Vector viewers = mPrefs.getBeanViewers();

      for ( int i = 0; i < viewers.size(); i++ )
      {
         mViewerBox.addItem(viewers.elementAt(i));
      }

      mViewerBox.setSelectedItem(mPrefs.getBeanViewer());
      mViewerBox.addActionListener(new ActionListener()
         {
            public void actionPerformed (ActionEvent e)
            {
               beanViewer = (String) mViewerBox.getSelectedItem();
            }
         });
      // ----------------------------------------------------------------------

      // ----------------------------------------------------------------------
      // Add user level options to mLevelBox.
      for ( int i = GlobalPreferencesService.MIN_USER_LEVEL;
            i <= GlobalPreferencesService.MAX_USER_LEVEL;
            i++ )
      {
         mLevelBox.addItem(String.valueOf(i));
      }

      mLevelBox.setSelectedIndex(mPrefs.getUserLevel() - 1);
      mLevelBox.addActionListener(new ActionListener()
         {
            public void actionPerformed (ActionEvent e)
            {
               userLevel = mLevelBox.getSelectedIndex() + 1;
            }
         });
      // ----------------------------------------------------------------------

      // ----------------------------------------------------------------------
      // Handle the Look-and-Feel box.
      UIManager.LookAndFeelInfo[] lafs = UIManager.getInstalledLookAndFeels();
      int selected_index = 0;
      String cur_laf = mPrefs.getLookAndFeel();

      // Add the available look-and-feel objects to the combo box.
      for ( int i = 0; i < lafs.length; ++i )
      {
         mLafBox.addItem(lafs[i]);

         if ( lafs[i].getClassName().equals(cur_laf) )
         {
            selected_index = i;
         }
      }

      mLafBox.setSelectedIndex(selected_index);
      mLafBox.setRenderer(new LAFBoxRenderer());
      mLafBox.addActionListener(new ActionListener()
         {
            public void actionPerformed (ActionEvent e)
            {
               UIManager.LookAndFeelInfo val =
                  (UIManager.LookAndFeelInfo) mLafBox.getSelectedItem();
               lookAndFeel = val.getClassName();
            }
         });
      // ----------------------------------------------------------------------

      // ----------------------------------------------------------------------
      // Handle the file chooser starting directory box stuff.
      Iterator i = GlobalPreferencesService.getStartDirList().iterator();

      boolean has_start_dir = false;

      while ( i.hasNext() )
      {
         String dir = (String) i.next();

         if ( dir.equals(chooserStartDir) )
         {
            has_start_dir = true;
         }

         mFcStartDirBox.addItem(dir);
      }

      if ( ! has_start_dir )
      {
         mFcStartDirBox.addItem(chooserStartDir);
      }

      mFcStartDirBox.setSelectedItem(chooserStartDir);

      mFcStartDirBox.addActionListener(new ActionListener()
         {
            public void actionPerformed (ActionEvent e)
            {
               chooserStartDir = (String) mFcStartDirBox.getSelectedItem();
            }
         });
      // ----------------------------------------------------------------------
   }

   private void okButtonAction (ActionEvent e)
   {
      status = OK_OPTION;
      commit();
      mPrefs.save();
      setVisible(false);
   }

   private void applyButtonAction(ActionEvent e)
   {
      commit();
      mPrefs.save();
   }

   private void cancelButtonAction (ActionEvent e)
   {
      status = CANCEL_OPTION;
      setVisible(false);
   }

   private void commit ()
   {
      mPrefs.setUserLevel(userLevel);
      mPrefs.setLookAndFeel(lookAndFeel);
      mPrefs.setBeanViewer(beanViewer);
      mPrefs.setWindowWidth(windowWidth.intValue());
      mPrefs.setWindowHeight(windowHeight.intValue());
      mPrefs.setChooserStartDir(chooserStartDir);
      mPrefs.setChooserOpenStyle(chooserOpenStyle);
      mPrefs.setLazyPanelBeanInstantiation(mLazyInstanceButton.isSelected());
      mPrefs.setDefaultCorbaHost(defaultCorbaHost);
      mPrefs.setDefaultCorbaPort(defaultCorbaPort);
   }

   /**
    * Action taken when the user changes the text field containing the default
    * CORBA port.  This validates the entered port number to ensure that it is
    * valid.
    */
   private void corbaHostFieldChanged()
   {
      defaultCorbaHost = mCorbaHostField.getText();
   }

   /**
    * Action taken when the user changes the text field containing the default
    * CORBA port.  This validates the entered port number to ensure that it is
    * valid.
    */
   private void corbaPortFieldChanged()
   {
      try
      {
         int port = Integer.parseInt(mCorbaPortField.getText());

         if ( port > 0 && port < 65536 )
         {
            defaultCorbaPort = port;
         }
         else
         {
            mCorbaPortField.setText(String.valueOf(defaultCorbaPort));
         }
      }
      catch (Exception ex)
      {
         mCorbaPortField.setText(String.valueOf(defaultCorbaPort));
      }
   }

   private class WindowSizeTableModel extends AbstractTableModel
   {
      public WindowSizeTableModel(Integer width, Integer height)
      {
         this.width  = width;
         this.height = height;
      }

      public int getRowCount()
      {
         return 1;
      }

      public int getColumnCount()
      {
         return 2;
      }

      public String getColumnName(int col)
      {
         return (col == 0 ? "Width" : "Height");
      }

      public Object getValueAt(int row, int column)
      {
         return (column == 0 ? width : height);
      }

      public void setValueAt(Object newVal, int row, int column)
      {
         switch (column)
         {
            case 0:
               width = (Integer) newVal;
               break;
            case 1:
               height = (Integer) newVal;
               break;
         }

         fireTableCellUpdated(row, column);
      }

      /**
       * Overrides the default getColumnClass().  This method is critical for
       * getting cell renderers to work.
       */
      public Class getColumnClass(int column)
      {
         return Integer.class;
      }

      public boolean isCellEditable(int row, int col)
      {
         return true;
      }

      private Integer width;
      private Integer height;
   }

   private static class LAFBoxRenderer extends JLabel
                                       implements ListCellRenderer
   {
      public LAFBoxRenderer()
      {
         setOpaque(true);
      }

      public Component getListCellRendererComponent(JList list, Object value,
                                                    int index,
                                                    boolean isSelected,
                                                    boolean cellHasFocus)
      {
         if ( isSelected )
         {
            setBackground(list.getSelectionBackground());
            setForeground(list.getSelectionForeground());
         }
         else
         {
            setBackground(list.getBackground());
            setForeground(list.getForeground());
         }

         UIManager.LookAndFeelInfo laf = (UIManager.LookAndFeelInfo) value;
         setText(laf.getName());
         return this;
      }
   }

   // =========================================================================
   // Private data members.
   // =========================================================================

   private int status;

   private int     userLevel        = 0;
   private String  lookAndFeel      = null;
   private String  beanViewer       = null;
   private Integer windowWidth      = new Integer(1024);
   private Integer windowHeight     = new Integer(768);
   private String  chooserStartDir  = GlobalPreferencesService.DEFAULT_START;
   private int     chooserOpenStyle = GlobalPreferencesService.DEFAULT_CHOOSER;
   private String  defaultCorbaHost = "";
   private int     defaultCorbaPort = 0;

   private GlobalPreferencesService mPrefs = null;

   private JPanel       mFileChooserPanel = new JPanel();
   private BorderLayout mFileChooserLayout = new BorderLayout();

   private JPanel        mFcConfigPanel  = new JPanel();
   private GridBagLayout mFcConfigLayout = new GridBagLayout();

   private JLabel       mFcStartDirLabel         = new JLabel();
   private JComboBox    mFcStartDirBox           = new JComboBox();
   private JLabel       mFcOpenStyleTitleLabel   = new JLabel();
   private JPanel       mFcOpenStyleButtonPanel  = new JPanel();
   private GridLayout   mFcOpenStyleButtonLayout = new GridLayout();
   private ButtonGroup  mOpenStyleButtonGroup    = new ButtonGroup();
   private JRadioButton mWindowsStyleButton      = new JRadioButton();
   private JRadioButton mEmacsStyleButton        = new JRadioButton();

   private JPanel  mButtonPanel  = new JPanel();
   private JButton mCancelButton = new JButton();
   private JButton mApplyButton  = new JButton();
   private JButton mOkButton     = new JButton();

   private JTabbedPane mContentPane = new JTabbedPane();
   private JCheckBox mLazyInstanceButton = new JCheckBox();
   private JComboBox mLevelBox = new JComboBox();
   private JComboBox mViewerBox = new JComboBox();
   private JPanel mGeneralPanel = new JPanel();
   private JLabel mLevelLabel = new JLabel();
   private BorderLayout mGenLayout = new BorderLayout();
   private JLabel mViewerLabel = new JLabel();
   private GridBagLayout mGenConfigLayout = new GridBagLayout();
   private JPanel mGenConfigPanel = new JPanel();
   private JComboBox mLafBox = new JComboBox();
   private JLabel mLafLabel = new JLabel();
   private JTextField mCorbaPortField = new JTextField();
   private JLabel mCorbaPortLabel = new JLabel();
   private JPanel mCorbaPanel = new JPanel();
   private JTextField mCorbaHostField = new JTextField();
   private JLabel mCorbaHostLabel = new JLabel();
   private GridBagLayout mCorbaLayout = new GridBagLayout();
   private JLabel mWinSizeLabel = new JLabel();
   private JTable mWinSizeTable = null;
   private JScrollPane mWinSizeTablePane = new JScrollPane();
}
