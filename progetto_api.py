from collections import defaultdict
from copy import deepcopy
import os

def print_tape(tape, tape_pos):
    #print(min(list(tape) + [tape_pos]), end='')
    print('|', end='')
    for i in range(min(list(tape) + [tape_pos]), max(list(tape) + [tape_pos]) + 1):
        if i == tape_pos:
            print('[', end='')
            print(tape[i], end='')
            print(']', end='')
        else:
            print(tape[i], end='')
    print('|', end='')
    #print(max(list(tape) + [tape_pos]), end='')
    print('')

def run_processes():
    global PROCESSES
    one_stalled = False
    while len(PROCESSES) > 0:
        cp = next(iter(PROCESSES))
        if DEBUG: print('NEW_PROCESS')
        while True:
            if DEBUG: print(cp['current_state'], end='')
            if DEBUG: print_tape(cp['tape'], cp['tape_pos'])
            if cp['current_state'] in TERMINAL_STATES:
                if DEBUG: print('||1||')
                if BUFFER_MODE: OUT_BUFFER[len(OUT_BUFFER)] = '1'
                else: print('1')
                return
            if cp['steps'] >= MAX_STEPS:
                PROCESSES.remove(cp)
                if DEBUG: print('||U||')
                one_stalled = True
                break
            valid_transitions = TRANSITIONS[(cp['current_state'], cp['tape'][cp['tape_pos']])]
            if len(valid_transitions) <= 0:
                PROCESSES.remove(cp)
                if DEBUG: print('||0||')
                break
            else:
                for vtr in valid_transitions[1:]:
                    if DEBUG: print('SPLIT')
                    new_tape = deepcopy(cp['tape'])
                    new_tape[cp['tape_pos']] = vtr['write_char']
                    np = {'current_state': vtr['end_state'], 'tape': new_tape,
                          'tape_pos': cp['tape_pos'] + MOVES[vtr['direction']], 'steps': cp['steps'] + 1}
                    PROCESSES.insert(0,np)
                vtr = valid_transitions[0]
                cp['tape'][cp['tape_pos']] = vtr['write_char']
                cp['current_state'] = vtr['end_state']
                cp['tape_pos'] += MOVES[vtr['direction']]
                cp['steps'] += 1
    if one_stalled:
        if BUFFER_MODE: OUT_BUFFER[len(OUT_BUFFER)] = 'U'
        else: print('U')
    else:
        if BUFFER_MODE: OUT_BUFFER[len(OUT_BUFFER)] = '0'
        else: print('0')

def parse_file():
    global PROCESSES
    global MAX_STEPS
    mode = ''
    run = 0
    with open(FILE) as f:
        for line in f.readlines():
            if line.strip() in ['tr','acc','max','run']:
                mode = line.strip()
                continue
            if mode == 'tr':
                start_state, read_char, write_char, direction, end_state = line.strip().split(' ')
                transition = {'start_state': int(start_state), 'read_char': read_char}
                transition.update({'write_char': write_char, 'direction': direction, 'end_state': int(end_state)})
                TRANSITIONS[(int(start_state), read_char)].insert(0, transition)
            elif mode == 'acc':
                TERMINAL_STATES.append(int(line.strip()))
            elif mode == 'max':
                MAX_STEPS = int(line.strip())
            elif mode == 'run':
                tape = defaultdict(lambda: '_')
                for i, char in enumerate(line.strip()):
                    tape[i] = char
                PROCESSES = [{'current_state': 0, 'tape': tape, 'tape_pos': 0, 'steps': 0}]
                run_processes()
                run += 1

if True and __name__ == '__main__':
    FILE = 'problems/ToCOrNotToC.prob.txt'
    TRANSITIONS = defaultdict(list)
    MOVES = {'R': 1, 'S': 0, 'L': -1}
    TERMINAL_STATES = []
    MAX_STEPS = 0
    PROCESSES = []
    DEBUG = True
    BUFFER_MODE = False
    OUT_BUFFER = defaultdict(lambda: 'E')
    parse_file()

if False and __name__ == '__main__':
    for file in os.listdir('problems'):
        FILE = os.path.join('problems', file)
        TRANSITIONS = defaultdict(list)
        MOVES = {'R': 1, 'S': 0, 'L': -1}
        TERMINAL_STATES = []
        MAX_STEPS = 0
        PROCESSES = []
        DEBUG = False
        BUFFER_MODE = True
        OUT_BUFFER = defaultdict(lambda: 'E')
        print(file[:-9])
        parse_file()
        with open(os.path.join('solutions', file.replace('prob', 'sol'))) as f:
            for i, line in enumerate(f.readlines()):
                s = line.strip()
                print(OUT_BUFFER[i], s, OUT_BUFFER[i] == s)