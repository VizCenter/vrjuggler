/*************** <auto-copyright.pl BEGIN do not edit this line> **************
 *
 * VR Juggler is (C) Copyright 1998, 1999, 2000 by Iowa State University
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



package VjComponents.ConfigEditor;

import java.awt.*;
import java.awt.event.*;
import javax.swing.*;
import javax.swing.tree.*;
import javax.swing.border.*;
import javax.swing.event.*;
import java.util.Enumeration;
import java.io.File;

import VjControl.*;
import VjComponents.UI.Widgets.*;
import VjConfig.*;
import VjComponents.UI.PlugPanel;
import VjComponents.ConfigEditor.ConfigModule;
import VjComponents.ConfigEditor.ConfigModuleEvent;
import VjComponents.ConfigEditor.ConfigModuleListener;
import VjComponents.ConfigEditor.ChunkOrgTreeModule;
import VjComponents.UI.ControlUIModule;

/* there are some big issues with this piece of code.
 * handling of orgtrees is pathologically stupid in some ways 
 * (do we need to have the orgtree member at all?)
 */

/** Panel for editing ChunkOrgTrees.
 *  This panel can only handle a single tree; by default the one actually
 *  in use.  It can make its current tree the active one, but needs a
 *  "revert" button or something to recapture the active ChunkOrgTree and
 *  cancel changes.
 * 
 *  @author Christopher Just
 *  @version $Revision$
 */
