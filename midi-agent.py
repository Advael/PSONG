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
from PIL import Image
from keras.utils import np_utils
from music21 import converter, instrument, note, chord
import random
import pong_agent
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
    image = image[::2,::2,2] # downsample by factor of 2 and use the Blue color channel
    colors = np.stack(np.unique(image[:,:], return_counts = True)) 
    bgc = colors[0][np.argmax(colors[1])] #Find the most common color
    image[image != bgc] = 1
    image[image == bgc] = 0
    return np.expand_dims(image.astype(np.float16), axis=2)

def song_model(obs, batch_size, n_vocab):
    songModel = Sequential()
    songModel.add(Embedding(input_dim=400, output_dim=n_vocab, input_length=None, batch_input_shape = (batch_size,) + (400,)))
    songModel.add(LSTM(512, dropout = 0.3, stateful = True, return_sequences=True))
    songModel.add(LSTM(512, dropout = 0.3, stateful = True, return_sequences=True))
    songModel.add(LSTM(512, dropout = 0.3, stateful = True))
    songModel.add(Dense(n_vocab))
    songModel.add(Activation('softmax'))
    return songModel

def agent_model(env, batch_size, n_vocab, lr = 1e-3):
    obs = env.reset()
    obs = preprocess(obs)
    batch_size = 1
    i = Input(shape = obs.shape, batch_shape = (batch_size,) + obs.shape)
    eyes = visionModel(obs, batch_size)
    seen = eyes(i)
    voice = song_model(obs, batch_size, songs = 10)
    song = voice(seen)
    midiModel = Model(inputs = i, outputs = song)
    midiModel.summary()
    action = Dense(env.action_space.n)(song)
    action = Activation('softmax')(action)
    agent = Model(inputs = i, outputs = action)
    y = agent.output
    t = Input(y.shape)
    loss = lambda y,t: -t
    agent.compile(RMSprop(lr=lr), loss)
    return agent

def discount_rewards(rewards, gamma = 0.99):
    discounted = K.zeros_like(rewards)
    for x in reversed(range(len(rewards))):
        accumulator *= gamma
        accumulator += rewards[x]
        rewards[x] = accumulator
    return discounted

def __main__():
    K.set_learning_phase(1)
    songs, n_vocab = load_midis(maxSongs = 10)
    songData,songLabelTemplate = process_songs(songs, n_vocab)
    nBatches = 1
    env = gym.make('Pong-v0')
    agent = agent_model(env,nBatches,n_vocab)
    agent.summary()

    nIter = 10
    iterations = 0
    done = False
    obs = [preprocess(env.reset())]
    batch = K.stack(obs,axis=0)
    print(batch.shape)
    actions = []
    rewards = []
    while iterations < nIter:
        actions.append(agent.predict_on_batch(batch))
        print(notes[-1])
        steps = 0
        obs[0], reward, done, _ = env.step(actions[-1])
        obs[0] = preprocess(obs[0])
        rewards.append(reward)
        batch = K.stack(obs,axis=0)
        agent.save_reward(reward)
        if reward is not 0 or done[x]:
            K.set_learning_phase(1)
            print("Reward {} at step {}".format(reward, steps))
            iterations += 1
            agent.train_on_batch(K.convert_to_tensor(actions), discount_rewards(K.convert_to_tensor(np.array(rewards))))
            env.reset()
            rewards = []
            actions = []
            if(done):
                done= False
                print("Batch {} reached 'Done' at step {}".format(x, steps[x]))
            steps += 1
            K.set_learning_phase(0)
__main__()
