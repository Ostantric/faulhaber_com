Faulhaber Driver
======================

Depedendcies
------------

cmake, g++, googletest, boost, and git:
```
$ sudo apt-get install cmake g++ googletest libboost-all-dev git
$ cd ~
$ mkdir googletest
$ cd googletest
$ cmake /usr/src/googletest
$ make && sudo make install
```
repo:
```
./build
```
which will run a cmake build in the `.build` directory.

Summary
------------

Implementing tools for opening up communication with, and communicating with a `Faulhaber QUICKSHAFT LM0830-040-01`
controlled over RS-232 via a `Faulhaber MCLM3006 S RS` motion controller (see communication manual in `references/`).

-home, set position, get state

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

    // Return the current state.
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

CLI
-----------

`debug_tool.cpp` file implements a CLI interface that allows us to test and debug faulhauber axes quickly.
