# kde-night-color-playback

`kde-night-color-playback` is an mpv plugin that inhibits the "Night Color" feature (the blue light filter) of KDE Plasma **only when media is actively playing**. 

This differs from the original `kde-night-color` which inhibits the feature anytime mpv is running. 

(Yes, the name is a bit confusing - it's an *mpv plugin* that *inhibits* KDE's Night Color feature, not the feature itself!)

This plugin only works on Linux while running KDE Plasma, but it could probably be changed to fit other DEs that support a similar feature. Mpv must be compiled with cplugin support.

## Important Note

This modification is entirely on vibe coding with ChatGPT and Gemini because I am not C developer. Please check changes yourself if you worried about it. Of cource, I checked, compile and run it on mpv myself. I use the plugin myself daily.

## Build

You need `libdbus-1` and `libmpv` to build this plugin.

### For Ubuntu and Debian

```
sudo apt install libdbus-1-dev libmpv-dev
```

### For Arch Linux
```
sudo pacman -S libdbus mpv
```

Run `make`, the included Makefile should do the rest.

## Install

Run `make install` to automatically copy the object file in `$HOME/.config/mpv/scripts`, which will enable it for your user.

## Usage

Once installed, the plugin works automatically. Night Color will be inhibited only when mpv is actively playing media and restored otherwise. No further configuration is needed.

## Acknowledgements

This project is a fork of the original [kde-night-color](https://gitlab.com/smaniottonicola/kde-night-color) project. Thanks to Nicola Smaniotto for their work!

## License

This project is released into the public domain. See the [UNLICENSE.md](./UNLICENSE.md) file for details.
