# Makefile for tFont, by Tomas ™gren

all: tFont.exe dosFont.exe

tFont.exe: tFont.c tFont.def
	gcc -s -Zomf -Zmtd -O3 -o tFont.exe tFont.c tFont.def

standalone: tFont.c tFont.def
	gcc -s -Zomf -Zsys -O3 -o tFont.exe tFont.c tFont.def

dosFont.exe: dosFont.c
	bcc -mh -3 dosFont.c

