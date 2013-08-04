#define COPYRIGHT "(C) Tomas ™gren, Ilya Zakharevich 1996-2001"
#define INCL_VIO                          /* I want some Vio functions.. */
#include <os2.h>                                         /* Misc stuff.. */
#include <os2thunk.h>  /* included for the 32 bit to 16 bit pointer conv */
#include <stdio.h>                            /* standard input output.. */
#include <sys/types.h>                        /* Needed by sys/dir.h. */
#include <sys/dir.h>                          /* MAXPATHLEN. */
#include <getopt.h>                           /* for the option switches */

int opt_displayonly=0,opt_force=0, opt_write=0;          /* some options */
int opt_autosize=0;

ULONG filesize(FILE*f)                           /* returns the filesize */
{
    ULONG i,j;
    i=ftell(f);
    fseek(f,0,SEEK_END);
    j=ftell(f);
    fseek(f,i,SEEK_SET);
    return j;
}

int RUSure(char*prompt)                               /* Are you sure... */
{
    int c=255;
    printf("%s [y/N]  ",prompt);
    c=toupper(getchar());
    if (c=='Y') return 1;
    return 0;
}

void usage(void)                                                /* Usage */
{
    printf("Usage:\ttFont.exe [-?|-h|-d|-f|-w|-q] <Filename> [<Tail>]\n");
    printf("\t<Filename>          Installs the font <Filename>\n");
    printf("\t-f                  Force yes on all questions\n");
    printf("\t-w                  Write font file\n");
    printf("\t-d                  Display status only\n");
    printf("\t-s                  Chose subdirectory basing on the charcell size\n");
    printf("\t-q                  Quiet\n");
    printf("\t-?                  This help\n");
    printf("\t-h                  This help\n");
    printf("\nThis program gets/sets the current font in a fullscreen OS/2 VIO\n");
    printf("\nWith -s the actual filename is Filename/HHxWW/Tail,\n");
    printf("HH and WW are the height and width of the character cell.\n");
    printf("Tail defaults to cpNNN.fnt, here NNN is the current codepage.\n");
    printf("See docs for more information.\n");
    printf("%s\n",COPYRIGHT);
    exit(1);
}

void PrintError(APIRET err)                             /* Prints errors */
{
    switch (err) {
    case 494:
        printf("You can't run this program in an Windowed session.\n");
        break;
    case 355:
        printf("Sorry, your drivers does not support this call.\nGet newer drivers and try again or give up.. :(.\nYou must exit this session, since it's \"broken\" now.\n");
        break;
    default:
        printf("An error has occurred, errorcode: %ld=%#x\n",err,err);
        break;
    }
}

int main(int argc,char*argv[])                                /* Main .. */
{
    VIOFONTINFO font;
    UCHAR b[1<<17];
    UCHAR *buf = b;
    UCHAR *fname;
    UCHAR *tail;
    UCHAR *dir = NULL;
    UCHAR fnamebuf[MAXPATHLEN];
    UCHAR tailbuf[MAXPATHLEN];
    UCHAR dirbuf[MAXPATHLEN];
    FILE*file;
    APIRET rc;
    int c;

    printf("tFont v1.2 by Tomas ™gren <stric@freenet.hut.fi>, Ilya Zakharevich\n\n");
    fflush(stdout);
    if (((ULONG)buf) & 0xFFFF)
        buf += 0x10000 - (((ULONG)buf) & 0xFFFF);
    opterr = 0;
    while ((c = getopt (argc, argv, "dfws")) != EOF)
        switch (c)
        {
        case 'd':
            opt_displayonly = 1;
            break;
        case 'w':
            opt_write = 1;
            break;
        case 'f':
            opt_force = 1;
            break;
        case 's':
            opt_autosize = 1;
            break;
        default:
            usage ();
        }
    if (!opt_displayonly)
        if (argc - optind != 1 && (!opt_autosize || argc - optind != 2))
            usage ();

    font.cb=sizeof(font);                 /* How large is this structure */
    font.type=0;              /* Dunno, If you know what it is, mail me! */
    font.cbData=65535;                        /* How large is my buffer? */
    font.pbData=_emx_32to16(buf);  /* Wants an 16:16 pointer, converting */

    rc=VioGetFont (&font,0);           /* Retrieve data for current font */
    if (rc)
    {
        PrintError(rc);
        return 2;
    }
    fname = argv[optind];
    if (opt_autosize) {
	if (optind >= argc - 1) {
            unsigned long lst[3];
            unsigned long len;

            if (DosQueryCp(sizeof(lst), lst, &len) == 0 && len >= 1) {
                sprintf(tailbuf, "cp%lu.fnt", lst[0]);
                tail = tailbuf;
            } else {
	        printf("Failure to query codepage!\n");
	        return 1;
	    }
	} else
	    tail = argv[optind+1];
	sprintf(dirbuf, "%s/%dx%d", fname, font.cyCell, font.cxCell);
	dir = dirbuf;
	sprintf(fnamebuf, "%s/%dx%d/%s", fname, font.cyCell, font.cxCell, tail);
	fname = fnamebuf;
    }
    if (opt_write) {
	if (dir) {
	    rc = mkdir(argv[optind], 0777); /* Ignore the error, dir may exists. */
	    rc = mkdir(dir, 0777);	/* Ignore the error, dir may exists. */
	}
        if ((file=fopen(fname,"wb"))==NULL)
        {
            printf("Error opening file '%s'!\n", fname);
            return 1;
        }
	if (fwrite(buf, font.cbData, 1, file) != 1) {
            printf("Error writing to file '%s'!\n", fname);
            return 1;
        }
	if (fclose(file) != 0) {
            printf("Error closing file '%s'!\n", fname);
            return 1;
        }
	return 0;
    }
    if (!opt_displayonly)
        if ((file=fopen(fname,"rb"))==NULL)
        {
            printf("Error opening file '%s'!\n", fname);
            return 1;
        }
    printf("Fontsize: %lux%lu\nTotal bytes needed for font: %lu",font.cxCell, font.cyCell, font.cbData);
    if (opt_displayonly)
        exit(0);
    if (filesize(file)!=font.cbData)
    {
        printf(", size of %s is %u",fname,filesize(file));
        if (!opt_force)
        {
            if (RUSure("\nFilesize differs from size needed for the font..\nInstall anyway?")==0)
            {
                printf("Aborting...\n");
                exit(1);
            }
        }
    }
    printf("\n");
    fread(buf,1,font.cbData,file);
    fclose(file);
    rc=VioSetFont(&font,0);                         /* Put it all back.. */
    if (rc)
    {
        PrintError(rc);
        return 1;
    }
    printf("All Ok, font '%s' installed successfully.\n",fname);
}
