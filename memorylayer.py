from keras.engine.base_layer import Layer
import keras.backend as K
class Memory(Layer):
    """Stores memory statefully and returns the whole thing as a time series"""

    def __init__(self, **kwargs):
        super(Memory, self).__init__(**kwargs)
        self.stateful = True
        self.states = None

    def call(self, inputs):
        print(inputs.shape)
        #super(Memory, self).call(inputs)
        new = K.expand_dims(inputs, axis=1)
        print(new.shape)
        if(self.states is None):
            self.states = new
        else:
            self.states = K.append(self.states, new, axis=1)
        return self.states

    def compute_output_shape(self, input_shape):
        #super(Memory, self).compute_output_shape(input_shape)
        return (None,) + input_shape

    def reset_states(self):
        #super(Memory, self).reset_states()
        self.states = None
