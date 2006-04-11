package VjGUI;

import java.io.*;
import java.awt.*;
import java.awt.event.*;
import javax.swing.*;
import javax.swing.tree.*;
import javax.swing.event.*;
import VjConfig.*;
import VjGUI.*;
import VjGUI.configchunk.*;

public class ConfigurePane extends JSplitPane implements MouseListener /*TreeSelectionListener*/ {

  ChunkDBPanel leftpanel, rightpanel;


  public ConfigurePane() {
    super(JSplitPane.HORIZONTAL_SPLIT);

    //------------------------ BUILD CONFIGURATION PANE ------------------------

    setLeftComponent (leftpanel = new ChunkDBPanel(0));
    setRightComponent (rightpanel = new ChunkDBPanel(1));

    leftpanel.setMinimumSize (new Dimension (20, 50));
    rightpanel.setMinimumSize (new Dimension (20, 50));
    //setDividerLocation (.5);

    leftpanel.addMouseListener (leftpanel);
    rightpanel.addMouseListener (rightpanel);

  }



    public void sendAcross (ConfigChunkDB db, ChunkDBPanel panel) {
	ChunkDBPanel destpanel;

	if (db == null)
	    return;
	if (panel == leftpanel)
	    destpanel = rightpanel;
	else
	    destpanel = leftpanel;
	if (destpanel.current_treemodel == Core.active_treemodel)
	    Core.net.sendChunks(db);
	else
	    destpanel.addChunks(db);
    }



    public void updateInsertTypes() {
	leftpanel.updateInsertTypes();
	rightpanel.updateInsertTypes();
    }



    public void removeChunkDBTree (ChunkDBTreeModel dbt) {
	leftpanel.removeChunkDBTree (dbt);
	rightpanel.removeChunkDBTree (dbt);
    }


    public void addChunkDBTree (ChunkDBTreeModel dbt) {
	leftpanel.addChunkDBTree (dbt);
	rightpanel.addChunkDBTree (dbt);
    }


  /* TreeSelectionListener stuff */
//   public void valueChanged (TreeSelectionEvent e) {
//     ChunkTreeNodeInfo nodeinfo;
//     System.out.println ("Tree Selection Changed");
//     TreePath[] tp = ((JTree)e.getSource()).getSelectionPaths();
//     for (int i = 0; i < tp.length; i++) {
//       System.out.println ("  " + tp[i]);
//       nodeinfo = ((ChunkTreeNodeInfo)((DefaultMutableTreeNode)tp[i].getLastPathComponent()).getUserObject());
//       if (nodeinfo.ch != null)
// 	System.out.println ("    " + nodeinfo.ch);
//     }

//   }


  public void selectLeftDB (String name) {
    leftpanel.selectDB (name);
  }
  public void selectRightDB (String name) {
    rightpanel.selectDB (name);
  }



  public void mouseClicked(MouseEvent e) {
      if (e.getSource() == leftpanel)
	  System.out.println ("left panel click");
      else
	  System.out.println ("right panel click");
  }


  public void mouseEntered(MouseEvent e) {}
  public void mouseExited(MouseEvent e) {}
  public void mousePressed(MouseEvent e) {}
  public void mouseReleased(MouseEvent e) {}

}