public class ChunkOrgTreePane 
    extends JPanel 
    implements PlugPanel, 
               ActionListener, 
               CellEditorListener, 
               MouseListener, 
               ConfigModuleListener {

    protected ConfigModule config_module;
    protected ChunkOrgTreeModule orgtree_module;
    protected ControlUIModule ui_module;
    protected String component_name;
    protected ConfigChunk component_chunk;

    protected SuffixFilter orgtree_filter;

    private JButton   new_button;
    private JButton   load_button;
    private JButton   save_button;
    private JButton   apply_button;

    private JButton   remove_button;
    private JButton   insertfolder_button;
    private JButton   insertdescname_button;

    private TreePath treeitem_menu_path;
    private JMenuItem addfolder_mi, remove_mi, cremove_mi, unnameddesc_mi;
    EasyPopupMenu treeitem_menu, childlesstreeitem_menu;

    ChunkOrgTree orgtree;

    JScrollPane scroll_pane;
    JTree     tree;
    DefaultTreeModel treemodel;
    DefaultMutableTreeNode root;
    boolean ignore_next_reload; // used so we don't reload tree as a result
                                // of message sent because of an apply
                                // ie. we already knew that.


    public ChunkOrgTreePane () {
        super();

        component_name = "OrgTree";
        component_chunk = null;

        JPanel south_panel;
        JPanel side_panel;
        Box center_panel;

	ignore_next_reload = false;

        config_module = null;
        orgtree_module = null;
        ui_module = null;


	// popup menu for tree
	treeitem_menu_path = null;
        treeitem_menu = new EasyPopupMenu("foo", 25);
        addfolder_mi = 
            treeitem_menu.addMenuItem ("Add Folder");
        unnameddesc_mi =
            treeitem_menu.addMenuItem ("Add ChunkDesc/Unnamed");
        remove_mi = 
            treeitem_menu.addMenuItem ("Remove");

        childlesstreeitem_menu = new EasyPopupMenu ("foo", 25);
        cremove_mi =
            childlesstreeitem_menu.addMenuItem ("Remove");


        setBorder (new EmptyBorder (5,5,5,5));
        setLayout (new BorderLayout (5, 5));

	root = new DefaultMutableTreeNode ("nothing", false);
	treemodel = new DefaultTreeModel(root, true);
	tree = new JTree (treemodel);

        center_panel = new Box (BoxLayout.Y_AXIS);
	scroll_pane = new JScrollPane (tree, JScrollPane.VERTICAL_SCROLLBAR_ALWAYS,
                                       JScrollPane.HORIZONTAL_SCROLLBAR_AS_NEEDED);
	center_panel.add (scroll_pane);
        add (center_panel, "Center");

	orgtree = new ChunkOrgTree();

	JTextField editfield = new JTextField();
	DefaultCellEditor editor = new DefaultCellEditor (editfield);
	tree.setEditable (true);
	tree.setCellEditor (editor);
	editor.addCellEditorListener (this);

	// Side Panel
	side_panel = new JPanel();
	side_panel.setLayout (new GridLayout (9, 1));

        //ImageIcon new_icn, load_icn, save_icn, close_icn;
	
	load_button = new JButton ("Load");
	save_button = new JButton ("Save");
	new_button = new JButton ("New");
	//close_button = new JButton ("Close");

        Insets ins = new Insets (0,0,0,0);
        new_button.setMargin (ins);
        load_button.setMargin (ins);
        save_button.setMargin (ins);
        //close_button.setMargin (ins);
 
	apply_button = new JButton ("Apply");
	remove_button = new JButton ("Remove");

	side_panel.add (new JLabel ("File"));
	side_panel.add (new_button);
	side_panel.add (load_button);
	side_panel.add (save_button);
	side_panel.add (apply_button);
	side_panel.add (new JLabel ("Element"));
	side_panel.add (remove_button);
	add (side_panel, "East");

	new_button.addActionListener (this);
	load_button.addActionListener (this);
	save_button.addActionListener (this);
	apply_button.addActionListener (this);
	remove_button.addActionListener (this);
	tree.addMouseListener (this);

	addfolder_mi.addActionListener (this);
	remove_mi.addActionListener (this);
	unnameddesc_mi.addActionListener (this);
	cremove_mi.addActionListener (this);

    }



    private void updateInsertTypes () {
	int i;
	ChunkDesc d;
	JMenuItem mi;
//  	addchunkdesc_menu.removeAll();
//  	addchunkdesc_menu.add (unnameddesc_mi);
//  	addchunkdesc_menu.addSeparator();

        String desc_names[] = config_module.getDescNames();
	for (i = 0; i < desc_names.length; i++) {
            mi = treeitem_menu.addMenuItem ("Add ChunkDesc/" + desc_names[i]);
            mi.addActionListener (this);
            mi = childlesstreeitem_menu.addMenuItem ("Add ChunkDesc/" + desc_names[i]);
            mi.addActionListener (this);
	}
    }



    public void buildTree (ChunkOrgTree orgtree) {
	root = buildTreeHelper (orgtree.root);
	treemodel.setRoot (root);
	treemodel.reload();
    }



    private DefaultMutableTreeNode buildTreeHelper (Object orgtree) {
	OrgTreeElem ot;
	int i;
	DefaultMutableTreeNode n1, n2;
	if (orgtree instanceof String) {
	    n1 = new DefaultMutableTreeNode ((String)orgtree, 
					     false);
	    return n1;
	}
	else if (orgtree instanceof OrgTreeElem) {
	    ot = (OrgTreeElem)orgtree;
	    n1 = new DefaultMutableTreeNode (ot.label, 
					     true);
	    for (i = 0; i < ot.children.size(); i++)
		n1.add (buildTreeHelper (ot.children.elementAt(i)));
	    return n1;
	}
	else
	    return null;
    }
	


    public void setCOTValue (ChunkOrgTree ot) {
	// replaces the old value of ot w/ the value defined by the panel's tree
	ignore_next_reload = true;
	ot.setRoot ((OrgTreeElem)setCOTValueHelper (root));
    }
   
    protected Object setCOTValueHelper (DefaultMutableTreeNode n) {
	DefaultMutableTreeNode n1, n2;
	OrgTreeElem ot;
	Enumeration e;
	if (n.getAllowsChildren() == false) {
	    return n.toString();
	}
	else {
	    ot = new OrgTreeElem(n.toString());
	    e = n.children();
	    while (e.hasMoreElements()) {
		ot.children.addElement (setCOTValueHelper ((DefaultMutableTreeNode)e.nextElement()));
	    }
	    return ot;
	}
    }



    public void actionPerformed (ActionEvent e) {
	ChunkOrgTree ot;
	Object source = e.getSource();
	if (e.getActionCommand().equalsIgnoreCase ("reload")) {
	    if (ignore_next_reload)
		ignore_next_reload = false;
	    else {
		orgtree.copyValueFrom (orgtree_module.getOrgTree());
		buildTree (orgtree);
	    }
	}
	else if (source == new_button) {
	    //System.out.println ("pushed new");
	    orgtree.copyValueFrom (new ChunkOrgTree());
	    buildTree (orgtree);
	}
	else if (source == save_button) {
	    setCOTValue (orgtree);
            File f = ui_module.getEasyFileDialog().requestSaveFile (orgtree.getFile(), ui_module, orgtree_filter);
            if (f != null)
                orgtree_module.saveChunkOrgTree (orgtree, f);
	}
	else if (source == load_button) {
            File f = ui_module.getEasyFileDialog().requestOpenFile (orgtree.getFile().getParentFile(), ui_module, orgtree_filter);
            if (f != null) {
                orgtree_module.loadChunkOrgTree (orgtree, f); 
                buildTree (orgtree);
            }
	}
	else if (source == apply_button) {
	    setCOTValue (orgtree_module.getOrgTree());
	}
	else if (source == remove_button) {
	    //System.out.println ("Remove button pressed");
	    TreePath[] tp = tree.getSelectionPaths();
	    if (tp == null)
		return;
	    tree.removeSelectionPaths (tp);
	    DefaultMutableTreeNode n, n1;
	    for (int i = 0; i < tp.length; i++) {
		n = (DefaultMutableTreeNode)tp[i].getLastPathComponent();
		n1 = (DefaultMutableTreeNode)n.getParent();
		n.removeFromParent();
		treemodel.reload(n1);
	    }
	}
	else if ((source == remove_mi) || (source == cremove_mi)) {
	    DefaultMutableTreeNode n, n1;
	    n = (DefaultMutableTreeNode)treeitem_menu_path.getLastPathComponent();
	    n1 = (DefaultMutableTreeNode)n.getParent();
	    n.removeFromParent();
	    treemodel.reload(n1);
	}
	else if (source == addfolder_mi) {
	    DefaultMutableTreeNode n = (DefaultMutableTreeNode)treeitem_menu_path.getLastPathComponent();
	    if (n != null && n.getAllowsChildren()) {
		n.add (new DefaultMutableTreeNode ("New Folder", true));
		treemodel.reload(n);
	    }
	}
	else if (e.getSource() instanceof JMenuItem) {
	    /* well, it's got to be something in the chunkdesc menu */
	    DefaultMutableTreeNode n = (DefaultMutableTreeNode)treeitem_menu_path.getLastPathComponent();
	    if (n != null && n.getAllowsChildren()) {
		n.add (new DefaultMutableTreeNode (e.getActionCommand(), false));
		treemodel.reload(n);
	    }

	}
    }


    public void mouseClicked(MouseEvent e) {}
    public void mouseEntered(MouseEvent e) {}
    public void mouseExited(MouseEvent e) {}
    public void mouseReleased(MouseEvent e) {}

    public void mousePressed(MouseEvent e) {
	int mod = e.getModifiers();
	if ((mod == MouseEvent.BUTTON2_MASK) || (mod == MouseEvent.BUTTON3_MASK)) {
	    //System.out.println ("RightButtonClick");
	    treeitem_menu_path = tree.getPathForLocation (e.getX(), e.getY());
	    if (treeitem_menu_path != null) {
		DefaultMutableTreeNode n = (DefaultMutableTreeNode)treeitem_menu_path.getLastPathComponent();
		if (n.getAllowsChildren())
		    treeitem_menu.getJPopupMenu().show (tree, e.getX(), e.getY());
		else
		    childlesstreeitem_menu.getJPopupMenu().show (tree, e.getX(), e.getY());
	    }
	}
    }

 

    public void editingCanceled (ChangeEvent e) {
    }

    public void editingStopped (ChangeEvent e) {
	//System.out.println ("Edit event :" + e + ", from " + e.getSource());
    }



    // ConfigModuleListener stuff

    public void addChunkDB (ConfigModuleEvent e) {
    }

    public void removeChunkDB (ConfigModuleEvent e) {
    }

    public void addDescDB (ConfigModuleEvent e) {
	updateInsertTypes();
    }

    public void removeDescDB (ConfigModuleEvent e) {
	updateInsertTypes();
    }


    /********************** PlugPanel Stuff **************************/

    public String getComponentName () {
        return component_name;
    }


    public ImageIcon getComponentIcon () {
        return null;
    }


    public boolean configure (ConfigChunk ch) {
        component_chunk = ch;
        component_name = ch.getName();

        // get pointers to the modules we need.
        Property p = ch.getPropertyFromToken ("Dependencies");
        if (p != null) {
            int i;
            int n = p.getNum();
            String s;
            VjComponent c;
            for (i = 0; i < n; i++) {
                s = p.getValue(i).toString();
                c = Core.getComponentFromRegistry(s);
                if (c != null) {
                    if (c instanceof ControlUIModule)
                        ui_module = (ControlUIModule)c;
                    if (c instanceof ConfigModule)
                        config_module = (ConfigModule)c;
                    if (c instanceof ChunkOrgTreeModule)
                        orgtree_module = (ChunkOrgTreeModule)c;
                }
            }
        }
        if (ui_module == null)
            ui_module = (ControlUIModule)Core.getComponentFromRegistry ("ControlUI Module");
        if (config_module == null)
            config_module = (ConfigModule)Core.getComponentFromRegistry ("Config Module");
        if (orgtree_module == null)
            orgtree_module = (ChunkOrgTreeModule)Core.getComponentFromRegistry ("ChunkOrgTree Module");
        if ((ui_module == null) || (config_module == null) || (orgtree_module == null)) {
            Core.consoleErrorMessage (component_name, "Instantiated with unmet VjComponent Dependencies. Fatal Configuration Error!");
            return false;
        }

        // add our filter to the UI module
        orgtree_filter = new SuffixFilter ("Chunk Org Tree Files (*.org)", ".org");
        ui_module.getEasyFileDialog().addFilter (orgtree_filter, "ConfigOrgTree");

        // set up initial info from config module
	config_module.addConfigModuleListener (this);
        updateInsertTypes();

        // set up initial org tree
	orgtree_module.getOrgTree().addActionListener (this);
	orgtree.copyValueFrom (orgtree_module.getOrgTree());
	//orgtree.addActionListener (this);
	//orgtree = chunkorgtree;
	buildTree (orgtree/*_module.getOrgTree()*/);

        // this is fairly horrific. do all this button stuff here just cuz we
        // can't get the icons inside the constructor
        load_button.setIcon (ui_module.getIcon ("open file", 0));
        save_button.setIcon (ui_module.getIcon ("save file", 0));
        new_button.setIcon (ui_module.getIcon ("new file", 0));

        return true;
    }


    public ConfigChunk getConfiguration () {
        return component_chunk;
    }


    public boolean addConfig (ConfigChunk ch) {
        return false;
    }


    public boolean removeConfig (String name) {
        return false;
    }


    public void destroy () {
        ;
    }


    public void rebuildDisplay () {
    }


}








