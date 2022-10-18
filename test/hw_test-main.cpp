// custom-main.cpp
#define CATCH_CONFIG_RUNNER
#include <catch2/catch.hpp>

#include "hw_test.hpp"

std::string  tty      = "/dev/pts/0";
unsigned int baudrate = 115200;

int main(int argc, char* argv[])
{
    Catch::Session session = Catch::Session();

    using namespace Catch::clara;
    auto cli = session.cli() | Opt(tty, "tty")["-g"]["--terminal"]("tty to use") |
               Opt(baudrate, "baudrate")["-j"]["--baudrate"]("Baudrate");

    session.cli(cli);

    int ret = session.applyCommandLine(argc, argv);
    if (ret != 0)
    {
        return ret;
    }

    return session.run();
}
