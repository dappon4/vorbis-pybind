#include <pybind11/pybind11.h>
#include <ogg/ogg.h>

int test_ogg_link() {
    ogg_stream_state os;
    ogg_packet op;
    return ogg_stream_packetin(&os, &op);
}

PYBIND11_MODULE(test_ogg_link, m) {
    m.def("test_ogg_link", &test_ogg_link);
}
