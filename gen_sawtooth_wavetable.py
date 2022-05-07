import numpy as np
import math
import matplotlib.pyplot as plt
import struct

def float_to_uint32(f):
    bin = struct.pack('f', f)
    i = struct.unpack('I', bin)[0]
    return hex(i)

max_frequency = 20000.
table_size = 128 # for a half wave
max_phi = 0.5
max_value = 1.0 / 1.18
max_harmonics = table_size / 2

def note_to_freq(note):
    a = 440.0
    return (a / 32) * (2 ** ((note - 9.0) / 12))

def num_of_harmonics(freq):
    return int(max_frequency / freq)

def note_and_harmonics(note):
    freq = note_to_freq(note)
    harm = num_of_harmonics(freq)
    return (note, harm)

def harmonics_to_notes(notes):
    note_harmonics = list(map(note_and_harmonics, notes))
    harmonics = list(set(map(lambda x: x[1], note_harmonics)))
    result = {h:[] for h in harmonics}
    for n, h in note_harmonics:
        try:
            result[h].extend([n])
        except:
            print(h, n)
    return result

note_numbers = list(range(54, 138))
harmonics_notes = harmonics_to_notes(note_numbers)
harmonics = harmonics_notes.keys()
harmonics.sort(reverse=True)
notes = [max(n) for n in harmonics_notes.values()]
notes.sort()

# generate wavetables

t = np.linspace(0., max_phi, table_size, endpoint=False)
t += max_phi / table_size / 2

wave_tables = []
for h in harmonics:
    wt = np.zeros(shape = (t.shape[0],),)
##    for n in range(1, h + 1):
    for n in range(1, h):
        wt += max_value * 2/(np.pi * n) * np.sin(2 * np.pi * n * t)
    wave_tables.append(wt)

# print note numbers for each table

_text_ = '''
#define w_tbl_idx_size {_idx_size_}
#define w_tbl_size {_tbl_size_}

uint8_t note_boundary[w_tbl_idx_size] = '''.format(
    _idx_size_ = str(len(notes)),
    _tbl_size_ = str(table_size),
)

print(_text_)

print('    {')

pos = 0
for i in range(0, 1 + len(harmonics)/4):
    print("        " + ', '.join(map(str, notes[pos:pos+4:1])) + ',')
    pos += 4
    if pos >= len(harmonics):
        break
print("    };\n\n");

# print all wavetables

print("uint32_t w_tbl[w_tbl_idx_size][w_tbl_size + 1] = ")
print("    {")
for wt in wave_tables:
    print("        {")
    pos = 0
    wt2 = np.append(wt, -wt[-1])
    for i in range(0, 1 + len(wt2) / 8):
        print("            " + ', '.join(map(float_to_uint32, wt2[pos:pos+8:1])) + ',')
        pos += 8
        if pos >= len(wt2):
            break
    print("        },")
print("    };")

# plot wavetables for debug

def rgb(i):
    b = (i & 4) >> 2
    g = (i & 2) >> 1
    r = i & 1
    return (r, g, b)

colors = list(map(rgb, [1, 2, 4, 3, 5, 6]))

plt.title("Sawtooth wave")
step = 8
for i in range(0, len(wave_tables), step):
    plt.plot(t, wave_tables[i], color=colors[(i / step) % 6], linewidth=1.0, label='n='+str(harmonics[i]))
plt.legend()
plt.xlabel('t')
plt.ylabel('x(t)')
plt.grid(True)
plt.show()
