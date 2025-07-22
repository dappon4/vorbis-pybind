
import numpy as np
import vorbis_pybind
import argparse
import os
import librosa

def main():
    parser = argparse.ArgumentParser(description="Test vorbis_pybind with debug options.")
    parser.add_argument('--input', type=str, default=None, help='Input PCM .npy file (shape NxC, float32)')
    parser.add_argument('--output', type=str, default='test_output.ogg', help='Output Ogg file name')
    parser.add_argument('--rate', type=int, default=44100, help='Sample rate')
    parser.add_argument('--channels', type=int, default=2, help='Number of channels')
    parser.add_argument('--duration', type=float, default=0.1, help='Duration (s) if generating test signal')
    args = parser.parse_args()

    if args.input and os.path.exists(args.input):
        # Use the bound function to load WAV and convert to PCM
        # pcm, channels, rate = vorbis_pybind.wav_file_to_pcm(args.input)
        pcm, rate = librosa.load(args.input, sr=args.rate, mono=(args.channels == 1))
        pcm = pcm.T  # Transpose to shape (N, C)
        channels = 1 if pcm.ndim == 1 else pcm.shape[1]
        print(f"Loaded WAV and converted to PCM: shape={pcm.shape}, channels={channels}, rate={rate}")
    else:
        # Generate a short test PCM buffer: 1kHz sine wave
        t = np.linspace(0, args.duration, int(args.rate * args.duration), endpoint=False)
        signal = 0.5 * np.sin(2 * np.pi * 1000 * t)
        pcm = np.stack([signal]*args.channels, axis=1).astype(np.float32)
        channels = args.channels
        rate = args.rate
        print(f"Generated test sine wave: shape={pcm.shape}, dtype={pcm.dtype}")

    # Test 1: PCM to quantized state (MDCT extraction only)
    state = vorbis_pybind.pcm_to_quantized_state(pcm, channels, rate)

    print("--- QuantizedState (MDCT only) ---")
    print("  channels:", state.channels)
    print("  rate:", state.rate)
    print("  num_headers:", state.num_headers)
    print("  mdct_coeffs blocks:")
    for i, block in enumerate(state.mdct_coeffs):
        print(f"    Block {i}: shape={block.shape}, first 10 coeffs={block[:10]}")

    # Test 2: Generate Ogg from quantized state
    ogg_bytes = vorbis_pybind.quantized_state_to_ogg(state.capsule)
    with open(args.output, "wb") as f:
        f.write(ogg_bytes)
    print(f"Test complete. Ogg file written as {args.output}")

    # Test 3: Check Ogg packet count and bytes (after Ogg generation)
    # The quantized_state_to_ogg function updates the packets in-place
    print("--- After Ogg Generation ---")
    print("  num_packets:", state.num_packets)
    if state.num_packets > 0:
        print("  quantized_bytes (first 2 packets):")
        for i, pkt in enumerate(state.quantized_bytes[:2]):
            print(f"    Packet {i}: len={len(pkt)} bytes, first 16 bytes={pkt[:16]}")
    else:
        print("  No packets generated. If this persists, check the C++ binding and ensure the same QuantizedState object is used.")

    # Test 4: Edge case - empty PCM
    print("--- Edge Case: Empty PCM ---")
    try:
        empty_pcm = np.zeros((0, channels), dtype=np.float32)
        empty_state = vorbis_pybind.pcm_to_quantized_state(empty_pcm, channels, rate)
        print("  Empty state mdct_coeffs:", empty_state.mdct_coeffs)
        empty_ogg = vorbis_pybind.quantized_state_to_ogg(empty_state.capsule)
        print("  Empty Ogg file size:", len(empty_ogg))
    except Exception as e:
        print("  Exception for empty PCM:", e)

    # Test 5: Single channel, short duration
    print("--- Test: Single Channel, Short Duration ---")
    t = np.linspace(0, 0.01, int(rate * 0.01), endpoint=False)
    signal = 0.5 * np.sin(2 * np.pi * 440 * t)
    pcm1 = signal.astype(np.float32).reshape(-1, 1)
    state1 = vorbis_pybind.pcm_to_quantized_state(pcm1, 1, rate)
    print("  mdct_coeffs blocks:", [block.shape for block in state1.mdct_coeffs])
    ogg1 = vorbis_pybind.quantized_state_to_ogg(state1.capsule)
    print("  Ogg file size:", len(ogg1))

if __name__ == "__main__":
    main()
