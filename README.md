
# Alpha Language

**Description**: Alpha, a very simple clone of Delta.

## Authors

* Angelos T. Dimoglis csd5078
* Panagiwths Antonakakhs csd5137
* Aris Patramanis csd5249

## Testing

Testing was done at kerasi.

## How to compile

```
make all
```
or just
```
make
```
(all is the default)

## Command line options

All options are optional, the default input and output are stdin and stdout.

* -i \<input file\>
* -o \<output file\>
* -d (yydebug)
* -t (lexer tokens)
* -s (symbol table)
* -b \<binary filename\> (if not given the default is binary.abc)

## Note on the directory structure

* bin/
    * all .o files
* lib/
    * all .h files and the generated .hpp file
* src/
    * all .cpp files (includeing main and the generated lexer, parser and parser.output)
* lexer.l
* parser.y
* Makefile
* README.md
