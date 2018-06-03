from midi import *
from sys import argv
from copy import deepcopy
import midi.sequencer as sequencer
import time
import socket
import os

##################################### Usage #########################################
# fileToNotes reads a midi file and returns a list of notes in order without        #
# spacing, one note per key press                                                   #
#                                                                                   #
# fileToTickNotes reads a midi file and returns a list of the active notes at each  #
# tick of the song; if more than one key is pressed it takes the most recent        #
# I believe this is the one we want to use, possibly sampling every nth tick        #
#                                                                                   #
# There is the potential for other ways of dealing with multiple simultaneous notes #
# but this way is simple and effective                                              #
#####################################################################################

def fileToNotes(f):
  pattern = read_midifile(f)
  track = pattern[0]
  track.make_ticks_abs()
  return [event.data[0] for event in track if isinstance(event, NoteOnEvent)]

def fileToTicks(f):
  pattern = read_midifile(f)
  track = pattern[0]
  track.make_ticks_abs()
  lastTick = track[len(track)-1].tick
  ticks = [[] for i in range(lastTick)]
  j = 0
  for i in range(lastTick):
    ticks[i] = deepcopy(ticks[i-1])
    e = track[j]
    while e.tick == i:
      if isinstance(e, NoteOnEvent):
        ticks[i].append(e.data[0])
      if isinstance(e, NoteOffEvent):
        ticks[i].remove(e.data[0])
      j += 1
      e = track[j]
  return ticks

def singleNote(notes):
  if len(notes) == 0: return 0
  else: return notes[len(notes)-1]

def fileToTickNotes(f):
  return list(map(singleNote, fileToTicks(f)))

def stageToSeq(tickNotes):
  parent, child = socket.socket
  client = 128
  port = 0
  seq = sequencer.SequencerWrite()
  seq.subscribe_port(client, port)
  seq.start_sequencer()
  for tick in range(len(tickNotes)):
    # create an event
    buf = None
    if (tick-1 < 0) or tickNotes[tick] != tickNotes[tick-1] and tickNotes[tick] != 0:
      event = NoteOnEvent()
      event.tick = 3*tick
      event.set_pitch(tickNotes[tick])
      event.set_velocity(40)
      buf = seq.event_write(event, False, False, True)
    if (tick+1 >= len(tickNotes)) or tickNotes[tick] != tickNotes[tick+1] and tickNotes[tick] != 0:
      event = NoteOffEvent()
      event.tick = 3*tick+1
      event.set_pitch(tickNotes[tick])
      event.set_velocity(40)
      buf = seq.event_write(event, False, False, True)
    if buf == None:
      continue
    if buf < 1000:
      time.sleep(.1)
  time.sleep(30)

if len(argv) > 1:
#  print(list(map(singleNote, fileToTicks(argv[1]))))
  stageToSeq(list(map(singleNote, fileToTicks(argv[1]))))
