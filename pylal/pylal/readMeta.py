#!/usr/bin/env python
"""
This modules provides a metaDataTable class which allows access to
the contents of LIGO lightweight files from python.  It relies on
wrapped versions of the LAL reading utilities in the metaio shared
library.
"""

__author__ = 'Patrick Brady <patrick@gravity.phys.uwm.edu>'
__date__ = '$Date$'
__version__ = '$Revision$'[11:-2]

import sys, getopt
import re
from pylal import support
from pylab import *

def uniq(list):
  """
  return a list containing the unique elements 
  from the original list
  """
  l = []
  for m in list:
    if m not in l:
      l.append(m)
  return l

class metaDataTable:
  """
  Generic metadata table class.  Provides methods to read the contents
  of LIGO lightweight files and manipulate the corresponding tables.
  The tables are stored as a list of dictionaries:  each element of
  the list represents a row,  each element of the dictionary
  represents a column of the table.
  """
  def __init__(self, triggerfile, tabletype):
    """
    @param triggerfile: a regex to match input files
    @param tabletype: the type of table to read in
    """
    self.tabletype = tabletype
    if ( triggerfile ):
      self.readfiles(triggerfile, tabletype)
    else:
      self.table = []

  def readfiles(self, triggerfile, tabletype):
    """
    Populate the table from the list of files 
    """
    if tabletype == "process_params":
      self.table = support.read_process_params(triggerfile)
    if tabletype == "process":
      self.table = support.read_process(triggerfile)
    if tabletype == "search_summary":
      self.table = support.read_search_summary(triggerfile)
    if tabletype == "summ_value":
      self.table = support.read_summ_value(triggerfile)
    if tabletype == "sngl_inspiral":
      self.table = support.read_sngl_inspiral(triggerfile)
    if tabletype == "sim_inspiral":
      self.table = support.read_sim_inspiral(triggerfile)
    if tabletype == "sngl_burst":
      self.table = support.read_sngl_burst(triggerfile)
    if tabletype == "multi_inspiral":
      self.table = support.read_multi_inspiral(triggerfile)

  def nevents(self):
    """
    Return the number of rows in the resulting table
    """
    return len(self.table)

  def mkarray(self, colname):
    """
    Return a numarray sliced from the metadata table based on the
    provided column name.
    @param colname: string name of the column to be sliced out
    """
    myarray = asarray([ self.table[i][colname] for i in range(self.nevents())])
    return myarray

  def ifocut(self, ifo):
    """
    Return a subset of the table, containing only events from the specified ifo
    @param ifo: ifo from which to keep events
    """
    ifocuttable = metaDataTable(None, self.tabletype )

    ifocuttable.table = [entry for entry in self.table \
      if (re.match(ifo,entry["ifo"]))]

    return ifocuttable

  def append(self, table):
    """
    Appends another table to the current table
    """
    self.table.extend(table.table);

  def getslide(self, slide_num):
    """
    Return the triggers with a specific slide number.
    @param slide_num: the slide number to recover (contained in the event_id)
    """

    slide_trigs = metaDataTable(None,None)
    for trig in self.table:
      if ( (trig['event_id'] % 1000000000) / 100000 ) == slide_num:
        slide_trigs.table.append(trig)
     
    return slide_trigs 
  

