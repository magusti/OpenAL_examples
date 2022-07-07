/*
 Tret de <https://www.cnblogs.com/fancycode/archive/2012/06/02/2531879.html>
 12/05/2022 Modificat per fer-lo funcionar sobre GNU/Linux
 Compilar per a Linux
$ pkg-config --modversion libmpg123
1.25.13
$ pkg-config  --cflags libmpg123
$ pkg-config  --libs libmpg123
-lmpg123
$ pkg-config  --cflags --libs freealut openal
-I/usr/include/AL -lalut -lopenal

 
 gcc -o openal_mpg123 openal_mpg123.c $(pkg-config  --cflags --libs libmpg123) $(pkg-config  --cflags --libs freealut openal)

*AUTHOR bowman han
* NOTE  this is a test program for mpg123 basic usage
* EMAIL  fancycode+blogcn123@gmail.com
* DATA　６／２　２０１２
* Please DO NOT REMOVE THIS
*/

// #include "openal\include\al.h"
// #include "openal\include\alc.h"
 #include <AL/al.h>
 #include <AL/alc.h>  // Ja els porten darrere en alut.h
// #include <AL/alut.h> 
#include <stdio.h>
// #include <Windows.h>
#define NUM_BUFFERS 4
// #include "mpg123\mpg123.h"
#include <mpg123.h>
// #include <conio.h> // conio.h' es un archivo de cabecera escrito en C usado mayormente por los compiladores de MS-DOS para proveer un sistema de E/S por consola.1​ Éste no es parte de la biblioteca estándar de C o ISO C, ni está definida por POSIX. 
#include <stdio.h> //getchar, printf
//#include <curses.h> //getch
#include <unistd.h> //usleep( 1000000 ); // microsecond intervals
#include <curses.h> //getch, initscr();


