#define COPYRIGHT "(C) Tomas ™gren '96"
#include <stdio.h>
#include <string.h>
#include <dos.h>

int main(int argc,char*argv[])
{
    char buf[32768];
    FILE*f;
    int bseg, bofs;
    printf("dosFont v1.0 by Tomas ™gren <stric@freenet.hut.fi>\n\n");
    if (argc!=2)
    {
        printf("Usage: dosFont.exe <Filename>\n");
        printf("\nThis program changes the current font in a fullscreen DOS session\n");
        printf("See docs for more information.\n");
        printf("%s\n",COPYRIGHT);
	return 1;
    }
    if ((f=fopen(argv[1],"rb"))==NULL)
    {
        printf("Error opening file!\n");
        return 2;
    }
    fread(buf,1,4096,f);
    bofs = ((unsigned long)buf)&0xFFFF;
    bseg = (((unsigned long)buf>>16)&0xFFFF);

    asm {
        push bp
        mov ax,bseg
        mov es,ax
        mov ah,0x11
        mov al,0x00
        mov bh,0x10
        mov bl,0x00
        mov cx,0x100
        mov dx,0x00
        mov bp,bofs
        int 0x10
        pop bp
    }                                                      /* Trust me.. */

    printf("%s installed (un)successfully.\n",argv[1]);
                        /* Dont know really if it went ok.. You decide.. */
    return 0;
}

