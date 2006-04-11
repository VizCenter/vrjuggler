/*
 * VRJuggler
 *   Copyright (C) 1997,1998,1999,2000
 *   Iowa State University Research Foundation, Inc.
 *   All Rights Reserved
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
 */




/*
 * PerfAnalyzerPanel
 *
 * This panel is connected to a PerfDataCollection object.
 * It will include a chooser to select which data set in the collection
 * to show, a scrollpane for displaying text or graphics, and some other
 * gadgest to control what gets displayed.
 *
 */

package VjPerf;

import java.awt.*;
import java.awt.event.*;
import javax.swing.*;
import javax.swing.border.EmptyBorder;
import java.util.Vector;
import VjPerf.*;
import VjGUI.FileControl;
import VjGUI.util.*;
import VjGUI.Core;

public class PerfAnalyzerPanel extends JPanel implements ActionListener, JFrameParent {


    private class CollectorSummaryButton extends JButton {
	public PerfDataCollector collector;

	public CollectorSummaryButton (PerfDataCollector _collector) {
	    super ("Summary");
	    collector = _collector;
	}
    }


    private class AnomaliesButton extends JButton {
	public PerfDataCollector collector;
	public int phase;

	public AnomaliesButton (PerfDataCollector _collector, int _phase) {
	    super ("Anomalies");
	    collector = _collector;
	    phase = _phase;
	}
    }


    private class GraphButton extends JButton {
	public PerfDataCollector collector;
	public int phase;

	public GraphButton (PerfDataCollector _collector, int _phase) {
	    super ("Graph");
	    collector = _collector;
	    phase = _phase;
	}
    }


    private String padFloat (double f) {
	// reformats f to a string w/ 3 places after decimal
	String s = Double.toString(f);
	if (s.indexOf('E') != -1)
	    return s;
	int i = s.lastIndexOf('.');
	if ((i >= 0) && (i+5 < s.length()))
	    s = s.substring (0, i + 5);
	return s;
    }

    public class DataPanelElem {
	public PerfDataCollector col;
	public JLabel sep_label;
	public JLabel colname_label;
	public CollectorSummaryButton colsummary_button;
	public JLabel avgs_label;
	public JLabel[] phase_labels;
	public JLabel[] avg_labels;
	public GraphButton[] graph_buttons;
	public AnomaliesButton[] anomalies_buttons;
	public int num;

	public DataPanelElem (PerfDataCollector _col, JPanel panel, 
			      GridBagLayout gblayout, GridBagConstraints gbc) {
	    Insets insets = new Insets (1,1,1,1);
	    col = _col;
	    num = col.getNumPhases();
	    phase_labels = new JLabel[num];
	    avg_labels = new JLabel[num];
	    graph_buttons = new GraphButton[num];
	    anomalies_buttons = new AnomaliesButton[num];

	    col.generateAverages(preskip, postskip);
	    gbc.gridwidth = gbc.REMAINDER;
	    sep_label = new JLabel ("----------------------------------------------");
	    gblayout.setConstraints (sep_label, gbc);
	    panel.add (sep_label);

	    colname_label = new JLabel (col.getName());
	    gbc.gridwidth = 1;
	    gblayout.setConstraints (colname_label, gbc);
	    panel.add (colname_label);

	    colsummary_button = new CollectorSummaryButton (col);
	    colsummary_button.addActionListener (PerfAnalyzerPanel.this);
	    gbc.gridwidth = gbc.REMAINDER;
	    gblayout.setConstraints (colsummary_button, gbc);
	    panel.add (colsummary_button);

	    avgs_label = new JLabel ("Average times in milliseconds:");
	    gblayout.setConstraints(avgs_label, gbc);
	    panel.add (avgs_label);

	    JLabel l;
	    JButton b;
	    for (int j = 0; j < num; j++) {
		double avg = col.getAverageForPhase(j);
		// this below will cause trouble
		if (avg == 0.0)
		    continue;
		gbc.gridwidth = 1;
		l = phase_labels[j] = new JLabel (j + ": " + col.getLabelForPhase(j));
		gblayout.setConstraints (l, gbc);
		panel.add(l);
		l = avg_labels[j] = new JLabel (padFloat(avg/1000.0), JLabel.RIGHT);
		gblayout.setConstraints (l, gbc);
		panel.add(l);
		b = graph_buttons[j] = new GraphButton (col, j);
		b.addActionListener (PerfAnalyzerPanel.this);
		b.setMargin(insets);
		gblayout.setConstraints (b, gbc);
		panel.add(b);
		b = anomalies_buttons[j] = new AnomaliesButton (col, j);
		b.setEnabled(false);
		b.addActionListener (PerfAnalyzerPanel.this);
		b.setMargin(insets);
		gbc.gridwidth = gbc.REMAINDER;
		gblayout.setConstraints (b, gbc);
		panel.add(b);
	    }
	}

