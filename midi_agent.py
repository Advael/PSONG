### current bpython session - make changes and save to reevaluate session.
### lines beginning with ### will be ignored.
### To return to bpython without reevaluating make no changes to this file
### or save an empty file.
from keras.models import Sequential
from keras.layers import Reshape, TimeDistributed, Dropout, Flatten, Concatenate
from keras.layers import Dense, Conv2D, LSTM
from keras.layers import Activation, Input, Embedding, Lambda, concatenate
from keras.optimizers import RMSprop
from keras.callbacks import ModelCheckpoint
from keras import Model
from keras import backend as K
from keras.utils import np_utils
#import pygame
import os
from music21.midi import realtime
from music21 import converter, instrument, note, chord, stream
import argparse
import cv2
import random
import glob
import pickle
import numpy as np
import gym
def load_midis(maxSongs = None):
    songs = []
    data = glob.glob("midi_songs/*.mid")
    if maxSongs is None:
        n = len(data)
    else:
        n = maxSongs
    allNotes = []
    for file in random.sample(data, n):
        notes = []
        midi = converter.parse(file)
        print("Parsing %s" % file, end = '')
        notes_to_parse = None
        try: # file has instrument parts
            s2 = instrument.partitionByInstrument(midi)
            notes_to_parse = s2.parts[0].recurse() 
        except: # file has notes in a flat structure
            notes_to_parse = midi.flat.notes
        for element in notes_to_parse:
            if isinstance(element, note.Note):
                notes.append(str(element.pitch))
                allNotes.append(notes[-1])
            elif isinstance(element, chord.Chord):
                notes.append('.'.join(str(n) for n in element.normalOrder))
                allNotes.append(notes[-1])
        songs.append(np.array(notes))
        print(" ({} notes)".format(len(notes)))
    with open('data/notes', 'wb') as filepath:
        pickle.dump(allNotes, filepath)
    return (songs, len(set(allNotes)))

def process_songs(songs, n_vocab):
    pitchnames = sorted(set([item for sublist in songs for item in sublist]))
    note_to_int = dict((note, number) for number, note in enumerate(pitchnames))
    network_input = []
    network_output = []
    # create input sequences and the corresponding outputs
    for i in songs:
        f = np.array([note_to_int[char] for char in i])
        f = f / float(n_vocab)
        network_input.append(f)
        network_output.append(note_to_int[i[-1]])
    n_songs = len(network_input)
    # normalize input
    network_output = np_utils.to_categorical(network_output)
    return (network_input, network_output)


def bitsquish(bits):
    y = 0
    for i,j in enumerate(bits):
        if j: y += int(j)<<i
    return y

def serialize(image):
    """ prepro 210x160x3 uint8 frame into 6400 (80x80) 1D float vector """
    image = image[35:195] # crop
    image = image[::,::,2] # downsample by factor of 2 and use the Blue color channel
    image = cv2.resize(image, (40,40))
    colors = np.stack(np.unique(image[:,:], return_counts = True)) 
    bgc = colors[0][np.argmax(colors[1])] #Find the most common color
    image[image != bgc] = 1
    image[image == bgc] = 0
    return image.astype(np.bool).ravel()

def add_noise(a, n = 4):
    return np.append(a, np.random.uniform(0.0,1.0,size=n))

def serialize_squish(image, n = 4):
    return np.array([bitsquish(s) for s in np.reshape(serialize(image), (100, 16))])

def serialize_squish_noise(image, n = 4):
    return add_noise(serialize_squish(image), n)

def disc_song_model(n_vocab):
    songModel = Sequential()
    songModel.add(LSTM(256, dropout = 0.3, return_sequences=True, input_shape=(None,1)))
    songModel.add(LSTM(256, dropout = 0.3, return_sequences=True))
    songModel.add(LSTM(256))
    songModel.add(Dense(128))
    songModel.add(Dropout(0.3))
    songModel.add(Dense(n_vocab))
    songModel.add(Activation('softmax'))
    return songModel

def gen_song_model(obs, n_vocab):
    songModel = Sequential()
    songModel.add(LSTM(256, dropout = 0.3, return_sequences=True, input_shape=(None,) + obs.shape))
    songModel.add(LSTM(256, dropout = 0.3, return_sequences=True))
    songModel.add(LSTM(256))
    songModel.add(Dense(128))
    songModel.add(Dropout(0.3))
    songModel.add(Dense(n_vocab))
    songModel.add(Activation('softmax'))
    return songModel

