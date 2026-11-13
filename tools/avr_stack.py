#!/usr/bin/env python3
# Copyright 2026 Andreas Burnickl
# SPDX-License-Identifier: GPL-3.0-or-later
"""Checks that an AVR program's deepest stack fits the RAM the program leaves free.

An AVR has a few KiB of RAM. `.data` and `.bss` take theirs from the bottom, the stack grows
down from the top, and nothing stops the one from running into the other: no MMU, no guard, a
corrupted variable somewhere and a program that does something odd once a week. tools/stack.sh
runs this for every program of every AVR target that CMake builds.

What it adds up, from the call graph avr-gcc writes beside each object when it compiles with
`-fstack-usage -fcallgraph-info=su`, as cmake/toolchains/avr-gcc.cmake has it (an image linked
with -flto takes them at the link, with -save-temps=obj, as LTO inlines across sources; the
image is byte for byte the same with or without them):

- **The deepest path from main()**, each function's frame from the call graph, which counts
  its return address, its saved registers and its locals. The static constructors run before
  main() and are taken as entries of their own.
- **Plus the deepest interrupt handlers that can be on the stack at once.** A classic AVR
  clears the interrupt flag on entering a handler, so one handler at most, as long as none sets
  it again; one that does (`sei`, ISR_NOBLOCK) is refused, as it nests without a bound. An AVR
  of the megaAVR 0 or Dx series has two interrupt levels, so two: one of level 0 and the
  level-1 one on top of it. Taking the deepest, whichever they are, is never less than that.
- **Library code by its disassembly.** avr-libc and libgcc are linked as compiled, so the call
  graph knows their names but not their frames. Those are read off the instructions: every
  push, every `rcall .+0` (avr-gcc's way to reserve a small frame), and the frame a prologue
  cuts out of the stack pointer, plus what they call. And a call into libgcc that the graph
  does not list at all: avr-gcc calls its helpers, as __udivmodsi4 for a division of 32 bits,
  only after it wrote the graph, so each function's own instructions are read for them too.

A call pushes its return address: two bytes, or three on a part with more than 128 KiB of
flash, as avr-gcc's __AVR_3_BYTE_PC__ says of it.

Against the RAM the part has, from avr-libc's header for it, minus `.data` and `.bss` as
avr-size reports them. There is no heap to subtract: this fails if the program calls malloc(),
as a heap is exactly what no static analysis can bound.

It refuses rather than guesses where the sum would not hold: recursion, a call through a
pointer the graph cannot follow, a frame of dynamic size, a call into code it cannot place.
Each would make the number a lower bound, and a lower bound is the one answer this question
cannot use.

Out of Wordclock's tools/avr-stack.py, for the AVR128DA48 alone, made general.

Usage:
    tools/avr_stack.py --mcu atmega328p PROGRAM.elf GRAPH... [--paths]

GRAPH is a .ci file or a folder searched for them.
"""

import argparse
import bisect
import re
import subprocess
import sys
from pathlib import Path

# What a call pushes: set by main() for the part, two bytes or three.
RETURN_ADDRESS_BYTES = 2

NODE = re.compile(r'node: \{ title: "([^"]+)" label: "([^"]*)"')
EDGE = re.compile(r'edge: \{ sourcename: "([^"]+)" targetname: "([^"]+)"')
FRAME = re.compile(r'(\d+) bytes \((\w+)')
CHUNK = re.compile(r'^([0-9a-f]+) <(.+)>:$')
INSN = re.compile(r'^\s+[0-9a-f]+:\t(\S+)\t?(.*)$')
TARGET_ADDRESS = re.compile(r'; 0x([0-9a-f]+) <')


def fail(message):
    raise SystemExit(f'avr-stack.py: {message}')


def run(command):
    return subprocess.run(command, capture_output=True, text=True, check=True).stdout


def symbol_name(title):
    """The call graph prefixes a function of the image with its object file; a library one not."""
    return title.split(':')[-1]


class CallGraph:
    """The functions LTO compiled, with their frames, and the calls between them."""

    def __init__(self, paths):
        self.frames = {}
        self.labels = {}
        self.calls = {}
        for path in paths:
            text = path.read_text()
            for title, label in NODE.findall(text):
                self.labels[title] = label.split('\\n')[0]
                frame = FRAME.search(label)
                if frame is None:
                    continue
                if frame.group(2) != 'static':
                    fail(f'{self.labels[title]} has a frame of {frame.group(2)} size')
                self.frames[title] = int(frame.group(1))
            for source, target in EDGE.findall(text):
                self.calls.setdefault(source, []).append(target)
        if not self.frames:
            fail('the call graph is empty - was the program compiled with -fcallgraph-info=su?')


