package org.vrjuggler.jccl.editors;

import java.awt.*;
import java.awt.datatransfer.*;
import java.awt.dnd.*;
import java.awt.event.*;
import java.util.*;
import javax.swing.*;
import javax.swing.event.*;
import javax.swing.tree.*;

import org.vrjuggler.jccl.config.*;

/**
 * Wraps a ConfigElement to allow us to transfer it from on context to another
 * either using copy/past or drag and drop methods.
 */
class ConfigElementSelection implements ClipboardOwner, Transferable
{
   /**
    * Constructor that takes the given ConfigElement and creates a Transferable
    * wrapper around it to allow us to move the ConfigElement from one context
    * to another.
    */
   public ConfigElementSelection(ConfigElement elm)
   {
      mConfigElement = elm;
   }

   /**
    * Return the ConfigElement that we are transfering.
    */
   public Object getTransferData(DataFlavor flavor) 
      throws UnsupportedFlavorException
   {
      if (flavor.getRepresentationClass().equals(ConfigElement.class)) 
      {
         return mConfigElement;
      }
      else 
      {
         throw new UnsupportedFlavorException(flavor);
      }
   }

   /**
    * Return an array of valid DataFlavors that we can use to pass
    * ConfigElements from one context to another context.
    */
   public DataFlavor[] getTransferDataFlavors()
   {
      return flavors;
   }

   /**
    * Check if the given DataFlavor is a valid flavor for transfering
    * ConfigElements from one context to another.
    */
   public boolean isDataFlavorSupported(DataFlavor flavor) 
   {
      if (flavor.getRepresentationClass().equals(ConfigElement.class))
      {
         return true;
      }
      return false;
   }

   /**
    * Informs the ConfigContextEditor that it has lost the clipboard.
    */
   public void lostOwnership(Clipboard clip, Transferable obj)
   {;}
   
   /** ConfigElement that we are transfering. */
   private ConfigElement mConfigElement = null;

   /** Array of valid DataFlavors for transfering a ConfigElement. */
   private DataFlavor flavors[] = {new DataFlavor(ConfigElement.class,
                                                "VR Juggler Config Element")};
}

/**
 * Returns a component used to edit the name of the ConfigElement in the JTree.
 */
class ElementNameEditor implements TreeCellEditor
{
   /**
    * Constructor that takes a reference to the JTree that we will be editing.
    * This is required so that we can determine if the selected node is
    * editable.
    */
   public ElementNameEditor(JTree tree)
   {
      mTree = tree;
   }
   
   /**
    * Returns a component that will be used to edit the name of the
    * ConfigElement.
    */
   public Component getTreeCellEditorComponent(JTree tree,
                                            Object value,
                                            boolean isSelected,
                                            boolean expanded,
                                            boolean leaf,
                                            int row)
   {
      // Get a reference to the selected TreeNode.
      DefaultMutableTreeNode node = (DefaultMutableTreeNode)value;
     
      if(!(node.getUserObject() instanceof ConfigElement))
      {
         throw new IllegalArgumentException("Error: "
               + "The selected node contains an object with type:" 
               + node.getUserObject().getClass());
      }
     
      // Get a reference to the selected ConfigElement.
      mElement = (ConfigElement)node.getUserObject();
      mTextField = new JTextField(mElement.getName());
      mTree = tree;
     
      
      // Specify what should happen when done editing.
      mTextField.addActionListener(new ActionListener()
      {
         public void actionPerformed(ActionEvent evt)
         {
            // Force the focus to be lost.
            //mTextField.transferFocusUpCycle();
            // Force the focus to be transfered to the next component.
            //mTextField.transferFocus();
            
            // This is no longer needed since the above line will force a
            // focusLostEvent. But I have choosen to leave this line here in
            // case it might become useful later.
            stopCellEditing();
            mTree.clearSelection();
         }
      });
      
      mTextField.addFocusListener(new FocusAdapter()
      {
         public void focusLost(FocusEvent evt)
         {
            stopCellEditing();
         }
      });
      
      return mTextField;
   }
   public Object getCellEditorValue()
   {
      return mElement;
   }
   public boolean isCellEditable(EventObject evt)
   {
      TreePath current_selection = mTree.getSelectionPath();
      if (current_selection != null) 
      {
         DefaultMutableTreeNode current_node = (DefaultMutableTreeNode)
            (current_selection.getLastPathComponent());
         if (current_node != null)
         {
            if(current_node.getUserObject() instanceof ConfigElement)
            {
               return true;
            }
         }
      } 
      return false;
   }
   public boolean shouldSelectCell(EventObject evt)
   { return true; }
   public boolean stopCellEditing()
   {
      System.out.println("Stop Cell Editing...");
      mElement.setName(mTextField.getText());      
      return true; 
   }
   public void cancelCellEditing()
   {;}
   public void addCellEditorListener(CellEditorListener l)
   {;}
   public void removeCellEditorListener(CellEditorListener l)
   {;}