class SequenceAgent:
    def __init__(self, env, actionModel, preprofunc = serialize):
        self.preprocess = preprofunc
        self.is_training = False
        self.gate_active = False
        i = actionModel.input
        hidden = actionModel(i)
        action = Dense(env.action_space.n)(hidden)
        action = Activation('softmax')(action)
        aModel = Model(inputs = i, outputs = action)

        self.load_data()

        voice = gen_song_model(self.preprocess(env.reset()), self.vocab)
        critic = disc_song_model(self.vocab)
        v = voice.input
        real = critic.input
        fake = Input(real.shape)

        info = voice(v)

        self.obsShape = self.preprocess(env.reset()).shape
        self.observationModel = voice
        self.criticModel = critic
        self.actionModel = aModel
        self.reset_memory()

    def load_data(self):
        with open('data/notes', 'rb') as filepath:
            notes = pickle.load(filepath)
        self.vocab = len(set(notes))
        if(self.is_training):
            songs, self.vocab = load_midis()
            if v != self.vocab:
                print("dataset does not match model, weights will be invalid for data")
                self.data,self.songLabelTemplate = process_songs(songs, self.vocab)

    def gate(self, o, n = 2, k = 1):
        obsSeq = np.reshape(np.array(self.obsMemory), (1, len(self.obsMemory)) + o.shape)
        fake = np.argmax(self.observationModel.predict(obsSeq))
        if(self.gate_active):
            l = len(self.actMemory)
            candidates = [d for d in self.data if d.shape[0] > l]
            c = candidates[np.random.randint(0,len(candidates))]
            if l < 1:
                realPrefix = np.reshape(np.array([-1.0]), (1,1,1))
                fakePrefix = np.reshape(np.array([-1.0]), (1,1,1))
            else:
                realPrefix = np.reshape(c[:l], (1,l,1))
                fakePrefix = np.reshape(np.array(self.actMemory), (1,l,1))

            real = (c[l] * self.vocab).astype(np.int)
            realPred = self.criticModel.predict(realPrefix)
            fakePred = self.criticModel.predict(fakePrefix)
            self.criticMemory.append(np.append(realPrefix,fakePrefix,axis=0))
            realAgreement = realPred[0][real]
            fakeAgreement = fakePred[0][fake]

            if(realAgreement > fakeAgreement):
                winner = real
                self.tally += 1
            else:
                winner = fake
            a = winner / self.vocab
        else:
            a = fake / self.vocab
        return a

    def compile(self, weights_path = 'checkpoints'):
        self.observationModel = self.agenty_compile(self.observationModel)
        self.actionModel = self.agenty_compile(self.actionModel)
        self.criticModel.compile(loss='binary_crossentropy', optimizer='rmsprop')

        if(os.path.exists(weights_path + '/weights-midi-gen.hdf5')):
            self.observationModel.load_weights(weights_path + '/weights-midi-gen.hdf5')
            print("Observation model loaded checkpoint")
        if(os.path.exists(weights_path + '/weights-midi-critic.hdf5')):
            self.criticModel.load_weights(weights_path + '/weights-midi-critic.hdf5')
            print("Critic model loaded checkpoint")
        if(os.path.exists(weights_path + '/weights-tiny-brain.hdf5')):
            self.actionModel.load_weights(weights_path + '/weights-tiny-brain.hdf5')
            print("Action model loaded checkpoint")

    def observe(self, obs):
        o = self.preprocess(obs)
        self.obsMemory.append(o)
        return self.gate(o)

    def act(self, actionable):
        self.actMemory.append(actionable)
        act = self.actionModel.predict(np.expand_dims(actionable, axis=0))
        choice = probArgMax(act[0])
        return choice

    def process_reward(self,rew):
        self.rewardMemory.append(rew)
        if(not np.isclose(rew, 0.0)):
            print("({} steps)".format(len(self.actMemory)))
            if(self.is_training):
                self.train()
            self.reset_memory()

    def train(self):
        K.set_learning_phase(1)
        disc = discount_rewards(self.rewardMemory)
        am = np.array(self.actMemory)
        criticModelLosses = 0
        random.shuffle(self.criticMemory)
        for x in self.criticMemory:
            criticModelInputs = x
            criticModelLabels = np.vstack([np.ones(self.vocab), np.zeros(self.vocab)])
            criticModelLosses += self.criticModel.train_on_batch(criticModelInputs, criticModelLabels)
        obsModelLosses = 0
        for x in range(1,len(self.rewardMemory)+1):
            #print('.', end = ''),
            obsModelInputs = np.reshape(np.array(self.obsMemory[:x]), (1, len(self.obsMemory[:x])) + self.obsShape)
            obsModelRewards = np.reshape(disc[x-1], (1, 1))
            obsModelLosses += self.observationModel.train_on_batch(obsModelInputs, obsModelRewards)
        actionModelInputs = np.reshape(am, (len(self.actMemory),1))
        actionModelRewards = np.reshape(disc, (len(self.rewardMemory),1))
        actionModelLosses = self.actionModel.train_on_batch(actionModelInputs, actionModelRewards)
        print(np.mean(disc))
        print("Observation Model Losses: {}".format(obsModelLosses))
        print("Action Model Losses: {}".format(actionModelLosses))
        if(self.gate_active):
            print("Critic Model Losses: {}".format(criticModelLosses))
            print("Critic Win Rate: {}".format(self.tally / len(self.actMemory)))
        K.set_learning_phase(0)

    def reset_memory(self):
        self.actMemory = []
        self.obsMemory = []
        self.rewardMemory = []
        self.criticMemory = []
        self.tally = 0

    def save_weights(self, path = 'checkpoints'):
        self.actionModel.save_weights(path + '/weights-tiny-brain.hdf5')
        self.observationModel.save_weights(path + '/weights-midi-gen.hdf5')
        self.criticModel.save_weights(path + '/weights-midi-critic.hdf5')

    def agenty_compile(self, model, lr = 1e-3):
        y = Input((1,))
        loss = lambda y,t: agenty_loss(y,t)
        model.compile(optimizer='rmsprop', loss=loss)
        return model

    def summary(self):
        self.actionModel.summary()
        self.observationModel.summary()
        self.criticModel.summary()

