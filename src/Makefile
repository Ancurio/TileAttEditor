app       = TileAttEditor
objects   = main.o settings.o ui.o callback.o tileset-area.o \
            new-file-dialog.o settings-dialog.o save-changes-dialog.o \
            color.o file.o attribute.o util.o  \
            attr-passability.o attr-quadpassability.o attr-priority.o  \
            attr-bushflag.o attr-counterflag.o attr-terrainflag.o
include   = `pkg-config --libs --cflags gtk+-3.0 cairo` \
            `xml2-config --libs --cflags`
gcc-flags = -g


$(app) : $(objects)
	cc -o $(app) $(objects) $(include) $(gcc-flags)

main.o : main.c tileatteditor.h tileset-area.h
	cc -c main.c -o main.o $(include) $(gcc-flags)

settings.o : settings.c tileatteditor.h settings.h
	cc -c settings.c -o settings.o $(include) $(gcc-flags)

ui.o : ui.c tileatteditor.h callback.h ui-menubar.xml
	cc -c ui.c -o ui.o $(include) $(gcc-flags)

callback.o : callback.c tileatteditor.h callback.h dialog.h
	cc -c callback.c -o callback.o $(include) $(gcc-flags)

tileset-area.o : tileset-area.c tileatteditor.h
	cc -c tileset-area.c -o tileset-area.o $(include) $(gcc-flags)

new-file-dialog.o : new-file-dialog.c tileatteditor.h dialog.h
	cc -c new-file-dialog.c -o new-file-dialog.o $(include) $(gcc-flags)

settings-dialog.o : settings-dialog.c tileatteditor.h dialog.h callback.h
	cc -c settings-dialog.c -o settings-dialog.o $(include) $(gcc-flags)

save-changes-dialog.o : save-changes-dialog.c tileatteditor.h dialog.h util.h
	cc -c save-changes-dialog.c -o save-changes-dialog.o $(include) $(gcc-flags)

color.o : color.c color.h
	cc -c color.c -o color.o $(include) $(gcc-flags)

file.o : file.c tileatteditor.h file.h
	cc -c file.c -o file.o $(include) $(gcc-flags)

attribute.o : attribute.c tileatteditor.h attribute.h
	cc -c attribute.c -o attribute.o $(include) $(gcc-flags)

util.o : util.c util.h
	cc -c util.c -o util.o $(include) $(gcc-flags)


attr-passability.o : attr-passability.c attribute.h
	cc -c attr-passability.c -o attr-passability.o $(include) $(gcc-flags)

attr-quadpassability.o : attr-quadpassability.c attribute.h
	cc -c attr-quadpassability.c -o attr-quadpassability.o $(include) $(gcc-flags)

attr-priority.o : attr-priority.c attribute.h
	cc -c attr-priority.c -o attr-priority.o $(include) $(gcc-flags)

attr-bushflag.o : attr-bushflag.c attribute.h
	cc -c attr-bushflag.c -o attr-bushflag.o $(include) $(gcc-flags)

attr-counterflag.o : attr-counterflag.c attribute.h
	cc -c attr-counterflag.c -o attr-counterflag.o $(include) $(gcc-flags)

attr-terrainflag.o : attr-terrainflag.c attribute.h
	cc -c attr-terrainflag.c -o attr-terrainflag.o $(include) $(gcc-flags)



.PHONY : clean
clean :
	rm $(app) $(objects)
