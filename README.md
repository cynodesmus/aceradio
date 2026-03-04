# Music Generator GUI

A Qt-based graphical user interface for generating music using acestep.cpp.

## Features

- **Song List Management**: Add, edit, and remove songs with captions and optional lyrics
- **Playback Controls**: Play, skip, and shuffle functionality
- **Settings Tab**: Customize the JSON template for AceStep generation parameters
- **Progress Tracking**: Visual progress bar during music generation
- **Seamless Playback**: Automatically generates and plays the next song when current one finishes

## Requirements

- Qt 5 or Qt 6 (with Core, Gui, Widgets, and Multimedia modules)
- CMake 3.14+
- acestep.cpp properly built with models downloaded

## Building

### Build acestep.cpp first:

```bash
cd acestep.cpp
git submodule update --init
mkdir build && cd build
cmake .. -DGGML_BLAS=ON  # or other backend options
cmake --build . --config Release -j$(nproc)
./models.sh  # Download models (requires ~7.7 GB free space)
```

### Build the GUI application:

```bash
cd ..
mkdir build && cd build
cmake ..
cmake --build . --config Release -j$(nproc)
```

## Usage

1. **Add Songs**: Click "Add Song" to create new song entries with captions and optional lyrics
2. **Edit Songs**: Select a song and click "Edit Song" to modify it
3. **Remove Songs**: Select a song and click "Remove Song" to delete it
4. **Play Music**: Click "Play" to start generating and playing music from the selected song or first song in the list
5. **Skip Songs**: Click "Skip" to move to the next song immediately
6. **Shuffle Mode**: Toggle "Shuffle" to play songs in random order
7. **Settings**: Click "Settings" in the menu bar to edit the JSON template for generation parameters

## Settings (JSON Template)

The JSON template allows you to customize AceStep generation parameters:

```json
{
    "inference_steps": 8,
    "shift": 3.0,
    "vocal_language": "en",
    "lm_temperature": 0.85,
    "lm_cfg_scale": 2.0,
    "lm_top_p": 0.9
}
```

Available fields:
- `caption` (required, will be overridden by song entry)
- `lyrics` (optional, can be empty to let LLM generate)
- `instrumental` (boolean)
- `bpm` (integer)
- `duration` (float in seconds)
- `keyscale` (string like "C major")
- `timesignature` (string like "4/4")
- `vocal_language` (string like "en", "fr", etc.)
- `seed` (integer for reproducibility)
- `lm_temperature`, `lm_cfg_scale`, `lm_top_p`, `lm_top_k` (LM generation parameters)
- `lm_negative_prompt` (string)
- `audio_codes` (string, for advanced users)
- `inference_steps` (integer)
- `guidance_scale` (float)
- `shift` (float)

## Notes

- The first time you generate a song, it may take several minutes as the models load into memory
- Generated WAV files are created in your system's temporary directory and played immediately
- Shuffle mode uses simple random selection without replacement within a playback session
- Skip button works even during generation - it will wait for current generation to finish then play next song
