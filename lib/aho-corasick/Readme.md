
# aho-corasick

From https://github.com/mischasan/aho-corasick

Multipart match strings

## build

~~~
cc -D_MSC_VER  -I/usr/local/include -D_FORTIFY_SOURCE=2 -D_GNU_SOURCE     -c -o acism.o acism.c
cc -D_MSC_VER  -I/usr/local/include -D_FORTIFY_SOURCE=2 -D_GNU_SOURCE     -c -o acism_create.o acism_create.c
cc -D_MSC_VER  -I/usr/local/include -D_FORTIFY_SOURCE=2 -D_GNU_SOURCE     -c -o acism_dump.o acism_dump.c
cc -D_MSC_VER  -I/usr/local/include -D_FORTIFY_SOURCE=2 -D_GNU_SOURCE     -c -o acism_file.o acism_file.c
cc -D_MSC_VER  -I/usr/local/include -D_FORTIFY_SOURCE=2 -D_GNU_SOURCE     -c -o msutil.o msutil.c
ar crs libacism.a acism.o acism_create.o acism_dump.o acism_file.o msutil.o
~~~
