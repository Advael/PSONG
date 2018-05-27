from midi import *
from sys import argv
from copy import deepcopy

##################################### Usage #########################################
# fileToNotes reads a midi file and returns a list of notes in order without        #
# spacing, one note per key press                                                   #
#                                                                                   #
# fileToTickNotes reads a midi file and returns a list of the active notes at each  #
# tick of the song; if more than one key is pressed it takes the median note        #
# I believe this is the one we want to use, possibly sampling every nth tick        #
#                                                                                   #
# I'm planning on modifying fileToTickNotes to not carry a big list of potential    #
# notes around, should speed it up a lot. The idea was to have different strategies #
# for handling concurrent notes, but there are better ways to that, too             #
#####################################################################################

def silence():
  return [0 for i in range(128)]

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
  ticks = [silence() for i in range(lastTick)]
  j = 0
  for i in range(lastTick):
    ticks[i] = deepcopy(ticks[i-1])
    e = track[j]
    while e.tick == i:
      if isinstance(e, NoteOnEvent):
        ticks[i][e.data[0]] = 1
      if isinstance(e, NoteOffEvent):
        ticks[i][e.data[0]] = 0
      j += 1
      e = track[j]
  return ticks

def tickNotes(t):
  return [i for i in range(len(t)) if t[i] == 1]

def singleNote(notes):
  if len(notes) == 0: return 0
  else: return notes[len(notes)//2]

def fileToTickNotes(f):
  return list(map(singleNote, map(tickNotes, fileToTicks(f)))) 

if len(argv) > 1:
  print(list(map(singleNote, map(tickNotes, fileToTicks(argv[1])))))