	public void update() {
	    col.generateAverages(preskip, postskip);
	    for (int i = 0; i < num; i++) {
		double avg = col.getAverageForPhase(i);
		if (avg_labels[i] != null)
		    avg_labels[i].setText(padFloat(avg/1000.0));
		else
		    ; // we should probably add something in this case...
	    }
	}
    }

    private void refreshDataPanel () {
	int i,j;
	PerfDataCollector col;
	double avg;
	//data_panel.removeAll();
	DataPanelElem dpe;

	for (i = 0; i < collection.getSize(); i++) {
	    col = collection.getCollector(i);
	    dpe = getDataPanelElem (col);
	    if (dpe == null) {
		addDataPanelElem (col);
	    }
	    else
		dpe.update();
	}
    }

    public DataPanelElem getDataPanelElem (PerfDataCollector col) {
	DataPanelElem dpe;
	for (int i = 0; i < datapanel_elems.size(); i++) {
	    dpe = (DataPanelElem)datapanel_elems.elementAt(i);
	    if (col == dpe.col)
		return dpe;
	}
	return null;
    }

    public void addDataPanelElem (PerfDataCollector col) {
	DataPanelElem dpe = new DataPanelElem (col, data_panel, gblayout, gbc);
	datapanel_elems.addElement(dpe);
    }


//    JComboBox  collection_choice;
    JComboBox  display_choice;
    JComboBox  max_samples_box;
    JTextField preskip_box;
    JTextField postskip_box;
    JButton savecontents_button;
    JButton load_button;
    JButton print_button;
    JButton print_all_button;
    JPanel data_panel;
    GridBagLayout gblayout;
    GridBagConstraints gbc;

    JScrollPane display_pane;
    JTextArea text_area;

    PerfDataCollector current_collector;
    PerfDataCollection collection;

    Vector child_frames;  // vector of GenericGraphFrames...
    Vector datapanel_elems;

    int preskip, postskip;
    float anomalycutoff;
    boolean doanomaly;

    public PerfAnalyzerPanel () {
	super();

	child_frames = new Vector();
	datapanel_elems = new Vector();
	collection = Core.perf_collection;
	current_collector = null;
	text_area = null;

	collection.addActionListener (this);

	preskip = 20;
	postskip = 20;
	doanomaly = true;
	anomalycutoff = 1.0f;

	setLayout (new BorderLayout (5, 5));
	setBorder (new EmptyBorder (5,5,5,5));

	data_panel = new JPanel();
	data_panel.setLayout (gblayout = new GridBagLayout());
	gbc = new GridBagConstraints();
	gbc.insets = new Insets (0, 4, 0, 4);
	gbc.fill = gbc.HORIZONTAL;


	JPanel epanel = new JPanel ();
	epanel.setLayout (new GridLayout (10, 1, 2, 0));
	add (epanel, "East");

	load_button = new JButton ("Load");
	epanel.add (load_button);
	load_button.addActionListener (this);

	epanel.add (savecontents_button = new JButton ("Save Data"));
	savecontents_button.addActionListener (this);

//  	print_button = new JButton ("Print");
//  	print_button.addActionListener (this);
//  	epanel.add (print_button);

	print_all_button = new JButton ("Print");
	print_all_button.addActionListener (this);
	epanel.add (print_all_button);
       

	JPanel npanel = new JPanel ();
	npanel.setLayout (new BoxLayout (npanel, BoxLayout.Y_AXIS));
	add (npanel, "North");
	JPanel ntoppanel = new JPanel();
	ntoppanel.setLayout (new BoxLayout (ntoppanel, BoxLayout.X_AXIS));
	npanel.add (ntoppanel);

	ntoppanel.add (new JLabel ("Maximum stored samples"));
	ntoppanel.add (max_samples_box = new JComboBox());
	max_samples_box.addItem ("100");
	max_samples_box.addItem ("500");
	max_samples_box.addItem ("1000");
	max_samples_box.addItem ("5000");
	max_samples_box.addItem ("<Infinite>");
	max_samples_box.setSelectedIndex(1);
	max_samples_box.addActionListener (this);

	display_pane = new JScrollPane (data_panel, JScrollPane.VERTICAL_SCROLLBAR_AS_NEEDED, JScrollPane.HORIZONTAL_SCROLLBAR_AS_NEEDED);

	//setCurrentCollector (null);

	add (display_pane, "Center");

    }



//      public void setCurrentCollector (PerfDataCollector c) {
//  	if (c == null) {
//  	    current_collector = null;
//  	    savecontents_button.setEnabled (false);
//  	    //print_button.setEnabled (false);
//  	    //print_all_button.setEnabled (false);
//  	}
//  	else {
//  	    current_collector = c;
//  	    savecontents_button.setEnabled (true);
//  	    //print_button.setEnabled (true);
//  	    print_all_button.setEnabled (true);
//  	}
//      }



