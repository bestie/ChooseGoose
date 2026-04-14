# Choose Goose

ChooseGoose is a silly graphical menu program that is general-purpose, customizable, and script-friendly.

Originally created for GarlicOS running on the Anbernic RG35XX handheld gaming system to solve some of the problems enthusiasts were having creating functionality with the existing GUI.

![Demo](./assets/MD%20star%20trek.gif)

## Features

* Script friendly
* Customizable background, colors, typeface, text sizes, margins, padding
* Selection filtering (keyboard only)
* Joypad and keyboard support
* Works on small Linux devices like RG35XXs, Raspberry Pis, and macOS too
* Embedded default font and background image
* Inactivity timeout (quits after n ms with no input)
* Fun! (Also silly)

## Usage

Given a newline separated list of items on stdin, ChooseGoose will present the user with an interactive menu and output their selection on stdout.

Select a file from the current directory (outputs to stdout)
```bash
$ ls | choosegoose
MY_SELECTED_FILE.md # or whatever you chose
```

Create an arbitrary list of items and capture the selection in a variable:
```sh
choice=$(printf "Duck\nGoose\nMoose" | choosegoose)
echo "You picked: $choice" # You should always choose 'Goose' of course.
```

Select a movie file and play with VLC
```bash
$ ls *.mp4 | choosegoose --hide-file-extensions | xargs vlc --play-and-exit
```

Or a MacOS application launcher
```sh
ls -1 /Applications | choosegoose --hide-file-extensions | xargs open
```

Use the following command line options to customize:

```
  --screen-width=N                         In pixels. Default 640.
  --screen-height=N                        In pixels. Default 480.
  --bits-per-pixel=N                       In pixels. Default 32.
  --title=TITLE                            Title text appears with extra padding and can have a different font size.
  --font=FILEPATH                          Path to a custom true type font file used for all text.
  --background-image=FILEPATH|DEFAULT      Path to a PNG image or DEFAULT for the built-in goose.
  --font-size=N                            In pixels. Default 22.
  --title-font-size=N                      In pixels. Default 28.
  --top-padding=N                          In pixels. Default 10.
  --bottom-padding=N                       In pixels. Default 10.
  --left-padding=N                         In pixels. Default 10.
  --right-padding=N                        In pixels. Default 10.
  --menu-item-padding=N                    Padding around menu text items. Default 2.
  --menu-item-margin=N                     Space between menu items in pixels. Default 0.
  --start-at-nth=N                         List item to start menu from; default 1 (first), set to 0 for no initial selection.
  --user-inactivity-timeout-ms=N           Quits after N milliseconds if the user has not made an input. Exits with status 124 like GNU timeout.
  --key-repeat-delay-ms=N                  Delay in milliseconds before key repeat starts. See SDL_EnableKeyRepeat.
  --key-repeat-interval-ms=N               Interval in milliseconds between key repeats. See SDL_EnableKeyRepeat.
  --hide-file-extensions=true|false        When using files as input and output, extensions can be hidden from the user.
  --prefix-with-number=true|false          Menu items are prepended with their list order number starting at 1.
  --background-color=RRGGBB                Visible when no background image is set and through transparent PNG regions.
  --text-color=RRGGBB                      
  --text-selected-color=RRGGBB             A different color for the selected item text.
  --text-selected-background-color=RRGGBB  A solid background color for the selected item text.
  --cover-images-dir=FILEPATH              Directory containing cover images for menu items. To display the image the filename must match the item text with '.png'.
  --log-file=FILEPATH|stderr|stdout        Log debug events to a file, stderr or stdout, though stdout is probably not a good idea.
```

## Button and Key Bindings

### Keyboard

| Key               | Function
|-------------------|--------------------------------
| Up/Down arrows    | menu up/down
| Left/Right arrows | page up/down
| Enter             | confirm selection
| Esc               | exit(0) with no selection

### Joypad Buttons

| Button            | Function
|-------------------|--------------------------------
| Up/Down           | menu up/down
| Left/Right        | page up/down
| Start             | confirm selection
| Menu              | exit(0) with no selection
| A                 | confirm selection
| B                 | exit(0) with no selection

## Displaying Cover Images

"Cover images" are background images for individual menu items loaded on the fly from the specified 'cover-images-dir`.

The image file name must match the item text input but have a '.png' file extension. Only PNG is supported.

For example a directory may contain:

```
|
|- s01e01.mp4
|- s01e02.mp4
|- s01e03.mp4
|- .imgs
    |
    |- s01e01.png
    |- s01e02.png
    |- s01e03.png

```

This command will then display the episode list with an individual background image for each one.

```sh
ls *.mp4 | choosegoose --hide-file-extensions=true --cover-images-dir=.imgs
```

## Installation on macOS

Homebrew will quickly install a pre-compiled statically binary with all the SDL dependencies baked in.
Currently only supporting Apple Silicon. Intel builds in the works.

```sh
brew tap bestie/choosegoose
brew install choosegoose
```

## Installation on GarlicOS

Check out the GarlicOS Game and Video Browser apps.

To install and try it out, go to the [releases page](../../releases), download and unzip RG35XX-demos.zip, and copy-paste over or merge into ROMS directory.

ChooseGoose has no dependencies beyond SDL 1.2 which is already on the device so you can just grab the binary from the demo zip and go play.

## Installation on Linux

Compile it from source.

This will produce a single binary, with only minimal system dependencies required.

```sh
sudo apt-get install build-essential pkg-config zlib1g-dev libx11-dev libxext-dev
make choosegoose
sudo make install
```

The first build downloads and compiles vendored SDL dependencies automatically. Subsequent builds skip this step. The binary is installed to `/usr/local/bin/choosegoose` by default, or override with `sudo make install PREFIX=/opt/somewhere`.

## Contributing

ChooseGoose is written in C with the ancient SDL1.2 framework and renders to the Linux framebuffer device without hardware support. The Makefile contains the flags to compile on macOS and render via Cocoa.

To compile natively (all SDL dependencies are built from source automatically):
```
make choosegoose
```

To cross-compile for RG35XX in Docker:
```
make docker-compile-rg35xx
```

1. Fork this repo
2. Create your feature branch (`git checkout -b my-new-feature`)
3. Commit your changes (`git commit -am 'Add some feature'`)
4. Push to the branch (`git push origin my-new-feature`)
5. Create a new Pull Request
