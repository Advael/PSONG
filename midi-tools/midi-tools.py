from midi import *
from sys import argv
from copy import deepcopy

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

if len(argv) > 1:
  print(list(map(singleNote, fileToTicks(argv[1]))))
