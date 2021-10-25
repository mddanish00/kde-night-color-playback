# kde-night-color
`kde-night-color` is an mpv plugin that inhibits the "Night Color" feature (the blue light filter) of KDE Plasma. This only works on Linux while running KDE Plasma, but it could probably be changed to fit other DEs that support a similar feature. Mpv must be compiled with cplugin support.

## Build
Run `make`, the included Makefile should do the rest.

## Install
Run `make install` to automatically copy the object file in `$HOME/.config/mpv/scripts`, which will enable it for your user.