def agenty_loss(y,t):
    return K.square(t ** 0 - y)

def discount_rewards(rewards, gamma = 0.99):
    discounted = np.zeros_like(rewards)
    accumulator = 0
    for x in reversed(range(len(rewards))):
        accumulator *= gamma
        accumulator += rewards[x]
        discounted[x] = accumulator
    return discounted

def probArgMax(x):
    return np.random.choice(range(x.size), p=x)

class DisplayBot:
    def __init__(self, env):
        self.env = env
        with open('data/notes', 'rb') as filepath:
            notes = pickle.load(filepath)
        self.pitchnames = sorted(set(item for item in notes))
        self.int_to_note = dict((number, note) for number, note in enumerate(self.pitchnames))
        self.vocab = len(set(notes))
        freq = 44100    # audio CD quality
        bitsize = -16   # unsigned 16 bit
        channels = 2    # 1 is mono, 2 is stereo
        buffer = 1024    # number of samples
        self.midi_stream = stream.Stream()
        pygame.mixer.init(freq, bitsize, channels, buffer)
        pygame.mixer.music.set_volume(0.8)
        pygame.init()
        self.reset()

    def pattern_generator(self, rawNote):
        index = (rawNote * self.vocab).astype(np.int)
        pattern = self.int_to_note[index]
        if ('.' in pattern) or pattern.isdigit():
            notes_in_chord = pattern.split('.')
            notes = []
            for current_note in notes_in_chord:
                new_note = note.Note(int(current_note))
                new_note.storedInstrument = instrument.Piano()
                notes.append(new_note)
            new_chord = chord.Chord(notes)
            new_chord.offset = self.offset
            n = new_chord
        # pattern is a note
        else:
            new_note = note.Note(pattern)
            new_note.offset = self.offset
            new_note.storedInstrument = instrument.Piano()
            n = new_note
        self.offset += 0.5
        self.midi_stream.append(n)

    def reset(self):
        self.offset = 0

def __main__():
    parser = argparse.ArgumentParser()
    parser.add_argument('--epochs', type=int, default=1000)
    parser.add_argument('--show-and-tell', dest='showAndTell', action='store_true')
    parser.add_argument('--train', dest='train', action='store_true')
    parser.add_argument('--no-use-gate', dest='useGate', action='store_false')
    parser.set_defaults(train = False)
    parser.set_defaults(useGate = True)
    parser.set_defaults(showAndTell = False)
    args = parser.parse_args()

    K.set_learning_phase(0)
    env = gym.make('Pong-v0')
    tinybrain = Sequential()
    tinybrain.add(Dense(128, input_shape=(1,)))
    agent = SequenceAgent(env,tinybrain,preprofunc = serialize_squish_noise)
    agent.summary()
    agent.compile()
    if(args.train):
        agent.is_training = True
    if(args.useGate):
        agent.gate_active = True

    nIter = args.epochs
    iterations = 0
    done = False
    showAndTell = args.showAndTell
    obs = env.reset()
    wins = 0
    losses = 0
#    if(showAndTell):
#        d = DisplayBot(env)
#        r = realtime.StreamPlayer(d.midi_stream)
#        r.play()
    while iterations < nIter:
        if(showAndTell):
#            d.pattern_generator(note)
            env.render()
        decision,note = agent.act(obs)
        obs, reward, done, _ = env.step(decision)
        if(not np.isclose(reward, 0.0)):
            if(reward < 0):
                losses += 1
            else:
                wins += 1
            print("Epoch {}: Score {}-{}".format(iterations, wins, losses), end = '')
        agent.process_reward(reward)
        if(done):
            print("Epoch {} ended with {} cumulative reward".format(iterations, wins - losses))
            env.reset()
            wins = 0
            losses = 0
            agent.reset_memory()
            iterations += 1
            if(agent.is_training and iterations % 1 is 0):
                agent.save_weights()
__main__();

