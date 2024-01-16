# Maschine MK1 Relay

This application communicates with the [Maschine MK1](https://en.wikipedia.org/wiki/Maschine) hardware and relays pad presses and encoder movements as [MIDI](https://en.wikipedia.org/wiki/MIDI) messages, and button presses as [OSC](https://en.wikipedia.org/wiki/Open_Sound_Control) messages. It is built on the modified version of the [cabl](https://github.com/shaduzlabs/cabl) library. It is tested on macOS only but might work on other platforms.

* Pads are sent as MIDI Note ON, MIDI Note OFF and Aftertouch in between
* Encoders are sent as MIDI CC
* Button presses are sent as OSC messages with addresses `/MMK1/*` or `/MMK1/Shift/*` and values `on` and `off`

## Usage

Check out command line arguments and their descriptions.

```
./mmk1-relay -h
```

## Installation

To begin, clone and build the [modified version](https://github.com/meownoid/cabl) of the cabl library.

Next, make sure to install required dependencies.

```
brew install rtmidi boost cxxopts
```

Clone the repository alongside the `cabl` (so both `cabl` and `mmk1-relay` are in the same directory) and proceed to build the project.

```
git clone https://github.com/meownoid/mmk1-relay.git
cd mmk1-relay
```

```
mkdir build && cd build
cmake ..
make
```

Finally, you can relocate the binary `build/mmk1-relay` to the desired location.
