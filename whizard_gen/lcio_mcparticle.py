'''
 example python script to read lcio files
  -> compute average E_cms from MCParticle truth
  Set these environment variables to run:

 export PYTHONPATH=$ROOTSYS/lib:$PYTHONPATH
 export PYTHONPATH=${LCIO}/src/python:${LCIO}/examples/python:${PYTHONPATH}
'''
from pyLCIO import IOIMPL, UTIL
import sys


reader = IOIMPL.LCFactory.getInstance().createLCReader()

reader.open( sys.argv[1] )

nEvt = 0
E_cms = 0.

for evt in reader:
#  print( "evt: " , evt.getEventNumber() )
  nEvt += 1
  mcpcol = evt.getCollection("MCParticle")

  sqrts = 0.

  for mcp in mcpcol:
    if( mcp.getGeneratorStatus() == 1 ):
      sqrts += mcp.getEnergy()

  E_cms += sqrts

print( "<E_cms> = " , E_cms/nEvt )

 
