#pragma once

struct Event {
    int seed;
    bool clock;
    bool globalReset;
    bool seedChanged;

    bool processed;

    Event() : seed(0), clock(false), globalReset(false), seedChanged(true), processed(false) {}
};