class coincInspiralTable:
  """
  Table to hold coincident inspiral triggers.  Coincidences are reconstructed 
  by making use of the event_id contained in the sngl_inspiral table.
  The coinc is a dictionary with entries: G1, H1, H2, L1, event_id, numifos, 
  snrsq.  
  The snrsq is the sum of the squares of the snrs of the individual triggers.
  """
  def __init__(self, inspTriggers = None):
    """
    @param inspTriggese: a metaDataTable containing inspiral triggers 
                         from which to construct coincidences
    """
    if not inspTriggers:
      self.table = []
    else:
      # use the supplied method to convert these columns into numarrays
      eventidlist = asarray(uniq(inspTriggers.mkarray("event_id")))
      self.table = []
      for event_id in eventidlist: 
        self.table.append({ "event_id":event_id, "numifos":0, "snrsq":0 })

      for coinc in self.table:
        for trig in inspTriggers.table:
          if coinc["event_id"] == trig["event_id"]:
            coinc[trig["ifo"]] = trig
            coinc["numifos"] += 1
            coinc["snrsq"] += trig["snr"] * trig["snr"]
    
    
  def nevents(self):
    """
    Return the number of rows in the resulting table
    """
    return len(self.table)

  def mkarray(self, colname, ifoname):
    mylist = []
    for i in range(len(self.table)):
      tmpvalue = 0;
      if self.table[i].has_key(ifoname) and \
        self.table[i][ifoname].has_key(colname):
        tmpvalue += (self.table[i][ifoname][colname])
      mylist.append( tmpvalue )

    return asarray( mylist )


  def coinctype(self, ifolist):
    """
    Return the coincs which are from ifos.
    @param ifos: a list of ifos 
    """
 
    selected_coincs = coincInspiralTable()
    for coinc in self.table:
      keep_trig = True
      if coinc["numifos"] == len(ifolist):
        for ifo in ifolist:
          if not coinc.has_key(ifo):
            keep_trig = False
            break
            
        if keep_trig == True:
          selected_coincs.table.append(coinc)
        
    return selected_coincs


  def getsngls(self, ifo):
    """
    Return the sngls for a specific ifo.
    @param ifo: ifo for which to retrieve the single inspirals.
    """

    ifo_sngl = metaDataTable(None, 'sngl_inspiral')
    for coinc in self.table:
      if coinc.has_key(ifo):
        ifo_sngl.table.append(coinc[ifo])
        
    return ifo_sngl


  def getslide(self, slide_num):
    """
    Return the triggers with a specific slide number.
    @param slide_num: the slide number to recover (contained in the event_id)
    """

    slide_coincs = coincInspiralTable()
    for coinc in self.table:
      if ( (coinc['event_id'] % 1000000000) / 100000 ) == slide_num:
        slide_coincs.table.append(coinc)
     
    return slide_coincs 

  
  def cluster(self, cluster_window):
    """
    Return the clustered triggers, returning the one with the largest snrsq in 
    each fixed cluster_window
    
    @param cluster_window: length of time over which to cluster (seconds)
    """
    ifolist = ['G1','H1','H2','L1']
    # find times when there is a trigger
    cluster_times = []
    for coinc in self.table:
      for ifo in ifolist:
        if coinc.has_key(ifo):
          end_time = coinc[ifo]['end_time']
          break
      cluster_times.append(cluster_window * (end_time/cluster_window) )
    cluster_times = uniq(cluster_times)
    
    cluster_triggers = coincInspiralTable()
    for cluster_time in cluster_times:
      # find all triggers at that time
      cluster = coincInspiralTable()
      for coinc in self.table:
        for ifo in ifolist:
          if coinc.has_key(ifo):
            end_time = coinc[ifo]['end_time']
            break
        if ((end_time - cluster_time) / cluster_window == 0):   
          cluster.table.append(coinc)

      # find loudest trigger in time and append
      loudest_snrsq = 0
      for trigger in cluster.table:
        if trigger["snrsq"] > loudest_snrsq:
          loudest_trig = trigger
          loudest_snrsq = trigger['snrsq']

      cluster_triggers.table.append(loudest_trig)
      
    return cluster_triggers 


def usage():
        print "readMeta.py -x xml file"
        sys.exit(0)

def main():
  # parse options
  try:
    opts,args=getopt.getopt(sys.argv[1:],"x:h",["xmlfile=","help"])
  except getopt.GetoptError:
    usage()
    sys.exit(2)
  for o,a in opts:
    if o in ("-x","--xmlfile"):
             xmlfile = a
    if o in ("-h", "--help"):
             usage()
             sys.exit()
  print 'main() does nothing at the moment' 

# execute main if this module is explicitly invoked by the user
if __name__=="__main__":
        main()