///*pragma comment (lib, "./mpg123/libmpg123.lib")
//#pragma comment (lib, "./openal/lib/OpenAL32.lib")
//int main()
int main( int argc, char **argv )
{
    ALuint g_Buffers[NUM_BUFFERS];
    ALuint uiSource;
    ALuint uiBuffer;
    ALCdevice * pDevice = NULL;
    ALCcontext *pContext = NULL;
    ALboolean g_bEAX;
    ALenum error, eBufferFormat;
    ALint iDataSize, iFrequency;
    ALuint ulFormat;
    int iLoop,iBuffersProcessed, iTotalBuffersProcessed;
    unsigned long ulDataSize = 0;
    unsigned long ulFrequency = 0;
    unsigned long ulBufferSize;
    unsigned long ulBytesWritten;
    long lRate;
    int iEncoding, iChannels;
    int encoding;
    void *        pData = NULL;
//     WAVEFORMATEX *pwfex;
    int iState, iQueuedBuffers;
    mpg123_handle *mpg123 ;
    int iMpg123_error;
    
    
    
    if ( argc < 2) {
        printf("Necesite un nom de fitxer: %s fitxer.mp3\n", argv[0] );
        exit(1);
    }
 
 //initscr();
 //halfdelay(5);
 
    //init mpg123 library, en versiones anteriores a la 46 (mpg123 1.27.0)
    // $ pkg-config --modversion libmpg123 --> 1.25.13
    if(MPG123_OK != (iMpg123_error = mpg123_init()))
    {
        printf("failed to init mpg123\n");
        return -1;
    }

    //open a default mpg123 decoder
    {int j = 0;
     printf("Decodificadors MPG123 disponibles: ");
     while (mpg123_decoders()[j] != NULL ) 
       printf("(%d) %s ", j, mpg123_decoders()[j++] );
// El mateix que fica en http://www.mpg123.de/
// Ultravioleta:
// Decodificadors MPG123 disponibles: AVX x86-64 generic generic_dither
    }
    mpg123 = mpg123_new(mpg123_decoders()[0], &iMpg123_error);

    //open a openal default device
    pDevice = alcOpenDevice(NULL); //select the perfered device
    
    if(pDevice){
        pContext = alcCreateContext(pDevice, NULL);
        alcMakeContextCurrent(pContext);
    }else
    {
        printf("failed to get a openal decice\n");
        return -2;
    }

// Per a q
//     g_bEAX = alIsExtensionPresent("EAX2.0");
//

    //Generate openal  Buffers
    alGetError();  //clear all error code
    
    alGenBuffers(NUM_BUFFERS, g_Buffers);

    if((error = alGetError()) != AL_NO_ERROR)
    {
//         fprintf(stderr, "alGenBuffers :%s", error);
// Si deixes ALUT        fprintf(stderr, "alGenBuffers :%s", alutGetErrorString ( error ) );
         fprintf(stderr, "alGenBuffers : errror:: %d.", error);
    }

    alGenSources(1, &uiSource);

    //try to open a mp3 file, modify to your own mp3 files
//     if(MPG123_OK != (iMpg123_error = mpg123_open(mpg123,"D:\\sound\\320k.mp3")))
    if(MPG123_OK != (iMpg123_error = mpg123_open(mpg123, argv[1])))
    {
        fprintf(stderr,"error in open mp3 file\n");
        return -1;
    }
    //get mp3 format infomation
    mpg123_getformat(mpg123, &lRate, &iChannels,  &iEncoding);

    //there only parse stereo mp3 file
    if(iChannels ==2)
    {
        ulFormat = alGetEnumValue("AL_FORMAT_STEREO16");
        ulBufferSize = lRate;  //set buffer to 250ms
        ulBufferSize -= (ulBufferSize % 4);  //set pcm Block align
        ulFrequency = lRate;  //set pcm sample rate

    }
    else
    {
        printf("channels info%i\n", iChannels);
        return -3;
    }


        

    pData = malloc(ulBufferSize);
    //feed data to openal buffer
    for(iLoop= 0; iLoop < 4; iLoop++)
    {
        mpg123_read(mpg123, (char *)pData, ulBufferSize,&ulBytesWritten);
        alBufferData(g_Buffers[iLoop], ulFormat, pData, ulBytesWritten, ulFrequency);
        alSourceQueueBuffers(uiSource, 1, &g_Buffers[iLoop]);
    }
    alSourcePlay(uiSource);
    iTotalBuffersProcessed = 0;
    printf("playing\n");
    //wait for key press
    while( 1 ) //!getch() ) //!getch()) // while(!_kbhit())
    {
        //Sleep(20);
        // ALboolean alutSleep (ALfloat duration); // en segons
        usleep( 20000 ); // microsecond intervals
        iBuffersProcessed = 0;
        alGetSourcei(uiSource, AL_BUFFERS_PROCESSED, &iBuffersProcessed);

        iTotalBuffersProcessed += iBuffersProcessed;
        printf("Buffers total Processed %d\r", iTotalBuffersProcessed);
        while(iBuffersProcessed)
        {
            uiBuffer = 0;
            alSourceUnqueueBuffers(uiSource, 1, &uiBuffer);

            mpg123_read(mpg123, (char *)pData, ulBufferSize, &ulBytesWritten);
            if(ulBytesWritten)
            {
                alBufferData(uiBuffer, ulFormat, pData, ulBytesWritten, ulFrequency);
                alSourceQueueBuffers(uiSource, 1, &uiBuffer);
            }                iBuffersProcessed--;

        }
        alGetSourcei(uiSource, AL_SOURCE_STATE, &iState);
        if(iState != AL_PLAYING)
        {
            alGetSourcei(uiSource, AL_BUFFERS_QUEUED, &iQueuedBuffers);
            
            if(iQueuedBuffers)
            {
                alSourcePlay(uiSource);//buffers have data, play it
            }
            else
            {
                //there is no data any more
                break;
            }
        }
    }
    //close mpg123
    mpg123_close(mpg123);
    //stop the source and clear the queue
    alSourceStop(uiSource);
    alSourcei(uiSource, AL_BUFFER, 0);

    free(pData);
    pData = NULL;


    

    return 0;
}

