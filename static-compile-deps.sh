#!/usr/bin/env bash

set -ex

OS=$(uname -s)

if [ "$OS" = "Darwin" ]; then
  jobs=$(sysctl -n hw.ncpu)
else
  jobs=$(nproc)
fi

ARCH=$(uname -m)
if [ "$OS" = "Linux" ]; then
  PLATFORM="${OS}-${ARCH}-glibc"
else
  PLATFORM="${OS}-${ARCH}"
fi

mkdir -p "./vendor/build/${PLATFORM}"
PREFIX=$(readlink -f "./vendor/build/${PLATFORM}")
BASE_DIR=$(readlink -f ./vendor)
export PKG_CONFIG_PATH="$PREFIX/lib/pkgconfig"
export PATH="$PREFIX/bin:$PATH"

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

  export LIBPNG_CFLAGS="-I$PREFIX/include"
  export LIBPNG_LIBS="-L$PREFIX/lib -lpng"
}

function sdl() {
  ls SDL-1.2 || git clone https://github.com/libsdl-org/SDL-1.2.git

  cd SDL-1.2/

  if [ "$OS" = "Darwin" ]; then
    ./configure \
      --disable-x11 \
      --enable-static \
      --disable-shared \
      --disable-video-x11 \
      --enable-video-cocoa \
      --prefix="$PREFIX"
  else
    ./configure \
      --enable-static \
      --disable-shared \
      --enable-video-x11 \
      --disable-video-cocoa \
      --disable-audio \
      --prefix="$PREFIX"
  fi

  make -j$jobs
  make install
}

function freetype() {
  ls freetype-2.13.2 || curl -LO https://download.savannah.gnu.org/releases/freetype/freetype-2.13.2.tar.gz \
    && tar xf freetype-2.13.2.tar.gz
  cd freetype-2.13.2

  local extra_flags=""
  if [ "$OS" != "Darwin" ]; then
    extra_flags="--with-brotli=no"
  fi

  ./configure \
    --disable-shared \
    --enable-static \
    --with-png=yes \
    --without-harfbuzz \
    --without-bzip2 \
    $extra_flags \
    --prefix="$PREFIX" \
    LIBPNG_CFLAGS="$LIBPNG_CFLAGS" \
    LIBPNG_LIBS="$LIBPNG_LIBS"

  make -j$jobs
  make install || {
    # install may fail on chmod of pkgconfig files on some filesystems;
    # verify the important artifacts landed then copy .pc file manually
    test -f $PREFIX/lib/libfreetype.a && test -d $PREFIX/include/freetype2
    rm -f $PREFIX/lib/pkgconfig/freetype2.pc 2>/dev/null; true
    cp builds/unix/freetype2.pc $PREFIX/lib/pkgconfig/freetype2.pc
  }
}

function sdl_ttf() {
  ls SDL_ttf || git clone https://github.com/libsdl-org/SDL_ttf.git

  cd SDL_ttf && git checkout SDL-1.2

  export CPPFLAGS="-I$PREFIX/include -I$PREFIX/include/freetype2"
  export SDL_LIBS="-L$PREFIX/lib"

  if [ "$OS" = "Darwin" ]; then
    export LDFLAGS="-L$PREFIX/lib -liconv"
    export LIBS="-lfreetype $MACOS_FRAMEWORKS"
  else
    export LDFLAGS="-L$PREFIX/lib"
    export LIBS="-lfreetype"
  fi

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

  if [ "$OS" = "Darwin" ]; then
    export LDFLAGS="-L$PREFIX/lib -lpng $MACOS_FRAMEWORKS"
  else
    export LDFLAGS="-L$PREFIX/lib -lpng"
  fi

  ls SDL_image || git clone https://github.com/libsdl-org/SDL_image.git

  cd SDL_image && git checkout SDL-1.2

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

  if [ "$OS" = "Darwin" ]; then
    make IMG_ImageIO.o
  fi

  ar rcs libSDL_image.a IMG*.o
  cp libSDL_image.a $PREFIX/lib
  cp SDL_image.h $PREFIX/include
}

cd $BASE_DIR
echo""
echo "🌅 Compiling libpng 🤺"
libpng
echo "✅ libpng"
echo ""
cd $BASE_DIR
echo ""
echo "⌨️ Compiling freetype 🤺"
freetype
echo "✅ freetype"
echo ""
cd $BASE_DIR
echo ""
echo "📹 Compiling SDL 🤺"
sdl
echo "✅ SDL (base)"
echo ""
cd $BASE_DIR
echo ""
echo "🆎 Compiling SDL TTF 🤺"
sdl_ttf
echo "✅ SDL TTF"
echo ""
cd $BASE_DIR
echo ""
echo "🌅 Compiling SDL image 🤺"
sdl_image
echo "✅ SDL image"
echo ""
