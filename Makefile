all: player GS

player: player.c
	gcc player.c -o player

GS: GS.c
	gcc GS.c -o GS

clean:
	rm -rf player GS *.jpg *.png *.jpeg