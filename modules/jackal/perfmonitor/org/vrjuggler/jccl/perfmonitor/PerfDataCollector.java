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



package VjComponents.PerfMonitor;

import java.awt.event.*;
import java.io.*;
import java.util.*;

import VjComponents.PerfMonitor.DataLine;
import VjConfig.*;
import VjControl.Core;
import VjConfig.ConfigStreamTokenizer;

/** Class for storing performance data from a single source.
 *  A source is, for example, a jccl::PerfDataBuffer.
 *
 *  PerfDataCollector is a source for ActionEvents, with the following
 *  ActionCommands:
 *      "Update" - when data is appended to the Collector.
 *      "Clear" - when the Collector clears out all its data.
 */
public class PerfDataCollector {
    // stores all performance data coming from a particular buffer
    public String name;
    public List datalines; // vector of DataLines & MissingCounts
    public int num; // # of different indices used for datapoints.
    public int numsamps[];
    public double totalsum;
    public int totalsamps;
    public double maxvals[];
    public double maxlinetotal;
    public double sums[];
    int prevplace = -1;
    double prevval = 0.0;
    DataLine dl = null;
    int place;
    ConfigChunk infochunk;
    int maxdatalines;
    protected List action_listeners;

    public PerfDataCollector(String _name, int _num, int maxsamples) {
        datalines = new LinkedList();
        action_listeners = new ArrayList();
	name = _name;
	num = _num;
	numsamps = new int[num];
	maxvals = new double[num];
	maxlinetotal = 0.0;
	sums = new double[num];
	totalsum = 0.0;
	totalsamps = 0;
	for (int i = 0; i < num; i++) {
	    numsamps[i] = 0;
	    maxvals[i] = 0.0;
	    sums[i] = 0.0;
	}
	dl = new DataLine (num);
	place = 0;
	maxdatalines = maxsamples;
	System.out.println ("creating PerfDataCollector " + _name + " with " + _num + " elements.");

        List infochunks = Core.vjcontrol_chunkdb.getOfDescToken ("PerfData");
        infochunk = null;
        ConfigChunk ch;
        for (int j = 0; j < infochunks.size(); j++) {
            ch = (ConfigChunk)infochunks.get(j);
            if (name.startsWith (ch.getName())) {
                infochunk = ch;
                break;
            }
        }

    }


    public void write (DataOutputStream out) throws IOException {
	out.writeBytes ("PerfData1 \"" + name + "\" " + num + "\n");
	ListIterator li = datalines.listIterator(0);
	while (li.hasNext()) {
	    DataLine dl = (DataLine)li.next();
	    int i = 0;
	    while ((i < num) && Double.isNaN(dl.vals[i]))
		i++;
	    while ((i < num) && !Double.isNaN(dl.vals[i])) {
		if (dl.vals[i] > 0.0f)
		    out.writeBytes (" " + i + "  " + dl.vals[i] + "\n");
		i++;
	    }
	    if (i != num) {
		out.writeBytes ("-1 " + dl.numlost + "\n");
		out.writeBytes ("PerfData1 \"" + name + "\" " + num + "\n");
	    }
	}
	out.writeBytes ("-1 0\n");
    }


    private void addDataLine (DataLine dl) {
	int i;

	dl.linetotal = 0.0;
	for (i = 0; i < num; i++)
	    dl.linetotal += dl.diffs[i];

	while (datalines.size() > maxdatalines) {
	    DataLine tmp = (DataLine)datalines.remove(0);
	    for (i = 0; i < num; i++) {
		if (!Double.isNaN (tmp.diffs[i])) {
		    sums[i] -= tmp.diffs[i];
		    numsamps[i]--;
		}
	    }
	    if (!Double.isNaN(tmp.linetotal)) {
		totalsum -= tmp.linetotal;
		totalsamps--;
	    }
	}
	for (i = 0; i < num; i++) {
	    if (!Double.isNaN (dl.diffs[i])) {
		sums[i] += dl.diffs[i];
		numsamps[i]++;
	    }
	}
	if (!Double.isNaN(dl.linetotal)) {
	    maxlinetotal = Math.max(maxlinetotal, dl.linetotal);
	    totalsum += dl.linetotal;
	    totalsamps++;
	}
	datalines.add(dl);
    }



    public void setMaxSamples (int n) {
	maxdatalines = n;
	System.out.println ("collector: max lines set to " + maxdatalines);
    }


    public int getNumPhases() {
	return num;
    }

    public String getName() {
	return name;
    }


    public void refreshMaxValues () {
	DataLine dl;
	int i;
	for (i = 0; i < num; i++)
	    maxvals[i] = 0.0;
	maxlinetotal = 0.0;
	ListIterator li = datalines.listIterator(0);
	while (li.hasNext()) {
	    dl = (DataLine)li.next();
	    for (i = 0; i < num; i++)
		if (!Double.isNaN(dl.diffs[i]))
		    maxvals[i] = Math.max(maxvals[i], dl.diffs[i]);
	    if (!Double.isNaN(dl.linetotal))
		maxlinetotal = Math.max (maxlinetotal, dl.linetotal);
	}
//  	for (i = 0; i < num; i++) 
//  	    System.out.println ("max " + i + " is " + maxvals[i]);
    }

    public double getMaxValue () {
	double maxval = 0.0;
	for (int i = 0; i < num; i++)
	    maxval += maxvals[i];
	System.out.println ("alt generation method show " + maxval);
	return maxlinetotal;
    }

    public double getMaxValueForPhase (int phase) {
	    return maxvals[phase];
    }


    public double getAverageForPhase (int _phase) {
	return sums[_phase]/numsamps[_phase];
    }