class Library:
    """The image's code as avr-objdump prints it, cut at every symbol into chunks.

    By address and not by name: local labels such as .L2 are in hundreds of functions, and a
    chunk found by its name would be somebody else's. A jump's target is the address objdump
    writes beside it.
    """

    def __init__(self, elf):
        self.chunks = {}
        self.names = {}
        current = None
        for line in run(['avr-objdump', '-d', '--no-show-raw-insn', str(elf)]).splitlines():
            header = CHUNK.match(line)
            if header:
                current = int(header.group(1), 16)
                self.chunks[current] = []
                self.names.setdefault(header.group(2), current)
                continue
            insn = INSN.match(line)
            if insn and current is not None:
                self.chunks[current].append((insn.group(1), insn.group(2)))
        self.starts = sorted(self.chunks)
        self.name_at = {address: name for name, address in self.names.items()}
        self.depths = {}

    def chunk_at(self, address):
        """The chunk an address falls in, a jump into the middle of one included."""
        index = bisect.bisect_right(self.starts, address) - 1
        return self.starts[index] if index >= 0 else None

    def frame(self, chunk):
        """What a chunk itself puts on the stack: its pushes, its rcall .+0 and the frame a
        prologue cuts out of the stack pointer - the sbiw or subi/sbci on r28 that follows
        reading it with in r28, 0x3d. An epilogue gives the frame back without reading the
        stack pointer first, so it is not counted a second time."""
        size = 0
        reading_frame = False
        for mnemonic, operands in self.chunks[chunk]:
            immediate = operands.split(',')[1].split()[0] if ',' in operands else None
            if mnemonic == 'push':
                size += 1
            elif (mnemonic == 'rcall') and operands.startswith('.+0'):
                size += 2
            elif (mnemonic == 'in') and operands.startswith('r28, 0x3d'):
                reading_frame = True
            elif reading_frame and (mnemonic in ('sbiw', 'subi')) and operands.startswith('r28'):
                size += int(immediate, 0) & 0xFF
                reading_frame = (mnemonic == 'subi')
            elif reading_frame and (mnemonic == 'sbci') and operands.startswith('r29'):
                size += (int(immediate, 0) & 0xFF) << 8
                reading_frame = False
        return size

    def successors(self, chunk):
        """Calls, which push a return address; jumps and the fall into the next chunk, which
        do not."""
        last = None
        for mnemonic, operands in self.chunks[chunk]:
            last = mnemonic
            if mnemonic in ('icall', 'eicall', 'ijmp', 'eijmp'):
                fail(f'library code at {chunk:#x} calls through a pointer')
            target = TARGET_ADDRESS.search(operands)
            if (target is None) or operands.startswith('.+0'):
                continue
            callee = self.chunk_at(int(target.group(1), 16))
            if (callee is None) or (callee == chunk):
                continue
            if mnemonic in ('call', 'rcall'):
                yield callee, RETURN_ADDRESS_BYTES
            elif mnemonic in ('jmp', 'rjmp') or mnemonic.startswith('br'):
                yield callee, 0
        if last not in ('jmp', 'rjmp', 'ret', 'reti', None):
            index = self.starts.index(chunk) + 1
            if index < len(self.starts):
                yield self.starts[index], 0

    def depth_of(self, name):
        """The deepest a library function goes, its return address left out."""
        return self.depth(self.names[name])

    def depth(self, chunk):
        """The deepest a chunk goes, its return address left out - the caller's to count.

        A loop in library code jumps back to a label, which is a chunk of its own, so the jumps
        between chunks have cycles. Each cycle is taken as one piece whose frame is the sum of
        all of its chunks' - more than any one way round it pushes, never less. A call inside
        such a piece is recursion, and refused.
        """
        if chunk not in self.depths:
            self.measure(chunk)
        return self.depths[chunk]

    def measure(self, root):
        """Tarjan's strongly connected components over the chunks below `root`, deepest first,
        so that every piece is measured after everything it leads to."""
        index, low, on_stack, stack, counter = {}, {}, set(), [], [0]
        edges = {}

        def visit(chunk):
            index[chunk] = low[chunk] = counter[0]
            counter[0] += 1
            stack.append(chunk)
            on_stack.add(chunk)
            edges[chunk] = list(self.successors(chunk))
            for callee, _ in edges[chunk]:
                if callee in self.depths:
                    continue
                if callee not in index:
                    visit(callee)
                    low[chunk] = min(low[chunk], low[callee])
                elif callee in on_stack:
                    low[chunk] = min(low[chunk], index[callee])
            if low[chunk] == index[chunk]:
                piece = []
                while True:
                    member = stack.pop()
                    on_stack.discard(member)
                    piece.append(member)
                    if member == chunk:
                        break
                frame = sum(self.frame(member) for member in piece)
                below = 0
                for member in piece:
                    for callee, pushed in edges[member]:
                        if callee in piece:
                            if pushed:
                                fail(f'library code at {member:#x} calls {callee:#x} recursively')
                            continue
                        below = max(below, pushed + self.depths[callee])
                for member in piece:
                    self.depths[member] = frame + below

        visit(root)


