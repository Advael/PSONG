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
from music21 import converter, instrument, note, chord
from PIL import Image
import matplotlib.pyplot as plt
from IPython import display
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
    for file in random.sample(data, n):
        notes = []
        midi = converter.parse(file)
        print("Parsing %s" % file)
        notes_to_parse = None
        try: # file has instrument parts
            s2 = instrument.partitionByInstrument(midi)
            notes_to_parse = s2.parts[0].recurse() 
        except: # file has notes in a flat structure
            notes_to_parse = midi.flat.notes
        for element in notes_to_parse:
            if isinstance(element, note.Note):
                notes.append(str(element.pitch))
            elif isinstance(element, chord.Chord):
                notes.append('.'.join(str(n) for n in element.normalOrder))
        songs.append(notes)
    with open('data/notes', 'wb') as filepath:
        pickle.dump(notes, filepath)
    return (songs, len(set([item for sublist in songs for item in sublist])))

def process_songs(songs, n_vocab):
    pitchnames = sorted(set([item for sublist in songs for item in sublist]))
    note_to_int = dict((note, number) for number, note in enumerate(pitchnames))
    network_input = []
    network_output = []
    # create input sequences and the corresponding outputs
    for i in songs:
        f = np.reshape([note_to_int[char] for char in i], (1, len(i), 1))
        f = f / float(n_vocab)
        network_input.append(f)
        network_output.append(note_to_int[i[-1]])
    n_songs = len(network_input)
    # normalize input
    network_output = np_utils.to_categorical(network_output)
    return (network_input, network_output)

def train(model, network_input, network_output):
    """ train the neural network """
    filepath = "weights-improvement-{epoch:02d}-{loss:.4f}-bigger.hdf5"
    checkpoint = ModelCheckpoint(
        filepath,
        monitor='loss',
        verbose=0,
        save_best_only=True,
        mode='min'
    )
    callbacks_list = [checkpoint]
    model.fit(network_input, network_output, epochs=200, batch_size=64, callbacks=callbacks_list)

def visionModel(obs, batch_size):
    model = Sequential()
    model.add(Conv2D(4, 3, strides=3, input_shape = obs.shape, batch_input_shape = (batch_size,) + obs.shape))
    model.add(Conv2D(8, 3, strides=2))
    model.add(Conv2D(16, 3, strides=2))
    model.add(Flatten())
    return model

def preprocess(image):
    """ prepro 210x160x3 uint8 frame into 6400 (80x80) 1D float vector """
    image = image[35:195] # crop
    image = image[::,::,2] # downsample by factor of 2 and use the Blue color channel
    image = cv2.resize(image, (40,40))
    colors = np.stack(np.unique(image[:,:], return_counts = True)) 
    bgc = colors[0][np.argmax(colors[1])] #Find the most common color
    image[image != bgc] = 1
    image[image == bgc] = 0
    return image.astype(np.float64).ravel()

def gen_song_model(obs, n_vocab):
    songModel = Sequential()
    songModel.add(LSTM(512, dropout = 0.3, return_sequences=True, input_shape=(None,) + obs.shape))
    songModel.add(LSTM(512, dropout = 0.3, return_sequences=True))
    songModel.add(LSTM(512))
    songModel.add(Dense(257))
    songModel.add(Dropout(0.3))
    songModel.add(Dense(n_vocab))
    songModel.add(Activation('softmax'))
    return songModel

class SequenceAgent:
    def __init__(self, obsModel, actionModel, env, gamma = .99, lr = 1e-3):
        i = actionModel.input
        hidden = actionModel(i)
        action = Dense(env.action_space.n)(hidden)

        action = Activation('softmax')(action)
        aModel = Model(inputs = i, outputs = action)
        self.obsShape = preprocess(env.reset()).shape
        self.observationModel = self.agenty_compile(obsModel)
        self.actionModel = self.agenty_compile(aModel)
        self.reset_memory()

    def decide(self, obs):
        o = preprocess(obs)
        self.obsMemory.append(o)
        obsSeq = np.reshape(np.array(self.obsMemory), (1, len(self.obsMemory)) + o.shape)
        p = self.observationModel.predict(obsSeq)
        print(p)
        a = np.argmax(p)
        self.actMemory.append(a)
        act = self.actionModel.predict(np.expand_dims(a, axis=0))
        choice = probArgMax(act[0])
        return choice

    def process_reward(self,rew):
        self.rewardMemory.append(rew)
        if(not np.isclose(rew, 0.0)):
            print("Reward of {} encountered at step {}, learning".format(rew, len(self.rewardMemory)))
            K.set_learning_phase(1)
            disc = discount_rewards(self.rewardMemory)
            am = np.array(self.actMemory)
            #rm = np.array(self.rewardMemory)
            #print(am.shape, rm.shape)
            actionModelInputs = np.reshape(am, (len(self.actMemory),1))
            actionModelRewards = np.reshape(disc, (len(self.rewardMemory),1))
            self.actionModel.train_on_batch(actionModelInputs, actionModelRewards)
            for x in range(1,len(self.rewardMemory)+1):
                print('.', end = ''),
                obsModelInputs = np.reshape(np.array(self.obsMemory[:x]), (1, len(self.obsMemory[:x])) + self.obsShape)
                obsModelRewards = np.reshape(disc[x-1], (1, 1))
                self.observationModel.train_on_batch(obsModelInputs, obsModelRewards)
            K.set_learning_phase(0)
            self.reset_memory()
            print("!")

    def reset_memory(self):
        self.actMemory = []
        self.obsMemory = []
        self.rewardMemory = []

    def agenty_compile(self, model, lr = 1e-3):
        y = model.output
        t = Input(y.shape)
        loss = lambda y,t: 1.0-t / 2.0 #Naive AF online RL loss function
        model.compile(RMSprop(lr=lr), loss)
        return model

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

def __main__():
    K.set_learning_phase(0)
    songs, n_vocab = load_midis(maxSongs = 1)
    songData,songLabelTemplate = process_songs(songs, n_vocab)
    env = gym.make('Pong-v0')
    obs = preprocess(env.reset())
    songModel = gen_song_model(obs, n_vocab)
    tinybrain = Sequential()
    tinybrain.add(Dense(128, input_shape=(1,)))
    agent = SequenceAgent(songModel,tinybrain,env)
    agent.actionModel.summary()
    agent.observationModel.summary()

    nIter = 10
    iterations = 0
    done = False
    obs = env.reset()
    cumulative_reward = 0
    while iterations < nIter:
        decision = agent.decide(obs)
        obs, reward, done, _ = env.step(decision)
        #env.render()
        agent.process_reward(reward)
        cumulative_reward += reward
        if(done):
            print("Epoch {} cumulative reward: {}".format(iterations,cumulative_reward))
            env.reset()
            agent.reset_memory()
            iterations += 1
            cumulative_reward = 0
__main__();

