# Choose Goose

ChooseGoose is a silly graphical menu program which is general purpose, customizable, and script friendly.

Originally created for the Anbernic RG35XX handheld gaming system and GarlicOS to solve some of problems enthusiasts were having creating functionality with the existing GUI.

ChooseGoose isn't an app but is it building block to create your own functionality.

## Features

* Customizable background, colors and font
* Joypad and keyboard support
* Works on small Linux devices like RG35XXs and Raspberry Pis, macOS too
* Embedded font and background
* Developer friendly
* Fun and a bit silly

## Usage

Given a new-line separated list of items on stdin, ChooseGoose will present the user with an interactive menu and output their selection on stdout.

To present a list of files from the current directory and echo out the user's selection with a message:

```
$ ls -1 | choosegoose | xargs -I{} echo "You chose `{}`"
```

Use the following command line options to customize:

```
  --screen-width SIZE in px              default 640
  --screen-height SIZE in px             default 480
  --bits-per-pixel N                     default 32
  --title TITLE                          set the window title
  --font FILEPATH                        path to true type font file
  --background-image FILEPATH            path to PNG image
  --font-size SIZE in px                 set the font size
  --top-padding SIZE in px               set the top padding
  --bottom-padding SIZE in px            set the bottom padding
  --left-padding SIZE in px              set the left padding
  --right-padding SIZE in px             set the right padding
  --start-at-nth N                       list item to start menu from, default 1 (first), set 0 for no initial selection
  --hide-file-extensions true|false      hide file extensions when using files as input and output
  --prefix-with-number true|false        prefix menu items with a number
  --background-color RRGGBB              set the background color (CSS style hex color without leading #)
  --text-color RRGGBB                    set the text color (CSS style hex color without leading #)
  --text-selected-color RRGGBB           set the text selected color (CSS style hex color without leading #)
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

To install and try those out, go to the releases page, download and unzip demos.zip, copy-paste or merge over your APPS and ROMS directory.

## Things You Could Build With It

With a little shell scripting it would be easy to build things like:

* Quiz games
* Theme switcher
* Game/Media launchers
* System admin interfaces

## Contributing

ChooseGoose is written in C with the ancient SDL1.2 framework and renders to the Linux framebuffer device without hardware support.

The Makefile contains the flags to compile on macOS and render via Cocao.

To compile natively (requires libraries and C compiler)
```
make goose
```

To compile in Docker for Linux (various):
```
make all
```

1. Fork it this repo
2. Create your feature branch (`git checkout -b my-new-feature`)
3. Commit your changes (`git commit -am 'Add some feature'`)
4. Push to the branch (`git push origin my-new-feature`)
5. Create a new Pull Request