class Stack:
    """The deepest stack below each function, through the call graph and into the library."""

    def __init__(self, graph, library, symbols):
        self.graph = graph
        self.library = library
        self.by_name = {symbol_name(title): title for title in graph.frames}
        self.by_address = {}
        for title in graph.frames:
            address = symbols.get(symbol_name(title))
            if address is not None:
                self.by_address.setdefault(address, title)
        self.symbols = symbols
        self.depths = {}
        self.deepest_callee = {}

    def resolve(self, target):
        """Where a call goes. Identical functions are folded into one, and the graph then
        keeps the node of one of them while calls still name the others; the address they
        share in the image tells which node is meant."""
        if target in self.graph.frames:
            return target
        name = symbol_name(target)
        if name in self.by_name:
            return self.by_name[name]
        address = self.symbols.get(name)
        if address is not None and address in self.by_address:
            return self.by_address[address]
        if name in self.library.names:
            return None
        if 'indirect' in name:
            fail('a call goes through a pointer, and the graph cannot say where')
        # Named in the graph, not in the image: a builtin the compiler expanded in place.
        return ''

    def unlisted_calls(self, title):
        """The library functions a function calls that its call graph does not list: avr-gcc
        calls libgcc's helpers, as __udivmodsi4 for a division of 32 bits, only after it wrote the
        graph. Read off the function's own instructions, as names of the library."""
        address = self.symbols.get(symbol_name(title))
        if address not in self.library.chunks:
            return []
        names = []
        for mnemonic, operands in self.library.chunks[address]:
            target = TARGET_ADDRESS.search(operands)
            if mnemonic not in ('call', 'rcall') or target is None or operands.startswith('.+0'):
                continue
            callee = self.library.chunk_at(int(target.group(1), 16))
            if callee is None or callee == address or callee in self.by_address:
                continue
            names.append(self.library.name_at[callee])
        return names

    def depth(self, title, active=()):
        if title in self.depths:
            return self.depths[title]
        if title in active:
            fail(f'{self.graph.labels[title]} is recursive: ' + ' -> '.join(self.graph.labels[t] for t in active))
        deepest, deepest_callee = 0, None
        for target in self.graph.calls.get(title, []) + self.unlisted_calls(title):
            callee = self.resolve(target)
            if callee == '':
                continue
            if callee is None:
                below = RETURN_ADDRESS_BYTES + self.library.depth_of(symbol_name(target))
                callee = symbol_name(target)
            else:
                below = self.depth(callee, active + (title,))
            if below > deepest:
                deepest, deepest_callee = below, callee
        self.depths[title] = self.graph.frames[title] + deepest
        self.deepest_callee[title] = deepest_callee
        return self.depths[title]

    def path(self, title):
        """The frames along the deepest path below `title`, as (function, bytes) pairs."""
        steps = []
        while title is not None:
            if title in self.graph.frames:
                steps.append((self.graph.labels[title], self.graph.frames[title]))
                title = self.deepest_callee.get(title)
            else:
                steps.append((title + ' (library)', RETURN_ADDRESS_BYTES + self.library.depth_of(title)))
                title = None
        return steps


def preprocess(mcu, text):
    """`text` as avr-gcc's preprocessor leaves it for the part, avr/io.h included."""
    return subprocess.run(['avr-gcc', f'-mmcu={mcu}', '-E', '-P', '-include', 'avr/io.h', '-x',
                           'c', '-'], input=text + '\n', capture_output=True, text=True,
                          check=True).stdout.strip().splitlines()[-1]


