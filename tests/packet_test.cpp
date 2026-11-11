// Reads commands on stdin, prints results, for comparison against a Python port
// of the decompiled FlyController / DeviceBLFragment logic (packet_cases.py).
//   pkt <fmt> <roll> <pitch> <thr> <yaw> <takeoff> <land> <stop> <flip> <headless> <fastret> <light> <gyro> <althold>
//   map <yaw> <thr> <roll> <pitch> <speed> <trimR> <trimP> <trimY>
#include "../src/dronelink.h"
#include <QCoreApplication>
#include <iostream>
#include <sstream>
#include <string>

int main(int argc, char **argv)
{
    QCoreApplication app(argc, argv);
    std::string line;
    while (std::getline(std::cin, line)) {
        std::istringstream in(line);
        std::string cmd;
        in >> cmd;
        if (cmd == "pkt") {
            int fmt; DroneLink::ControlFrame f; int b[9];
            in >> fmt >> f.roll >> f.pitch >> f.throttle >> f.yaw;
            for (int &x : b) in >> x;
            f.takeOff = b[0]; f.land = b[1]; f.emergencyStop = b[2]; f.flip = b[3];
            f.headless = b[4]; f.fastReturn = b[5]; f.light = b[6]; f.gyro = b[7]; f.altitudeHold = b[8];
            std::cout << DroneLink::buildPacket(fmt, f).toHex().constData() << "\n";
        } else if (cmd == "map") {
            FlightInput::Axes a; int speed, tr, tp, ty;
            in >> a.yaw >> a.throttle >> a.roll >> a.pitch >> speed >> tr >> tp >> ty;
            DroneLink::ControlFrame f;
            DroneLink::mapAxes(a, speed, tr, tp, ty, f);
            std::cout << f.roll << " " << f.pitch << " " << f.throttle << " " << f.yaw << "\n";
        }
    }
    return 0;
}
