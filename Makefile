app       = TileAttEditor
objects   = main.o settings.o ui.o callback.o tileset-area.o \
            new-file-dialog.o settings-dialog.o color.o file.o \
            attribute.o util.o attr-dummility.o attr-passability.o
include   = `pkg-config --libs --cflags gtk+-2.0 cairo` \
            `xml2-config --libs --cflags`
gcc-flags = -g


$(app) : $(objects)
	cc -o $(app) $(objects) $(include) $(gcc-flags)

main.o : main.c tileatteditor.h tileset-area.h
	cc -c main.c -o main.o $(include) $(gcc-flags)

settings.o : tileatteditor.h settings.h
	cc -c settings.c -o settings.o $(include) $(gcc-flags)

ui.o : tileatteditor.h callback.h ui-menubar.xml
	cc -c ui.c -o ui.o $(include) $(gcc-flags)

callback.o : tileatteditor.h callback.h dialog.h
	cc -c callback.c -o callback.o $(include) $(gcc-flags)

tileset-area.o : tileatteditor.h
	cc -c tileset-area.c -o tileset-area.o $(include) $(gcc-flags)

new-file-dialog.o : tileatteditor.h dialog.h
	cc -c new-file-dialog.c -o new-file-dialog.o $(include) $(gcc-flags)

settings-dialog.o : tileatteditor.h dialog.h callback.h
	cc -c settings-dialog.c -o settings-dialog.o $(include) $(gcc-flags)

color.o : color.h
	cc -c color.c -o color.o $(include) $(gcc-flags)

file.o : tileatteditor.h file.h
	cc -c file.c -o file.o $(include) $(gcc-flags)

attribute.o : tileatteditor.h attribute.h
	cc -c attribute.c -o attribute.o $(include) $(gcc-flags)

util.o : util.h
	cc -c util.c -o util.o $(include) $(gcc-flags)


attr-passability.o : attribute.h
	cc -c attr-passability.c -o attr-passability.o $(include) $(gcc-flags)

attr-dummility.o : attribute.h
	cc -c attr-dummility.c -o attr-dummility.o $(include) $(gcc-flags)



.PHONY : clean
clean :
	rm $(app) $(objects)
