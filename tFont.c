#define COPYRIGHT "(C) Tomas ™gren '96"
#define INCL_VIO                          /* I want some Vio functions.. */
#include <os2.h>                                         /* Misc stuff.. */
#include <os2thunk.h>  /* included for the 32 bit to 16 bit pointer conv */
#include <stdio.h>                            /* standard input output.. */
#include <getopt.h>                           /* for the option switches */

int opt_displayonly=0,opt_force=0;                       /* some options */

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
    printf("Usage:\ttFont.exe [-?|-h|-d|-f] <Filename>\n");
    printf("\t<Filename>          Installs the font <Filename>\n");
    printf("\t-f                  Force yes on all questions\n");
    printf("\t-d                  Display status only\n");
    printf("\t-?                  This help\n");
    printf("\t-h                  This help\n");
    printf("\nThis program changes the current font in a fullscreen OS/2 VIO\n");
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
        printf("An error has occurred, errorcode: %ld\n",err);
        break;
    }
}

int main(int argc,char*argv[])                                /* Main .. */
{
    VIOFONTINFO font;
    UCHAR buf[65535];
    UCHAR *fname;
    FILE*file;
    APIRET rc;
    int c;

    printf("tFont v1.0 by Tomas ™gren <stric@freenet.hut.fi>\n\n");
    fflush(stdout);
    opterr = 0;
    while ((c = getopt (argc, argv, "df")) != EOF)
        switch (c)
        {
        case 'd':
            opt_displayonly = 1;
            break;
        case 'f':
            opt_force = 1;
            break;
        default:
            usage ();
        }
    if (!opt_displayonly)
        if (argc - optind != 1)
            usage ();

    fname=argv[optind];
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
    if (!opt_displayonly)
        if ((file=fopen(fname,"rb"))==NULL)
        {
            printf("Error opening file '%s'!\n",argv[optind]);
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
