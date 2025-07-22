# vorbis_pybind

Python bindings for the Vorbis audio codec using pybind11. This project allows you to quantize PCM audio, inspect the quantized MDCT coefficients, and generate Ogg Vorbis files directly from Python.

## Features
- Load WAV files and convert to normalized PCM (float32)
- Quantize PCM audio to Vorbis encoder state (MDCT extraction)
- Inspect quantized MDCT coefficients and encoder state in Python
- Generate Ogg Vorbis files from quantized state
- All bindings use the original Vorbis C codebase

## Requirements
- Python 3.11+
- pybind11
- numpy
- setuptools
- libogg (system library)

## Build Instructions
1. **Install dependencies:**
   ```sh
   pip install pybind11 numpy setuptools librosa
   sudo apt-get install libogg-dev  # or your system's package manager
   ```

2. **Build the extension:**
   ```sh
   python3 setup.py build
   ```
   After building, the shared object (.so) file will be automatically copied to the project root for easy import.

3. **Run the test script:**
   ```sh
   python3 test_vorbis_pybind.py --input your.wav --output out.ogg
   ```
   The script will:
   - Load a WAV file
   - Quantize the PCM audio
   - Show MDCT blocks and encoder state
   - Generate an Ogg Vorbis file

## Usage Example
```python
import vorbis_pybind
import librosa

# Load WAV file
channels = 1  # Set to 2 for stereo
pcm, rate = librosa.load("cat.wav", sr=44100, mono=(channels == 1))

# Quantize PCM
state = vorbis_pybind.pcm_to_quantized_state(pcm, channels, rate)
print('MDCT blocks:', [block.shape for block in state.mdct_coeffs])

# Generate Ogg Vorbis file
ogg_bytes = vorbis_pybind.quantized_state_to_ogg(state.capsule)
with open('output.ogg', 'wb') as f:
    f.write(ogg_bytes)
```

## Project Structure
- `lib/` — Vorbis C source code and pybind11 wrapper
- `setup.py` — Build script
- `test_vorbis_pybind.py` — CLI test and debug script
- `vorbis_pybind.cpython-311-x86_64-linux-gnu.so` — Python extension (after build)

## Notes
- The build process automatically copies the extension to the project root for direct import.
- All quantization and Ogg generation logic uses the original Vorbis C implementation.
- You can inspect the quantized state, MDCT coefficients, and packets in Python for debugging and research.

## License
This project is provided for research and educational use. See the original Vorbis license for C source code details.
