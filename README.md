# Mangaloid Instance

C++ Implementation of [Mangaloid](https://github.com/a-manga-thing/documentation) instances.

## Requirements

To compile this instance, you will need the [Drogon](https://github.com/an-tao/drogon) HTTP application framework. You will also need its [dependency libraries](https://github.com/an-tao/drogon/wiki/ENG-02-Installation#Library-Dependencies).

Furthermore, you need the Meson build system, CMake, the MagickWand ImageMagick C API, Botan, and the [fmt](https://github.com/fmtlib/fmt) C++20 formatting library.

## Setup

```bash
#generate the database (optional)
sqlite3 database.db < schema.sql

#build
meson build
ninja -C build
```

## Usage

Read [config.json](config.json) for instructions on how to configure your instance.

```bash
build/instance
```

*NOTE: schema.sql comes with debug data*

### Admin API

__POST__ `/mod/login` takes a json object with the fields "name" and "pass" as argument, and returns a JSESSIONID cookie.

__POST__ `/mod/manga`, __DELETE__ `/mod/manga`, and __PUT__ `/mod/manga` take a manga encoded as a json object.

__POST__ `/mod/chapter`, __DELETE__ `/mod/chapter`, and __PUT__ `/mod/chapter` take a chapter encoded as a json object.
