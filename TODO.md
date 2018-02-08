Faulhaber Driver TODO
======================

Depedendcies
------------

You'll need cmake, g++, googletest, boost, and git to build and work on this.  On ubuntu:
```
$ sudo apt-get install cmake g++ googletest libboost-all-dev git
$ cd ~
$ mkdir googletest
$ cd googletest
$ cmake /usr/src/googletest
$ make && sudo make install
```
And then in the repo root you should be good to:
```
./build
```
which will run a cmake build in the `.build` directory.

To Implement
------------

Implement tools for opening up communication with, and communicating with a `Faulhaber QUICKSHAFT LM0830-040-01`
controlled over RS-232 via a `Faulhaber MCLM3006 S RS` motion controller (see communication manual in `references/`).
Initially we only want to expose basic functionality (home, set position, get state), but we want
it to be really easy to add in new Faulhaber motion controller command implementations as needed
(eg: some day we may want constant velocity mode, or the ability to configure all the different settings, etc).

Once the low level protocol tools are implemented, wrap them in an easy to use Faulhaber Client class that will
take care of setting up the serial connection, instantiating and sending low level commands, etc.  AKA a Client
class that gives a nice high level interface like:
```
namespace faulhaber {
  struct State {
    State(bool _homed, bool _moving, units::Length _position, units::Velocity _velocity) :
      homed(_homed), moving(_moving), position(_position), velocity(_velocity) {};

    bool homed;
    bool moving;

    units::Length position;
    units::Velocity velocity;
  }

  class Client {
    Client(std::string serial_device);

    // Start an asynchronous homing operation (kick off homing and return)
    //
    // Timeout is the maximum time to wait until assuming the starting homing
    // operation has failed (not the maximum time to allow for homing).
    util::ErrorT StartHoming(double timeout);

    // Start an asynchrounous move to the given position.
    util::ErrorT StartMoveTo(units::Length position, double timeout);

    // Return the current state.  It should be possible for users to poll this
    // to see when StartHoming(...), StartMoveTo(...), etc have completed
    std::unique_ptr<State> GetState(double timeout)

    // A helper to do a synchronous homing operation that does not return
    // until homing completes (or fails or times out)
    util::ErrorT Home(double timeout);

    // A helper to do a synchronous move that does not return until
    // the move completes (or fails or times out)
    util::Error MoveTo(units::Length position, double timeout);
  }
}
```

Code Style
----------

Use google's C++ style guidelines as best as possible.  Don't fret too much over this - consistency and clarity
is most important.

Deliverable
-----------

The above implemented in the form of this git repository.  You should also fill out the `debug_tool.cpp` file with a
CLI interface that allows us to test and debug faulhauber axes quickly.  EG, it should allow us to home an axis,
move an axis to a position, and query an axis' state.
