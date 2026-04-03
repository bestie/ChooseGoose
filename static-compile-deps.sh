#!/usr/bin/env bash

set -ex

jobs=${JOBS:-4}
platform=$(make echo-platform)

mkdir -p build/$platform/vendor
BASE_DIR=$(readlink -f build/$platform/vendor)
mkdir -p $BASE_DIR/build
PREFIX=$(readlink -f $BASE_DIR/build)
mkdir -p vendor
DOWNLOADS_DIR=$(readlink -f ./vendor)

export PKG_CONFIG_PATH="$PREFIX/lib/pkconfig"

if test $(uname -s) = "Darwin"; then
  PLATFORM_LIBS="-framework AudioToolbox \
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
else
  PLATFORM_LIBS=""
fi

# cd $DOWNLOADS_DIR
# curl --fail -LO https://zlib.net/zlib-1.3.2.tar.gz
# curl --fail -LO https://download.sourceforge.net/libpng/libpng-1.6.56.tar.gz
# curl --fail -LO https://download.savannah.gnu.org/releases/freetype/freetype-2.13.2.tar.gz
# curl --fail --output SDL-1.2.tar.gz -L https://github.com/libsdl-org/SDL-1.2/archive/refs/heads/main.tar.gz
# curl --fail --output  SDL_image.tar.gz -L https://github.com/libsdl-org/SDL_image/archive/refs/heads/SDL-1.2.tar.gz
# curl --fail --output SDL_ttf.tar.gz -L https://github.com/libsdl-org/SDL_ttf/archive/refs/heads/SDL-1.2.tar.gz
# exit

function zlib() {
  cp -r $DOWNLOADS_DIR/zlib* . && tar xf zlib-1.3.2.tar.gz
  cd zlib-1.3.2

  ./configure --static --prefix="$PREFIX"
  make && make install
}

function libpng() {
  cp -r $DOWNLOADS_DIR/libpng* . && tar xf libpng-1.6.56.tar.gz
  cd libpng-1.6.56

  export CPPFLAGS="-I$PREFIX/include"
  export LDFLAGS="-L$PREFIX/lib"

  ./configure \
    --prefix="$PREFIX" \
    --with-zlib-prefix="$PREFIX" \
    --enable-static \
    --disable-shared

  make -j$jobs
  make install

  export LIBPNG_CFLAGS="-I$PREFIX/include"
  export LIBPNG_LIBS="-L$PREFIX/lib"
}

function sdl() {
  cp -r $DOWNLOADS_DIR/SDL-1.2.* . && tar xf SDL-1.2.tar.gz

  cd SDL-1.2-main
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
  cp -r $DOWNLOADS_DIR/freetype* . && tar xf freetype-2.13.2.tar.gz
  cd freetype-2.13.2

  ./configure \
    --disable-shared \
    --enable-static \
    --with-png=yes \
    --without-harfbuzz \
    --without-bzip2 \
    --without-brotli \
    --prefix="$PREFIX" \
    LIBPNG_CFLAGS="$LIBPNG_CFLAGS" \
    LIBPNG_LIBS="$LIBPNG_LIBS"

  make -j$job
  make install
}

function sdl_ttf() {
  cp -r $DOWNLOADS_DIR/SDL_ttf* . && tar xf SDL_ttf.tar.gz
  cd SDL_ttf-SDL-1.2

  export CPPFLAGS="-I$PREFIX/include"
  export LDFLAGS="-L$PREFIX/lib"
  export LIBS="-lfreetype $PLATFORM_LIBS"
  export SDL_LIBS="-L$PREFIX/lib"

  export FT2_CFLAGS="-I$PREFIX/include/freetype2"
  export FT2_LIBS="-L$PREFIX/lib"

  ./configure \
    --enable-static \
    --disable-shared \
    --disable-sdltest \
    --prefix="$PREFIX" \
    --with-sdl-prefix="$PREFIX"

  make SDL_ttf.o
  mkdir -p $PREFIX/obj
  cp SDL_ttf.o $PREFIX/obj/libSDL_ttf.o
  cp SDL_ttf.h $PREFIX/include
}

function sdl_image() {
  cp -r $DOWNLOADS_DIR/SDL_image.* . && tar xf SDL_image.tar.gz
  cd SDL_image-SDL-1.2

  export LIBS=""
  export SDL_LIBS=""
  export CPPFLAGS="-I$PREFIX/include"
  export LDFLAGS="-L$PREFIX/lib -lpng $PLATFORM_LIBS"

  ./configure \
      --prefix="$PREFIX" \
      --enable-static \
      --disable-shared \
      --disable-png-shared \
      --disable-sdltest \
      --disable-tif \
      --disable-imageio \
      --disable-webp \
      --disable-jpg \
      --enable-png

  objs="IMG.o \
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
    IMG_xv.o"

  make $objs

  ar rcs libSDL_image.a $objs
  cp libSDL_image.a $PREFIX/lib
  cp SDL_image.h $PREFIX/include
}

cd $BASE_DIR
echo""
echo "🤐 Compiling zlib 🤺"
zlib
echo "✅ zlib"
echo""

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
