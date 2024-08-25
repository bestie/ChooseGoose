# Choose Goose

ChooseGoose is a silly graphical menu program that is general-purpose, customizable, and script-friendly.

Originally created for GarlicOS running on the Anbernic RG35XX handheld gaming system to solve some of the problems enthusiasts were having creating functionality with the existing GUI.

## Features

* Customizable background, colors and font
* Joypad and keyboard support
* Works on small Linux devices like RG35XXs, Raspberry Pis, and macOS too
* Embedded font and background
* Shell script friendly
* Inactivity timeout
* Fun and a bit silly

## Usage

Given a newline separated list of items on stdin, ChooseGoose will present the user with an interactive menu and output their selection on stdout.

To present a list of files from the current directory and echo the user's selection with a message:

```
$ ls -1 | choosegoose | xargs -I{} echo "You chose `{}`"
```

Use the following command line options to customize:

```
  --screen-width=N                         In pixels. Default 640.
  --screen-height=N                        In pixels. Default 480.
  --bits-per-pixel=N                       In pixels. Default 32.
  --title=TITLE                            Title text appears with extra padding and can have a different font size.
  --font=FILEPATH                          Path to a custom true type font file used for all text.
  --background-image=FILEPATH|DEFAULT      Path to a PNG image or DEFAULT for the built-in goose.
  --font-size=N                            In pixels.
  --title-font-size=N                      In pixels.
  --top-padding=N                          In pixels.
  --bottom-padding=N                       In pixels.
  --left-padding=N                         In pixels.
  --right-padding=N                        In pixels.
  --start-at-nth=N                         List item to start menu from; default 1 (first), set to 0 for no initial selection.
  --user-inactivity-timeout-ms=N           Quits after N milliseconds if the user has not made an input. Exits with status 124 like GNU timeout.
  --hide-file-extensions=true|false        When using files as input and output, extensions can be hidden from the user.
  --prefix-with-number=true|false          Menu items are prepended with their list order number starting at 1.
  --background-color=RRGGBB                Visible when no background image is set and through transparent PNG regions.
  --text-color=RRGGBB
  --text-selected-color=RRGGBB             A different color for the selected item text.
  --text-selected-background-color=RRGGBB  A solid background color for the selected item text.
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

## Getting Started

Check out the Demo GarlicOS Video Browser app.

To install and try it out, go to the releases page, download and unzip demos.zip, and copy-paste over or merge into your APPS and ROMS directory.

## Things You Could Build With It

With a little shell scripting it would be easy to build things like:

* Quiz games
* Theme switcher
* Game/Media launchers
* System admin interfaces

## Contributing

ChooseGoose is written in C with the ancient SDL1.2 framework and renders to the Linux framebuffer device without hardware support.

The Makefile contains the flags to compile on macOS and render via Cocoa.

To compile natively (requires libraries and C compiler)
```
make goose
```

To compile in Docker for Linux (various):
```
make all
```

1. Fork this repo
2. Create your feature branch (`git checkout -b my-new-feature`)
3. Commit your changes (`git commit -am 'Add some feature'`)
4. Push to the branch (`git push origin my-new-feature`)
5. Create a new Pull Request
