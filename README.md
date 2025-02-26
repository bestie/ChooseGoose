# Choose Goose

ChooseGoose is a silly graphical menu program that is general-purpose, customizable, and script-friendly.

Originally created for GarlicOS running on the Anbernic RG35XX handheld gaming system to solve some of the problems enthusiasts were having creating functionality with the existing GUI.

![Demo](https://private-user-images.githubusercontent.com/145723/417168771-7b958dce-f6a1-463d-83ee-ac730e4ac463.gif?jwt=eyJhbGciOiJIUzI1NiIsInR5cCI6IkpXVCJ9.eyJpc3MiOiJnaXRodWIuY29tIiwiYXVkIjoicmF3LmdpdGh1YnVzZXJjb250ZW50LmNvbSIsImtleSI6ImtleTUiLCJleHAiOjE3NDA1Nzg5MjgsIm5iZiI6MTc0MDU3ODYyOCwicGF0aCI6Ii8xNDU3MjMvNDE3MTY4NzcxLTdiOTU4ZGNlLWY2YTEtNDYzZC04M2VlLWFjNzMwZTRhYzQ2My5naWY_WC1BbXotQWxnb3JpdGhtPUFXUzQtSE1BQy1TSEEyNTYmWC1BbXotQ3JlZGVudGlhbD1BS0lBVkNPRFlMU0E1M1BRSzRaQSUyRjIwMjUwMjI2JTJGdXMtZWFzdC0xJTJGczMlMkZhd3M0X3JlcXVlc3QmWC1BbXotRGF0ZT0yMDI1MDIyNlQxNDAzNDhaJlgtQW16LUV4cGlyZXM9MzAwJlgtQW16LVNpZ25hdHVyZT00M2JiNTI3OTQ0ZTUyNjFhZWNmMWI4NzQzZDJkMTQzNjVjYmQ2ODc4NWQxMTgyYzQ1YzZlMmI3NjlhMGVlMGI2JlgtQW16LVNpZ25lZEhlYWRlcnM9aG9zdCJ9.HX4XeEjppZHOmtImrv4ocQlLazC_C31lj8xKyuCzI68)

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

## RG35XX Installation

Check out the GarlicOS Game and Video Browser apps.

To install and try it out, go to the [releases page](../../releases), download and unzip RG35XX-demos.zip, and copy-paste over or merge into ROMS directory.

ChooseGoose has no dependencies beyond SDL 1.2 which is already on the device so you can just grab the binary from the demo zip and go play.

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