   private JTree           mTree       = null;
   private JTextField      mTextField  = null;
   private ConfigElement   mElement    = null;
}
/**
 * A JTree that allows us to transfer ConfigElements to/from other ElementTrees
 * in other contexts.
 */
class ElementTree extends JTree implements DragGestureListener, 
         DragSourceListener, DropTargetListener, ActionListener
{
   JPopupMenu popup;
   JMenuItem mi;
   
   Clipboard clipboard = getToolkit().getSystemClipboard(); 
   
   /**
    * Create a JTree that supports the dragging and dropping of ConfigElements.
    */
   public ElementTree()
   {
      super();

      // Allow the user to change the name of a leaf node(an element)
      setEditable(true);
     
      //setCellEditor(new DefaultTreeCellEditor(this, new DefaultTreeCellRenderer()));
      setCellEditor(new DefaultTreeCellEditor(this, new DefaultTreeCellRenderer(), new ElementNameEditor(this)));

      
      DragSource dragSource = DragSource.getDefaultDragSource();
      
      // Create a new drop target. We do not need to keep a reference to this
      // since internally it associates itself with the componenets that it
      // needs.
      new DropTarget(this, // Component to associate DropTarget with.
          DnDConstants.ACTION_COPY_OR_MOVE, // Default acceptable actions.
          this);  // Drop target listener.
      
      // Create a DragGestureRecognizer to watch for drags. Once again we do not
      // need to keep the reference that is returned from this method.
      dragSource.createDefaultDragGestureRecognizer(
            this, // component where drag originates
            DnDConstants.ACTION_COPY_OR_MOVE, // actions
            this); // drag gesture recognizer

      // Load the icons for the popup menu.
      ClassLoader loader = getClass().getClassLoader();
      ImageIcon cut_icon = null;
      ImageIcon copy_icon = null;
      ImageIcon paste_icon = null;
      ImageIcon remove_icon = null;
      
      try
      {
         cut_icon = new ImageIcon(loader.getResource("org/vrjuggler/jccl/editors/images/Cut16.gif"));
         copy_icon = new ImageIcon(loader.getResource("org/vrjuggler/jccl/editors/images/Copy16.gif"));
         paste_icon = new ImageIcon(loader.getResource("org/vrjuggler/jccl/editors/images/Paste16.gif"));
         remove_icon = new ImageIcon(loader.getResource("org/vrjuggler/jccl/editors/images/Delete16.gif"));
      }
      catch(Exception ex)
      {
         ex.printStackTrace();
      }
      
      // Set up the pop up menu
      popup = new JPopupMenu();
      mi = new JMenuItem("Delete", remove_icon);
      mi.addActionListener(this);
      mi.setActionCommand("delete");
      mi.setAccelerator(KeyStroke.getKeyStroke(KeyEvent.VK_DELETE, 0));
      popup.add(mi);
      
      mi = new JMenuItem("Rename");
      mi.addActionListener(this);
      mi.setActionCommand("rename");
      popup.add(mi);

      popup.addSeparator();

      mi = new JMenuItem("Cut", cut_icon);
      mi.addActionListener(this);
      mi.setActionCommand("cut");
      mi.setAccelerator(KeyStroke.getKeyStroke(KeyEvent.VK_X, InputEvent.CTRL_MASK));
      popup.add(mi);
      
      mi = new JMenuItem("Copy", copy_icon);
      mi.addActionListener(this);
      mi.setActionCommand("copy");
      mi.setAccelerator(KeyStroke.getKeyStroke(KeyEvent.VK_C, InputEvent.CTRL_MASK));
      popup.add(mi);

      mi = new JMenuItem("Paste", paste_icon);
      mi.addActionListener(this);
      mi.setActionCommand("paste");
      mi.setAccelerator(KeyStroke.getKeyStroke(KeyEvent.VK_V, InputEvent.CTRL_MASK));
      popup.add(mi);

      popup.setOpaque(true);
      popup.setLightWeightPopupEnabled(true);

      // Add mouse listener to get the popup menu events.
      addMouseListener(
         new MouseAdapter() 
         {
            // NOTE: XWindows and Windows sense popup trigger events at
            //       different times. On windows it occurs when the mouse is
            //       released, while on XWindows is occurs when the mouse button
            //       is pressed.
         
            // Check for a popup trigger on XWindows.
            public void mousePressed( MouseEvent e )
            {
               if ( e.isPopupTrigger()) 
               {
                  popup.show( (JComponent)e.getSource(), e.getX(), e.getY() );
               }         
            }
            // Check for a popup trigger on Windows.
            public void mouseReleased( MouseEvent e ) 
            {
               if ( e.isPopupTrigger()) 
               {
                  popup.show( (JComponent)e.getSource(), e.getX(), e.getY() );
               }
            } 
         });
      
      // Add key listener to catch the shortcuts for cut/copy/paste/delete
      addKeyListener(
            new KeyAdapter()
            {
               public void keyPressed(KeyEvent e)
               {
                  if(e.getKeyCode() == KeyEvent.VK_C && e.getModifiers() == ActionEvent.CTRL_MASK)
                  {
                     System.out.println("We have a copy.");
                     actionPerformed(new ActionEvent(this, 0, "copy"));
                  }
                  if(e.getKeyCode() == KeyEvent.VK_V && e.getModifiers() == ActionEvent.CTRL_MASK)
                  {
                     System.out.println("We have a paste.");
                     actionPerformed(new ActionEvent(this, 0, "paste"));
                  }
                  if(e.getKeyCode() == KeyEvent.VK_X && e.getModifiers() == ActionEvent.CTRL_MASK)
                  {
                     System.out.println("We have a cut.");
                     actionPerformed(new ActionEvent(this, 0, "cut"));
                  }
                  if(e.getKeyCode() == KeyEvent.VK_DELETE && e.getModifiers() == 0)
                  {
                     System.out.println("We have a delete.");
                     actionPerformed(new ActionEvent(this, 0, "delete"));
                  }
               }
            });
   }

   /**
    * Catch all events for the pop-up menu events.
    */
   public void actionPerformed(ActionEvent ae) 
   {
      // If the user selcted the remove menu item.
      if(ae.getActionCommand().equals("delete")) 
      {
         // Get the currently selected ConfigElement.
         TreePath path = this.getLeadSelectionPath();
         DefaultMutableTreeNode node = (DefaultMutableTreeNode)path.getLastPathComponent();
         Object temp = node.getUserObject();
         TreeModel model = this.getModel();
         
         // Remove the ConfigElement from the context.
         if(temp instanceof ConfigElement && model instanceof ConfigContextModel)
         {
            // Try to remove the element.
            ConfigBroker broker = new ConfigBrokerProxy();
            if(!broker.remove( ((ConfigContextModel)model).getContext(), (ConfigElement)temp))
            {
               System.out.println("Failed...");
            }
         }
      }

      // Start editing the currently selected ConfigElement if we want to rename it.
      if(ae.getActionCommand().equals("rename"))
      {
         // Get the currently selected ConfigElement.
         TreePath path = this.getLeadSelectionPath();
         startEditingAtPath(path);
      }

      // If the user selected the copy action.
      if(ae.getActionCommand().equals("copy")) 
      {
         // Get the currently selected ConfigElement.
         TreePath path = this.getLeadSelectionPath();
         DefaultMutableTreeNode node = (DefaultMutableTreeNode)path.getLastPathComponent();
         Object temp = node.getUserObject();
     
         // Place ConfigElement in the system clipboard.
         if(temp instanceof ConfigElement)
         {
            ConfigElementSelection selection = new ConfigElementSelection((ConfigElement)temp);
            // TODO: Change owner of clipboard.
            clipboard.setContents(selection, selection);
         }
      }

      // If the user selects the cut operation we should first copy it into the
      // clipboard and then delete it from the active context.
      if(ae.getActionCommand().equals("cut"))
      {
         ActionEvent evt;
         evt = new ActionEvent(this, -1, "copy");
         actionPerformed(evt);
         evt = new ActionEvent(this, -1, "delete");
         actionPerformed(evt);
      }

      // If the user selected the paste action.
      if(ae.getActionCommand().equals("paste")) 
      {
         try
         {
            // Get the context model for this JTree.
            TreeModel model = this.getModel();
            ConfigContextModel config_model = (ConfigContextModel)model;

            // Get the ConfigElement to paste out of the clipboard.
            DataFlavor my_flavor = new DataFlavor(ConfigElement.class, 
                     "VR Juggler Config Element");
            Transferable tr = clipboard.getContents(this);
            
            // If this JTree has a valid JTreeModel and the incoming paste supports
            // the DataFlavor that we are trying to use.
            if(model instanceof ConfigContextModel && tr.isDataFlavorSupported(my_flavor))
            {
               // Get the ConfigElement that we are transfering and make a deep
               // copy of it.
               ConfigElement old_elm = (ConfigElement)tr.getTransferData(my_flavor);
               ConfigElement elm = new ConfigElement(old_elm);

               // Make sure that we have a meaningful unique name.
               ConfigContext ctx = config_model.getContext();
               String base_name = elm.getName();
               int num = 1;
               while(ctx.containsElement(elm))
               {
                  String new_name = base_name + "Copy" + Integer.toString(num);
                  elm.setName(new_name);
                  ++num;
               }
               
               // Make sure that we are not trying to add an element to ourselves.
               ConfigBroker broker = new ConfigBrokerProxy();
                
               // Make sure this add goes through successfully
               if (!broker.add(config_model.getContext(), elm))
               {
                  throw new Exception("Could not paste ConfigElement into context.");
               }
               System.out.println("Paste completed...");
            }
         }
         catch(Exception ex)
         {
            System.out.println(ex);
            ex.printStackTrace();
         }
      }
   }
   
   /**
    * When the user attempts to drag an object out of the JTree we need to
    * prepare the associated transfer object.
    */
   public void dragGestureRecognized(DragGestureEvent e) 
   {     
      if(e.getDragAction() == DnDConstants.ACTION_COPY)
      {
         System.out.println("Copy...");
      }
      else if(e.getDragAction() == DnDConstants.ACTION_MOVE)
      {
         System.out.println("Move...");
      }
      else
      {
         System.out.println("Something else: " + e.getDragAction());
      }
      
      // Get the selected node the JTree.
      TreePath path = getLeadSelectionPath();
      DefaultMutableTreeNode node = (DefaultMutableTreeNode)path.getLastPathComponent();
      
      // Get the object associated with it, and if it is a ConfigElement
      // prepare it to be transfered.
      Object temp = node.getUserObject();
      if(null != temp && temp instanceof ConfigElement)
      {
         // Start the dragging action with the given icon, transferable object
         // and listener to listen for the dropEnd event that will only occur in
         // the case of a successful drag and drop.
         e.startDrag(DragSource.DefaultCopyDrop, // cursor
                     new ConfigElementSelection((ConfigElement)temp), // transferable
                     this);  // drag source listener
      }
   }

   /**
    * Take appropriate actions after a drop has ended.
    */
   public void dragDropEnd(DragSourceDropEvent e) 
   {
      if(e.getDropAction() == DnDConstants.ACTION_COPY)
      {
         System.out.println("Copy has occured...");
      }
      else if(e.getDropAction() == DnDConstants.ACTION_MOVE)
      {
         System.out.println("Move has occured...");
      }
      else
      {
         System.out.println("Something else: " + e.getDropAction() + " has occured.");
      }    
      // If the drag was successful and it was not a copy action, then default
      // to a move action and remove the ConfigElement from our context.
      if(e.getDropSuccess() && (e.getDropAction() != DnDConstants.ACTION_COPY))
      {
         TreeModel model = this.getModel();
         if(model instanceof ConfigContextModel)
         {
            DataFlavor my_flavor = new DataFlavor(ConfigElement.class, 
                     "VR Juggler Config Element");
            try
            {
               // Get the ConfigElement that we are trying to transfer.
               // NOTE: We could get the selected ConfigElement from the JTree,
               // but this might be more error prone than getting it this way.
               ConfigElement elm = (ConfigElement)e.getDragSourceContext().getTransferable().getTransferData(my_flavor);
            
               ConfigContextModel config_model = (ConfigContextModel)model;
               ConfigBroker broker = new ConfigBrokerProxy();
               
               // Make sure this add goes through successfully
               if (!broker.remove(config_model.getContext(), elm))
               {
                  throw new Exception("Failed to remove ConfigElement from context after a copy action.");
               }
            }
            catch(Exception ex)
            {
               System.out.println(ex);
               ex.printStackTrace();
            }
         }
      }
   }
   public void dragEnter(DragSourceDragEvent e) 
   {;}
   public void dragExit(DragSourceEvent e) 
   {;}
   public void dragOver(DragSourceDragEvent e) 
   {;}
   public void dropActionChanged(DragSourceDragEvent e) 
   {;}

   /**
    * Attept to handle the dropping of a ConfigElement into this JTree.
    */
   public void drop(DropTargetDropEvent e)
   {
      if(e.getDropAction() == DnDConstants.ACTION_COPY)
      {
         System.out.println("Dropping a Copy...");
      }
      else if(e.getDropAction() == DnDConstants.ACTION_MOVE)
      {
         System.out.println("Dropping a Move...");
      }
      else
      {
         System.out.println("Dropping Something else: " + e.getDropAction());
      } 
      
      try
      {
         TreeModel model = this.getModel();
         ConfigContextModel config_model = (ConfigContextModel)model;
         DataFlavor my_flavor = new DataFlavor(ConfigElement.class, 
                  "VR Juggler Config Element");
         Transferable tr = e.getTransferable();
         
         // If this JTree has a valid JTreeModel and the incoming drop supports
         // the DataFlavor that we are trying to use.
         if(model instanceof ConfigContextModel && e.isDataFlavorSupported(my_flavor))
         {
            // Get the ConfigElement that we are transfering.
            ConfigElement elm = (ConfigElement)tr.getTransferData(my_flavor);

            // Make sure that we are not trying to add an element to ourselves.
            ConfigBroker broker = new ConfigBrokerProxy();
            java.util.List data_sources = config_model.getContext().getResources();
            for(Iterator itr = data_sources.iterator() ; itr.hasNext() ; )
            {
               java.util.List elm_list = broker.getElementsIn((String)itr.next());
               if(elm_list.contains(elm))
               {
                  System.out.println("You can not drag an Element into a "
                                   + "context in which it already exists.");
                  e.rejectDrop();
                  return;
               }
            }
            
            // Accept whatever action was sent to us.
            e.acceptDrop(e.getDropAction());
            
            // Make sure this add goes through successfully
            if (!broker.add(config_model.getContext(), elm))
            {
               e.rejectDrop();
               throw new Exception("Could not add dropped ConfigElement into context.");
            }
            System.out.println("Drop completed...");
            e.dropComplete(true);
         }
         else
         {
            e.rejectDrop();
         }
      }
      catch(Exception ex)
      {
         System.out.println(ex);
         ex.printStackTrace();
      }
   }
   
   public void dragEnter(DropTargetDragEvent e) 
   {;}
   public void dragExit(DropTargetEvent e) 
   {;}
   public void dragOver(DropTargetDragEvent e) 
   {;}
   public void dropActionChanged(DropTargetDragEvent e) 
   {;}
}
