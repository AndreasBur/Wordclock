#!/usr/bin/env python3
"""Checks that the AVR's deepest stack fits the RAM the image leaves free.

The AVR128DA48 has 16 KiB of RAM. `.data` and `.bss` take theirs from the bottom, the stack
grows down from the top, and nothing stops the one from running into the other: no MMU, no
guard, a corrupted variable somewhere and a clock that does something odd once a week. Every
other size here is checked by tools/documented-sizes.py; this one was measured by hand, when
somebody happened to notice the image move.

What it adds up, from what the LTO link writes beside the image when it is given
`-fstack-usage -fcallgraph-info=su` (platform/avr-dx/CMakeLists.txt does, and the image is
byte for byte the same with or without them):

- **The deepest path from main()**, each function's frame from the call graph, which counts
  its return address, its saved registers and its locals. The static constructors run before
  main() and are taken as entries of their own.
- **Plus the two deepest interrupt handlers.** A Dx part has two interrupt levels and one
  vector at the higher one (CPUINT.LVL1VEC), so at most two handlers are on the stack at once:
  one of level 0, and the level-1 one on top of it. Taking the two deepest, whichever they
  are, is never less than that and needs no knowledge of which vector is which.
- **Library code by its disassembly.** avr-libc and libgcc are linked as compiled, not through
  LTO, so the call graph knows their names but not their frames. Those are read off the
  instructions: every push, every `rcall .+0` (two bytes, avr-gcc's way to reserve a small
  frame), and the frame a prologue cuts out of the stack pointer, plus what they call.

Against the RAM the device has - its INTERNAL_SRAM_SIZE, asked of avr-gcc for the build's
AVR_MCU - minus `.data` and `.bss` as avr-size reports them. There is no heap to subtract:
nothing in the image calls malloc(), and this fails if something does, because a heap is
exactly what no static analysis can bound.

It refuses rather than guesses where the sum would not hold: recursion, a call through a
pointer the graph cannot follow, a frame of dynamic size, a call into code it cannot place.
Each of those would make the number a lower bound, and a lower bound is the one kind of
answer this question cannot use.

Usage:
    tools/avr-stack.py build-avr            # check, exit 1 if the stack does not fit
    tools/avr-stack.py build-avr --paths    # and print the deepest paths, frame by frame
"""

import argparse
import bisect
import re
import subprocess
import sys
from pathlib import Path

# A call pushes the return address: two bytes on a part whose flash is addressed in 16-bit
# words, which every AVR with at most 128 KiB is.
RETURN_ADDRESS_BYTES = 2
NESTED_HANDLERS = 2

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
            fail('the call graph is empty - was the image linked with -fcallgraph-info=su?')


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

    def depth(self, title, active=()):
        if title in self.depths:
            return self.depths[title]
        if title in active:
            fail(f'{self.graph.labels[title]} is recursive: ' + ' -> '.join(self.graph.labels[t] for t in active))
        deepest, deepest_callee = 0, None
        for target in self.graph.calls.get(title, []):
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


def cache_entry(build, key):
    for line in (build / 'CMakeCache.txt').read_text().splitlines():
        if line.startswith(key + ':'):
            return line.split('=', 1)[1]
    fail(f'{key} is not in {build}/CMakeCache.txt - is that an AVR build directory?')


def device_ram(mcu):
    """What the device has, as avr-libc's header for it says - not a number kept here."""
    out = subprocess.run(['avr-gcc', f'-mmcu={mcu}', '-E', '-P', '-include', 'avr/io.h', '-x', 'c', '-'],
                         input='INTERNAL_SRAM_SIZE\n', capture_output=True, text=True, check=True).stdout
    value = re.search(r'\((\d+)U?\)\s*$', out.strip())
    if value is None:
        fail(f'avr-libc does not say how much RAM {mcu} has')
    return int(value.group(1))


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


def main():
    parser = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    parser.add_argument('build', type=Path, help='the AVR build directory')
    parser.add_argument('--paths', action='store_true', help='print the deepest paths frame by frame')
    args = parser.parse_args()

    elf = args.build / 'platform' / 'avr-dx' / 'Wordclock.elf'
    graphs = sorted((args.build / 'platform' / 'avr-dx').glob('*.ci'))
    if not elf.is_file():
        fail(f'no image at {elf} - build first')
    if not graphs:
        fail(f'no call graph beside {elf} - the link needs -fcallgraph-info=su')

    symbols = image_symbols(elf)
    if 'malloc' in symbols:
        fail('the image calls malloc(): a heap grows towards the stack, and nothing here bounds it')

    mcu = cache_entry(args.build, 'AVR_MCU')
    graph = CallGraph(graphs)
    stack = Stack(graph, Library(elf), symbols)

    entries = [t for t in graph.frames if symbol_name(t) == 'main' or symbol_name(t).startswith('_GLOBAL__sub_I')]
    handlers = [t for t in graph.frames if re.fullmatch(r'__vector_\d+', symbol_name(t))]
    if not any(symbol_name(t) == 'main' for t in entries):
        fail('main() is not in the call graph')

    deepest_entry = max(entries, key=stack.depth)
    deepest_handlers = sorted(handlers, key=stack.depth, reverse=True)[:NESTED_HANDLERS]
    worst = stack.depth(deepest_entry) + sum(stack.depth(t) for t in deepest_handlers)
    free = device_ram(mcu) - used_ram(elf, mcu)

    print(f'{graph.labels[deepest_entry]}: {stack.depth(deepest_entry)} bytes at the deepest')
    for handler in deepest_handlers:
        print(f'{graph.labels[handler]}: {stack.depth(handler)} bytes on top')
    print(f'worst case {worst} bytes of {free} free ({device_ram(mcu)} on the {mcu} '
          f'less {used_ram(elf, mcu)} of .data and .bss): {free - worst} to spare')

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
