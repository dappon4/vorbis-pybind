
#include <pybind11/pybind11.h>
#include <pybind11/numpy.h>
#include <pybind11/stl.h>
#include <vector>
#include <string>
#ifdef __cplusplus
extern "C" {
#endif
#include "wav_to_pcm.h"
#ifdef __cplusplus
}
#endif
#ifdef __cplusplus
extern "C" {
#endif
#include <ogg/ogg.h>
#include "vorbis/codec.h"
#include "vorbis/vorbisenc.h"
#include <stdexcept>
#ifdef __cplusplus
}
#endif

namespace py = pybind11;

// Helper: load WAV file and return numpy array, channels, rate
py::tuple wav_file_to_pcm(const std::string& filename) {
    float* pcm = nullptr;
    int channels = 0, rate = 0, frames = 0;
    size_t nframes = wav_to_pcm(filename.c_str(), &pcm, &channels, &rate, &frames);
    if (!pcm || nframes == 0) throw std::runtime_error("Failed to load WAV file");
    // shape: (frames, channels)
    py::array_t<float> arr({frames, channels});
    float* arr_ptr = static_cast<float*>(arr.request().ptr);
    std::memcpy(arr_ptr, pcm, sizeof(float) * frames * channels);
    free(pcm);
    return py::make_tuple(arr, channels, rate);
}


// Helper struct to hold encoder state and packets
struct QuantizedState {
    vorbis_info vi;
    vorbis_dsp_state vd;
    vorbis_block vb;
    std::vector<ogg_packet> headers;
    std::vector<ogg_packet> packets;
    int channels;
    int rate;
    std::vector<std::vector<float>> mdct_coeffs; // [block][coeff]
    ~QuantizedState() {
        vorbis_block_clear(&vb);
        vorbis_dsp_clear(&vd);
        vorbis_info_clear(&vi);
    }
};

// Function 1: PCM to quantized state
std::unique_ptr<QuantizedState> pcm_to_quantized_state(py::array_t<float> pcm, int channels, int rate) {
    auto state = std::make_unique<QuantizedState>();
    state->channels = channels;
    state->rate = rate;
    vorbis_info_init(&state->vi);
    if (vorbis_encode_init_vbr(&state->vi, channels, rate, 0.4f)) {
        throw std::runtime_error("vorbis_encode_init_vbr failed");
    }
    vorbis_analysis_init(&state->vd, &state->vi);
    vorbis_block_init(&state->vd, &state->vb);

    // Generate headers
    ogg_packet header, header_comm, header_code;
    vorbis_comment vc;
    vorbis_comment_init(&vc);
    vorbis_comment_add_tag(&vc, "ENCODER", "pybind11-vorbis");
    vorbis_analysis_headerout(&state->vd, &vc, &header, &header_comm, &header_code);
    state->headers = {header, header_comm, header_code};
    vorbis_comment_clear(&vc);

    // Copy PCM data
    float **buffer = vorbis_analysis_buffer(&state->vd, pcm.shape(0));
    auto pcm_ptr = pcm.unchecked<2>();
    for (ssize_t c = 0; c < channels; ++c)
        for (ssize_t i = 0; i < pcm.shape(0); ++i)
            buffer[c][i] = pcm_ptr(i, c);
    vorbis_analysis_wrote(&state->vd, pcm.shape(0));

    // Process all available blocks and extract MDCT coefficients only
    while (vorbis_analysis_blockout(&state->vd, &state->vb) == 1) {
        int n = state->vb.pcmend;
        std::vector<float> block_mdct;
        for (int c = 0; c < channels; ++c) {
            float* coeffs = state->vb.pcm[c];
            block_mdct.insert(block_mdct.end(), coeffs, coeffs + n);
        }
        state->mdct_coeffs.push_back(std::move(block_mdct));
        // Do NOT create ogg packets here
    }
    // Do NOT signal end-of-stream or flush packets here
    return state;
}

