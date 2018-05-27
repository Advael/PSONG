from midi import *
from sys import argv

pattern = read_midifile(argv[1])
print(pattern)
