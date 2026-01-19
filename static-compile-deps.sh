#!/usr/bin/env bash

set -ex

jobs=$(sysctl -n hw.ncpu)
mkdir -p ./vendor/build
PREFIX=$(readlink -f ./vendor/build)
BASE_DIR=$(readlink -f ./vendor)
export PKG_CONFIG_PATH="$PREFIX/lib/pkconfig"

MACOS_FRAMEWORKS="-framework AudioToolbox \
	-framework AudioUnit \
	-framework Carbon \
	-framework Cocoa \
  -framework CoreAudio \
	-framework CoreHaptics \
	-framework CoreServices \
	-framework CoreVideo \
	-framework ForceFeedback \
	-framework GameController \
	-framework IOKit \
	-framework Metal \
	-framework OpenGL"

function libpng() {
  ls libpng-1.6.43 || curl -LO https://download.sourceforge.net/libpng/libpng-1.6.43.tar.gz \
    && tar xf libpng-1.6.43.tar.gz
  cd libpng-1.6.43

  ./configure \
    --prefix="$PREFIX" \
    --enable-static \
    --disable-shared

  make -j$jobs
  make install
}

function sdl() {
  ls SDL-1.2 || git clone https://github.com/libsdl-org/SDL-1.2.git

  cd SDL-1.2/
  ./configure \
    --disable-x11 \
    --enable-static \
    --disable-shared \
    --disable-video-x11 \
    --enable-video-cocoa \
    --prefix="$PREFIX"

  make -j$jobs
  make install
}

function freetype() {
  ls freetype-2.13.2 || curl -LO https://download.savannah.gnu.org/releases/freetype/freetype-2.13.2.tar.gz \
    && tar xf freetype-2.13.2.tar.gz
  cd freetype-2.13.2

  ./configure \
    --disable-shared \
    --enable-static \
    --with-png=yes \
    --without-harfbuzz \
    --without-bzip2 \
    --prefix="$PREFIX"

  make -j$(sysctl -n hw.ncpu)
  make install
}

function sdl_ttf() {
  ls SDL_ttf || git clone https://github.com/libsdl-org/SDL_ttf.git

  cd SDL_ttf && git checkout SDL-1.2

  export CPPFLAGS="-I$PREFIX/include -I$PREFIX/include/freetype2"
  export LDFLAGS="-L$PREFIX/lib -liconv"
  export LIBS="-lfreetype $MACOS_FRAMEWORKS"
  export SDL_LIBS="-L$PREFIX/lib"

  ./configure \
    --enable-static \
    --disable-shared \
    --with-freetype-prefix="$PREFIX" \
    --disable-examples \
    --disable-sdltest \
    --prefix="$PREFIX" \
    --with-sdl-prefix="$PREFIX"

  make SDL_ttf.o
  mkdir -p $PREFIX/obj
  cp SDL_ttf.o $PREFIX/obj/libSDL_ttf.o
  cp SDL_ttf.h $PREFIX/include
}

function sdl_image() {
  export CPPFLAGS="-I$PREFIX/include"
  export LDFLAGS="-L$PREFIX/lib -lpng $MACOS_FRAMEWORKS"
  export LIBPNG_CFLAGS="$CPPFLAGS"
  export LIBPNG_LIBS="-L$PREFIX/lib -lpng -lz"

  ls SDL_image || git clone https://github.com/libsdl-org/SDL_image.git

  cd SDL_image && git checkout SDL-1.2 \

  ./configure \
      --prefix="$PREFIX" \
      --enable-static \
      --disable-shared \
      --enable-png \
      --disable-sdltest \
      --disable-tif \
      --disable-webp

  make IMG.o \
    IMG_bmp.o \
    IMG_gif.o \
    IMG_ImageIO.o \
    IMG_jpg.o \
    IMG_lbm.o \
    IMG_pcx.o \
    IMG_png.o \
    IMG_pnm.o \
    IMG_tga.o \
    IMG_tif.o \
    IMG_webp.o \
    IMG_xcf.o \
    IMG_xpm.o \
    IMG_xv.o

  ar rcs libSDL_image.a IMG*.o
  cp libSDL_image.a $PREFIX/lib
  cp SDL_image.h $PREFIX/include
}

cd $BASE_DIR

libpng
cd $BASE_DIR
freetype
cd $BASE_DIR
sdl
cd $BASE_DIR
sdl_ttf
cd $BASE_DIR
sdl_image
