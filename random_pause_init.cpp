#include "pch.h"
#include <ctime>
#include <SDK/foobar2000.h>

class random_pause_init : public initquit {
public:
    void on_init() override {
        std::srand(static_cast<unsigned>(std::time(nullptr)));
        FB2K_console_formatter() << "[RandomPause] Random generator seeded.";
    }

    void on_quit() override {
        // Optional: cleanup logic
    }
};

FB2K_SERVICE_FACTORY(random_pause_init);