    public void actionPerformed (ActionEvent e) {
	if (e.getSource() instanceof AnomaliesButton) {
	    System.out.println ("not implemented");
	}
	else if (e.getSource() instanceof GraphButton) {
	    GraphButton b = (GraphButton)e.getSource();
	    GenericGraphPanel gp = new SummaryGraphPanel (b.collector, b.phase);
	    GenericGraphFrame gf = new GenericGraphFrame (gp, "Graph of " + b.collector.getName() + " phase " + b.phase, this);
	    child_frames.addElement(gf);
	    gf.show();

	}
	else if (e.getSource() instanceof CollectorSummaryButton) {
	    PerfDataCollector col = ((CollectorSummaryButton)e.getSource()).collector;
	    GenericGraphPanel gp = new SummaryGraphPanel (col);
	    GenericGraphFrame gf = new GenericGraphFrame (gp, "Summary Graph of " + col.getName() , this);
	    child_frames.addElement(gf);
	    gf.show();
	}
	else if (e.getSource() == max_samples_box) {
	    int numsamps;
	    String s = (String)max_samples_box.getSelectedItem();
	    if (s.equalsIgnoreCase("<Infinite>"))
		numsamps = -1;
	    else
		numsamps = Integer.parseInt(s);
	    //System.out.println ("setting max samples to " + numsamps);
	    collection.setMaxSamples(numsamps);
	}
	else if (e.getSource() == display_choice)
	    refreshDisplay();
//  	else if (e.getSource() == collection_choice) {
//  	    Object o = collection_choice.getSelectedItem();
//  	    if (o instanceof String)
//  		setCurrentCollector (null);
//  	    else 
//  		setCurrentCollector ((PerfDataCollector)collection_choice.getSelectedItem());
//  	    //current_collector = (PerfDataCollector)collection_choice.getSelectedItem();
//  	    //refreshDisplay();
//  	}
	else if (e.getSource() == preskip_box) {
	    preskip = Integer.parseInt (preskip_box.getText().trim());
	    refreshDisplay();
	}
	else if (e.getSource() == postskip_box) {
	    postskip = Integer.parseInt (postskip_box.getText().trim());
	    refreshDisplay();
	}
	else if (e.getSource() == savecontents_button) {
	    FileControl.savePerfDataFile (collection);
	}
	else if (e.getSource() == load_button) {
	    String name = FileControl.loadNewPerfDataFile ("", true);
	    refreshDisplay();
	}
	else if (e.getSource() == print_button) {
	     String s = current_collector.dumpAverages (preskip, postskip, true, anomalycutoff);
	     System.out.println (s);
	}
	else if (e.getSource() == print_all_button) {
	    String s = "";
	    PerfDataCollector col;
	    for (int i = 0; i < collection.getSize(); i++) {
		col = collection.getCollector(i);
		s += col.dumpAverages (preskip, postskip, false, anomalycutoff);
	    }
	    System.out.println (s);
	}
	else if (e.getSource() == collection) {
	    String s = e.getActionCommand();
	    if (s.equalsIgnoreCase ("update"))
		refreshDisplay();
	    else if (s.equalsIgnoreCase ("removealldata"))
		removeAllData();
	}
    }


    public void refreshDisplay() {
	for (int i = 0; i < child_frames.size(); i++)
	    ((GenericGraphFrame)child_frames.elementAt(i)).refresh();
	refreshDataPanel();
    }

    public void removeAllData() {
	for (int i = 0; i < child_frames.size(); i++)
	    ((GenericGraphFrame)child_frames.elementAt(i)).dispose();
	child_frames.removeAllElements();
	refreshDataPanel();
    }



    public void refresh() {
	refreshDisplay();
    }


    public void closedChild (ChildFrame f, boolean ok) {
	child_frames.removeElement(f);
	f.destroy();
    }


    /********************** PlugPanel Stuff **************************/
    public void destroy () {
    }

    public String getName () {
        return "Performance";
    }


}