    public String getLabelForPhase (int _phase) {
	if (infochunk == null)
	    return "";
	Property labelsprop = infochunk.getProperty ("labels");
	if (labelsprop != null) {
	    if ( _phase < labelsprop.getNum())
		return labelsprop.getValue(_phase).getString();
	}
	return "";
    }


    public String toString () {
	return name;
    }


    public String prebuff (double d, int b) {
	String s = Double.toString(d);
	while (s.length() < b)
	    s = " " + s;
	return s;
    }

    public String dumpData() {
	String s = name + "\n";
	DataLine d;
	ListIterator li;
	int i,j;
	for (j = 0; j < num; j++)
	    s = s + prebuff (j, (j==0)?9:18);
	s += "               Total\n";
	//for (i = 0; i < datalines.size(); i++) {
	li = datalines.listIterator(0);
	while (li.hasNext()) {
	    d = (DataLine)li.next();
	    //d = (DataLine)datalines.elementAt(i);
	    for (j = 0; j < num; j++) {
		s = s + prebuff(d.diffs[j],(j==0)?9:18);
	    }
	    if (d.numlost > 0)
		s = s + prebuff(d.linetotal,18) + "\t(lost " + d.numlost + ")\n";
	    else
		s += "\n";
	}

	return s;
    }


    private String padFloat (double f) {
	// reformats f to a string w/ 7 spaces before decimal, and
	// 3 after...
	String s = Double.toString(f);
	int i = s.lastIndexOf('.');
	if ((i >= 0) && (i +3 < s.length()))
	    s = s.substring (0, i + 3);
	while (s.length() < 11)
	    s = ' ' + s;
	return s;
    }


    public void generateAverages (int preskip, int postskip) {

    }

    public String dumpAverages (int preskip, int postskip, boolean doanomaly, double cutoff) {
	int i;
  	Property labelsprop = null;
  	String label;

        // kludgey - make sure we have latest perfdata labels chunk
        List infochunks = Core.vjcontrol_chunkdb.getOfDescToken ("PerfData");
        ConfigChunk ch;
        for (int j = 0; j < infochunks.size(); j++) {
            ch = (ConfigChunk)infochunks.get(j);
            if (name.startsWith (ch.getName())) {
                infochunk = ch;
                break;
            }
        }

        if (infochunk != null)
            labelsprop = infochunk.getProperty ("labels");

	String s = name + ":  averages Report per Cycle\n";
	double avg;
	for (i = 0; i < num; i++) {
	    avg = (sums[i]/numsamps[i]);
	    if (avg == 0.0)
		continue;
	    label = "";
	    if (labelsprop != null) {
		if ( i < labelsprop.getNum())
		    label = labelsprop.getValue(i).getString();
	    }
	    s += "  part " + i + ":  " + padFloat(avg) + " us\t"
		+ label + "\n";
	}

	s += "  Total: " + (totalsum/totalsamps) + " us\n";

	return s;
    }




    public void read (ConfigStreamTokenizer st) {
        // we'll assume that the initial id stuff has already been read
        // and we can dig straight into the data lines.
        int i, j;
        int index;
        double val;
        int lastlost;
        double lastval;


        try {
            for (;;) {
                st.nextToken();
                index = Integer.parseInt(st.sval);
                st.nextToken();
                //System.out.println ("index is " + index + "\nval is " + st.nval);
                if (index == -1) {
                    int numlost = Integer.parseInt(st.sval);
                    if (numlost == 0)
                        break; // no data lost, no reason to muck with things
                    for (; place < num; place++)
                        dl.vals[place] = Double.NaN;
                    dl.numlost = numlost;
                    prevplace = -1;
                    addDataLine (dl);
                    dl = new DataLine(num);
                    place = 0;
                    break;
                }
                //System.out.println ("place1 is " + place); 
                val = Double.parseDouble (st.sval);
                for (; place != index; place++) {
                    //System.out.println ("place1 is " + place);
                    if (place >= num) {
                        addDataLine (dl);
                        dl = new DataLine (num);
                        place = 0;
                    }
                    dl.vals[place] = 0.0;
                    dl.diffs[place] = 0.0;
                    if (place == index)
                        break; //kludge
                }
                place = index;
                //System.out.println ("place2 is " + place); 
                dl.vals[place] = val;
                if (prevplace != -1) {
                    dl.diffs[place] = dl.vals[place] - prevval;
                }
                else
                    dl.diffs[place] = Double.NaN;
                prevplace = place;
                prevval = val;
                place = (place+1);
                if (place >= num) {
                    addDataLine(dl);
                    dl = new DataLine(num);
                    place = 0;
                }
            }
        }
        catch (IOException e) {
            System.out.println ("Read failed:\n" + e);
        }

        notifyActionListenersUpdate();
    }


    //--------------------- ActionEvent Stuff ------------------------

    public void addActionListener (ActionListener l) {
	synchronized (action_listeners) {
	    action_listeners.add (l);
	}
    }

    public void removeActionListener (ActionListener l) {
	synchronized (action_listeners) {
	    action_listeners.remove (l);
	}
    }


    private void notifyActionListenersUpdate () {
        ActionEvent e = new ActionEvent (this, ActionEvent.ACTION_PERFORMED,
                                         "Update");
        notifyActionListeners (e);
    }

    private void notifyActionListenersClear () {
        ActionEvent e = new ActionEvent (this, ActionEvent.ACTION_PERFORMED,
                                         "Clear");
        notifyActionListeners (e);
    }

    private void notifyActionListeners (ActionEvent e) {
        ActionListener l;
        int i, n;
        synchronized (action_listeners) {
            n = action_listeners.size();
            for (i = 0; i < n; i++) {
                l = (ActionListener)action_listeners.get(i);
                l.actionPerformed (e);
            }
        }
    }




};






