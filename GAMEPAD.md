# Adding gamepad support

Gamepads aren't implemented yet, but the app is structured so that adding one
means writing a single new input class. Nothing else needs to change.

## How input flows

```
 touch sticks ─┐
 keyboard     ─┼─> FlightInput ──(sampled every 50 ms)──> DroneLink ──UDP──> drone
 gamepad (new)─┘     axes + actions
```

* **`FlightInput`** (`src/flightinput.h`) is the only entry point for pilot
  input. It is device-agnostic:
  * `setAxes(source, yaw, throttle, roll, pitch)`, all normalized −1..+1
  * `trigger(action)` for buttons, using the `FlightInput::Action` enum
    (TakeOff, Land, TakeOffOrLand, EmergencyStop, ArmFlip, SwitchCamera,
    SpeedUp/Down, ToggleArmed, trims, …)
  * `releaseSource(source)` when a device goes away
* **`DroneLink`** never looks at devices. It reads `FlightInput::axes()` on its
  own 50 ms timer, so a gamepad reporting at 250 Hz and touch at 60 Hz produce
  identical drone traffic.
* **Arbitration**: the most recently *deliberately moved* source wins. A source
  only takes over when it pushes an axis more than 0.15 from rest, so a gamepad
  lying on the table with stick drift can't fight the touch sticks.
* **The on-screen sticks mirror whatever is being commanded** when not touched,
  so gamepad input will be visible on screen with no extra work.
* **`KeyboardInput.qml`** is already a second, non-touch source built this way.
  It's the template for a gamepad backend.

## A gamepad backend, sketched

```cpp
class GamepadInput : public QObject {
    // ...open the device, then on every event:
    void onAxes(float lx, float ly, float rx, float ry) {
        m_input->setAxes(FlightInput::Gamepad,
                         deadzone(lx), throttleFrom(ly),   // Mode 2
                         deadzone(rx), deadzone(-ry));
    }
    void onButton(int button, bool pressed) {
        if (!pressed) return;
        switch (button) {
        case BtnA:     m_input->trigger(FlightInput::TakeOff); break;
        case BtnB:     m_input->trigger(FlightInput::Land); break;
        case BtnX:     m_input->trigger(FlightInput::ArmFlip); break;
        case BtnY:     m_input->trigger(FlightInput::SwitchCamera); break;
        case BtnStart: m_input->trigger(FlightInput::ToggleArmed); break;
        case BtnL1:    m_input->trigger(FlightInput::SpeedDown); break;
        case BtnR1:    m_input->trigger(FlightInput::SpeedUp); break;
        }
        // Emergency stop: use a deliberate combo (e.g. both triggers) so it
        // can't be hit by accident, but is still one motion away.
    }
    void onDisconnected() { m_input->releaseSource(FlightInput::Gamepad); }
};
```

Things the backend should own (not FlightInput):

* **Dead zones.** Apply them in the backend; every pad is different.
* **Throttle on self-centring sticks.** With altitude hold, stick centre = hover
  and it maps directly. Without altitude hold, a sprung stick at centre would mean
  50% throttle, so either use an analog trigger as absolute throttle, or make the
  stick change throttle at a rate (integrate it), like `KeyboardInput.qml` does
  with W/S.
* **Disconnect behaviour.** `releaseSource` centres roll/pitch/yaw, and centres
  throttle when altitude hold is on, so a drone with altitude hold will hover. Consider
  also triggering `Land` on disconnect as an option.

## The real obstacle: confinement on Ubuntu Touch

Getting gamepad events into a **confined** click app is the uncertain part, and
it's a platform question rather than a code one:

* Bluetooth and USB pads show up as evdev devices (`/dev/input/event*`). Qt
  Gamepad, SDL and libevdev all read those directly, and **AppArmor does not let
  confined apps open them**. I'm not aware of a policy group that grants it;
  anything that does would likely need manual review.
* The likely confinement-safe path is the **compositor**: if Lomiri/Mir forwards
  gamepad input to the focused app, it arrives as ordinary Qt key events. Buttons
  may well arrive this way. Analog sticks probably won't, since key events
  carry no axis values.

**How to check on your phone:** pair the controller, open the app, enable
*Settings → Show debug info*, and press buttons. The last line of the overlay
shows every key event the app receives, with its Qt key code and native scan
code. If buttons show up there, mapping them to actions in `KeyboardInput.qml`
is a few lines. If nothing shows up, events aren't reaching confined apps, and
full gamepad support would need either a platform-level input path or an
unconfined build (not submittable without manual review).

On the desktop build there is no confinement, so a Qt Gamepad, SDL or evdev
backend works there today, which is handy for development.
