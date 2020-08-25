# pptxt
pptxt is a simple, fast pptx to txt conversion tool written in C.
It converts powerpoint presetations into txt according to the [suckless sent](https://tools.suckless.org/sent/ "suckless sent") format.

### Dependencies

libzip, lixml2 for building

```sh
$ apt install lixml2-dev
$ apt install lizip-dev
```

### Installation

```sh
$ make clean
$ make
$ make install
```

### Usage

```
$txt [FILE] [-o outfile]
			[ -c / --clean]
			[-v]
```

if -o is omitted output will be written to out.txt
