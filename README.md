# CursorEffect 1.0

CursorEffect is a lightweight Windows application that renders customizable particle trails behind the mouse cursor. It runs in the background and sits in the system tray.

## Features

- Particle trails with dots, custom image sprites or animated GIFs
- System tray integration with right-click menu
- Real-time configuration hot-reload on file save
- Color support with HEX (#RRGGBB or #RRGGBBAA) and RGBA values
- Dynamic skin detection in the skins folder
- Default configuration generated automatically on first run
- No console window during runtime

## Requirements

- Windows 10 or Windows 11
- C++20 compatible compiler (MSVC recommended)
- CMake 3.20 or newer

## Build Instructions

Clone the repository and build using CMake:

```powershell
cmake -B build
cmake --build build --config Release
```

The output executable `CursorEffect.exe` will be located inside the `build/Release` folder.

## Configuration

On the first launch, the program creates a configuration directory in your user profile:

```text
%USERPROFILE%\.cureff\
  ├── settings.cfg
  └── skins\
```

### Settings File (settings.cfg)

Edit `settings.cfg` using any text editor. The program updates settings automatically when you save the file.

| Setting | Default | Description |
| --- | --- | --- |
| `active_skin` | `default` | Skin folder name inside `skins` directory or `default` |
| `mode` | `dot` | Trail mode: `dot`, `sprite` or `gif` |
| `sprite_file` | `texture.png` | Image file name for sprite mode |
| `gif_file` | `animation.gif` | Animation file name for GIF mode |
| `max_fps` | `120` | Frame rate limit (15 to 360) |
| `trail_enabled` | `true` | Enable or disable particle trail |
| `step_distance` | `6.0` | Distance between spawned particles in pixels |
| `particle_life` | `0.45` | Particle lifetime in seconds |
| `start_radius` | `18.0` | Initial particle size |
| `end_radius` | `0.0` | Particle size at the end of its life |
| `start_color` | `#00CCFF` | Start color in HEX or RGBA |
| `end_color` | `#0033AA00` | Fade color in HEX or RGBA |
| `exit_on_escape` | `false` | Close application when pressing Escape |
| `hot_reload` | `true` | Enable real-time config reloading |

### Custom Skins

To add a custom skin:

1. Create a new folder inside `%USERPROFILE%\.cureff\skins\<skin_name>\`.
2. Place your image (`texture.png`) or GIF file (`animation.gif`) inside that folder.
3. Add an optional `skin.cfg` file with custom parameters:

```ini
type = gif
file = animation.gif
base_size = 32.0
particle_life = 0.5
step_distance = 6.0
align_to_motion = true
```

4. Select the skin from the system tray menu under `Skins` or set `active_skin = <skin_name>` in `settings.cfg`.

## System Tray Controls

- Double-click tray icon: Open the configuration folder in File Explorer
- Right-click tray icon:
  - Enable Trail: Turn particle rendering on or off
  - Skins: Choose active skin from installed skins
  - Reload Config: Manually re-read `settings.cfg`
  - Open Config Folder: Open `%USERPROFILE%\.cureff`
  - About: View program information
  - Exit: Close the application

## License

This project is licensed under the GNU General Public License v2 (GPL-2.0). See [license.md](license.md) for details.

## Author

ewasion137