def device_ram(mcu):
    """What the part has, as avr-libc's header for it says - not a number kept here: from
    RAMSTART to RAMEND, which every part's header defines, as INTERNAL_SRAM_SIZE only some do."""
    expression = preprocess(mcu, '(RAMEND) - (RAMSTART) + 1')
    expression = re.sub(r'(?<=[0-9a-fA-F])[uUlL]+\b', '', expression)
    if not re.fullmatch(r'[0-9a-fA-Fx()+\-\s]+', expression):
        fail(f'avr-libc does not say how much RAM {mcu} has: {expression}')
    return int(eval(expression, {'__builtins__': {}}))  # noqa: S307 - numbers, + and - only


def predefined(mcu, macro):
    """Whether avr-gcc predefines `macro` for the part."""
    return preprocess(mcu, f'#ifdef {macro}\nyes\n#else\nno\n#endif') == 'yes'


def used_ram(elf, mcu):
    """.data and .bss, as the link rule prints them under Data."""
    out = run(['avr-size', '--format=avr', f'--mcu={mcu}', str(elf)])
    data = re.search(r'Data:\s+(\d+) bytes', out)
    if data is None:
        fail(f'avr-size said something unexpected:\n{out}')
    return int(data.group(1))


def image_symbols(elf):
    symbols = {}
    for line in run(['avr-nm', '--defined-only', str(elf)]).splitlines():
        parts = line.split()
        if len(parts) == 3:
            symbols[parts[2]] = int(parts[0], 16)
    return symbols


def graph_files(paths):
    """The .ci files named, and those below the folders named."""
    files = []
    for path in paths:
        files += sorted(path.rglob('*.ci')) if path.is_dir() else [path]
    return files


def main():
    global RETURN_ADDRESS_BYTES  # noqa: PLW0603 - the part's, set once before anything reads it
    parser = argparse.ArgumentParser(description=__doc__,
                                     formatter_class=argparse.RawDescriptionHelpFormatter)
    parser.add_argument('--mcu', required=True, help="the part, as avr-gcc's -mmcu names it")
    parser.add_argument('elf', type=Path, help='the program')
    parser.add_argument('graphs', type=Path, nargs='+', help='its .ci files, or folders of them')
    parser.add_argument('--paths', action='store_true', help='print the deepest paths frame by frame')
    args = parser.parse_args()

    if not args.elf.is_file():
        fail(f'no program at {args.elf} - build first')
    graphs = graph_files(args.graphs)
    if not graphs:
        fail(f'no call graph for {args.elf} - it needs -fstack-usage -fcallgraph-info=su')

    symbols = image_symbols(args.elf)
    if 'malloc' in symbols:
        fail('the program calls malloc(): a heap grows towards the stack, and nothing here bounds it')

    if predefined(args.mcu, '__AVR_3_BYTE_PC__'):
        RETURN_ADDRESS_BYTES = 3
    nested_handlers = 2 if predefined(args.mcu, '__AVR_XMEGA__') else 1

    graph = CallGraph(graphs)
    library = Library(args.elf)
    stack = Stack(graph, library, symbols)

    entries = [t for t in graph.frames
               if symbol_name(t) == 'main' or symbol_name(t).startswith('_GLOBAL__sub_I')]
    handlers = [t for t in graph.frames if re.fullmatch(r'__vector_\d+', symbol_name(t))]
    if not any(symbol_name(t) == 'main' for t in entries):
        fail('main() is not in the call graph')
    if nested_handlers == 1:
        for handler in handlers:
            name = symbol_name(handler)
            if name in library.names and any(mnemonic == 'sei' for mnemonic, _ in
                                             library.chunks[library.names[name]]):
                fail(f'{name} sets the interrupt flag, and handlers can then nest without a bound')

    deepest_entry = max(entries, key=stack.depth)
    deepest_handlers = sorted(handlers, key=stack.depth, reverse=True)[:nested_handlers]
    worst = stack.depth(deepest_entry) + sum(stack.depth(t) for t in deepest_handlers)
    ram = device_ram(args.mcu)
    used = used_ram(args.elf, args.mcu)
    free = ram - used

    print(f'{graph.labels[deepest_entry]}: {stack.depth(deepest_entry)} bytes at the deepest')
    for handler in deepest_handlers:
        print(f'{graph.labels[handler]}: {stack.depth(handler)} bytes on top')
    print(f'worst case {worst} bytes of {free} free ({ram} on the {args.mcu} '
          f'less {used} of .data and .bss): {free - worst} to spare')

    if args.paths:
        for title in [deepest_entry] + deepest_handlers:
            print(f'\n{graph.labels[title]}:')
            for function, size in stack.path(title):
                print(f'  {size:5d}  {function}')

    if worst > free:
        print('\nThe stack can run into the variables.', file=sys.stderr)
        return 1
    return 0


if __name__ == '__main__':
    sys.exit(main())