// Function 2: Quantized state to Ogg file
py::bytes quantized_state_to_ogg(py::capsule state_capsule) {
    auto state = static_cast<QuantizedState*>(state_capsule);
    // Signal end-of-stream and flush all blocks and packets
    vorbis_analysis_wrote(&state->vd, 0); // 0 samples signals end-of-stream
    state->packets.clear();
    while (vorbis_analysis_blockout(&state->vd, &state->vb) == 1) {
        ogg_packet op;
        vorbis_analysis(&state->vb, &op);
        state->packets.push_back(op);
    }
    ogg_stream_state os;
    ogg_stream_init(&os, 12345); // Random serial
    std::string ogg_data;
    ogg_page og;
    // Write headers
    for (const auto& pkt : state->headers) {
        ogg_stream_packetin(&os, const_cast<ogg_packet*>(&pkt));
        while (ogg_stream_pageout(&os, &og)) {
            ogg_data.append(reinterpret_cast<const char*>(og.header), og.header_len);
            ogg_data.append(reinterpret_cast<const char*>(og.body), og.body_len);
        }
    }
    // Write audio packets
    for (const auto& pkt : state->packets) {
        ogg_stream_packetin(&os, const_cast<ogg_packet*>(&pkt));
        while (ogg_stream_pageout(&os, &og)) {
            ogg_data.append(reinterpret_cast<const char*>(og.header), og.header_len);
            ogg_data.append(reinterpret_cast<const char*>(og.body), og.body_len);
        }
    }
    // Flush remaining pages
    while (ogg_stream_flush(&os, &og)) {
        ogg_data.append(reinterpret_cast<const char*>(og.header), og.header_len);
        ogg_data.append(reinterpret_cast<const char*>(og.body), og.body_len);
    }
    ogg_stream_clear(&os);
    return py::bytes(ogg_data);
}


// Pybind11 module definition
PYBIND11_MODULE(vorbis_pybind, m) {

    m.def("wav_file_to_pcm", &wav_file_to_pcm, "Load a WAV file and return (pcm, channels, rate)", py::arg("filename"));
    py::class_<QuantizedState>(m, "QuantizedState")
        .def_property_readonly("channels", [](const QuantizedState& s) { return s.channels; })
        .def_property_readonly("rate", [](const QuantizedState& s) { return s.rate; })
        .def_property_readonly("num_headers", [](const QuantizedState& s) { return s.headers.size(); })
        .def_property_readonly("num_packets", [](const QuantizedState& s) { return s.packets.size(); })
        .def_property_readonly("headers", [](const QuantizedState& s) {
            std::vector<py::bytes> result;
            for (const auto& pkt : s.headers) {
                result.emplace_back(reinterpret_cast<const char*>(pkt.packet), pkt.bytes);
            }
            return result;
        })
        .def_property_readonly("packets", [](const QuantizedState& s) {
            std::vector<py::bytes> result;
            for (const auto& pkt : s.packets) {
                result.emplace_back(reinterpret_cast<const char*>(pkt.packet), pkt.bytes);
            }
            return result;
        })
        .def_property_readonly("quantized_bytes", [](const QuantizedState& s) {
            std::vector<py::bytes> result;
            for (const auto& pkt : s.packets) {
                result.emplace_back(reinterpret_cast<const char*>(pkt.packet), pkt.bytes);
            }
            return result;
        }, "List of quantized packet bytes (as produced by the Vorbis encoder)")
        .def_property_readonly("mdct_coeffs", [](const QuantizedState& s) {
            // Return as a list of numpy arrays, one per block
            py::list result;
            for (const auto& block : s.mdct_coeffs) {
                result.append(py::array_t<float>(block.size(), block.data()));
            }
            return result;
        }, "List of quantized MDCT coefficients (per block, concatenated channels)")
        .def_property_readonly("capsule", [](QuantizedState& self) {
            return py::capsule(&self, "QuantizedStatePtr");
        }, "Capsule pointer to the underlying QuantizedState object for use with quantized_state_to_ogg");

    m.def("pcm_to_quantized_state",
          static_cast<std::unique_ptr<QuantizedState>(*)(py::array_t<float>, int, int)>(&pcm_to_quantized_state),
          "Convert PCM to quantized Vorbis state",
          py::arg("pcm"), py::arg("channels"), py::arg("rate"));
    m.def("quantized_state_to_ogg", &quantized_state_to_ogg, "Convert quantized Vorbis state to Ogg file", py::arg("state"));
}
