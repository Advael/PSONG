from midi import *
from sys import argv
from copy import deepcopy
from multiprocessing import Process
import midi.sequencer as sequencer
import time
import socket
import os

##################################### Usage #########################################
# fileToTickNotes reads a midi file and returns a list of the active notes at each  #
# tick of the song; if more than one key is pressed it takes the most recent        #
# This is the one we want to use, possibly sampling every nth tick                  #
#                                                                                   #
# daemonStart opens a daemon that listens to a socket and forewards note info to    #
# a midi server running on client 128, port 0                                       #
# To send notes to it, call stageClient with a list of notes, like so:              #
#                                                                                   #
# p = daemonStart()                                                                 #
# if len(argv) > 1:                                                                 #
#   stageClient(list(fileToTickNotes(argv[1])))                                     #
# daemonEnd(p)                                                                      #
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
  client = 128
  port = 0
  seq = sequencer.SequencerWrite()
  seq.subscribe_port(client, port)
  seq.start_sequencer()
  for tick in range(len(tickNotes)):
    buf = None
    if (tick-1 < 0) or tickNotes[tick] != tickNotes[tick-1] and tickNotes[tick] != 0:
      event = NoteOnEvent()
      event.tick = tick
      event.set_pitch(tickNotes[tick])
      event.set_velocity(40)
      buf = seq.event_write(event, False, False, True)
    if (tick+1 >= len(tickNotes)) or tickNotes[tick] != tickNotes[tick+1] and tickNotes[tick] != 0:
      event = NoteOffEvent()
      event.tick = tick+1
      event.set_pitch(tickNotes[tick])
      event.set_velocity(40)
      buf = seq.event_write(event, False, False, True)
    if buf == None:
      continue
    if buf < 1000:
      time.sleep(.1)
  time.sleep(30)

def stageDaemon(client, port, filename):
  seq = sequencer.SequencerWrite()
  seq.subscribe_port(client, port)
  seq.start_sequencer()
  tempo = SetTempoEvent()
  tempo.set_bpm(60)
  sock = socket.socket(socket.AF_UNIX, socket.SOCK_STREAM)
  sock.bind(filename)
  sock.listen(10)
  tick = 0

  while True:
    conn,addr = sock.accept()
    lastNote = 0
    while True:
      data = conn.recv(1000)
      if not data:
        break
      notes = map(lambda b: int(b), data)
      for note in notes:
        tick+=5
        buf = None
        if note != lastNote:
          if lastNote != 0:
            event = NoteOffEvent()
            event.tick = tick
            event.set_pitch(note)
            event.set_velocity(40)
            buf = seq.event_write(event, False, False, True)
          if note == 0:
            tick+=5
          else:
            event = NoteOnEvent()
            event.tick = tick
            event.set_pitch(note)
            event.set_velocity(40)
            buf = seq.event_write(event, False, False, True)
        lastNote = note
        if buf == None:
          continue
      time.sleep(.1)
    conn.close()

def stageClient(notes):
  sock = socket.socket(socket.AF_UNIX, socket.SOCK_STREAM)
  sock.connect("./note_feed")
  for note in notes:
    sock.send(pack('!b',note))
  sock.close()

def daemonStart():
  p = Process(target = stageDaemon, args=(128,0,"./note_feed"))
  p.start()
  return p

def daemonEnd(p):
  p.join()

p = daemonStart()                                                                 
if len(argv) > 1:
  for i in range(10):
    stageClient(list(fileToTickNotes(argv[1])))
    time.sleep(5)
daemonEnd(p)
