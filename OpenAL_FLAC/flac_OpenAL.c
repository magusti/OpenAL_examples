/*
 Idees de magusti@verdet:~/docencia/2k20-2k21/docenciaEnXarxa/articles/flac__OpenAL/flac-master/src/flac/
 Documentació API  1.3.1 https://xiph.org/flac/api/
 FLAC is an Open Source lossless audio codec developed by Josh Coalson from 2001
to 2009.
ddiakopoulos / libnyquist
https://github.com/ddiakopoulos/libnyquist/blob/master/src/FlacDecoder.cpp


From January 2012 FLAC is being maintained by Erik de Castro Lopo under the
auspices of the Xiph.org Foundation.

FLAC is comprised of
  * `libFLAC', a library which implements reference encoders and
    decoders for native FLAC and Ogg FLAC, and a metadata interface
  * `libFLAC++', a C++ object wrapper library around libFLAC
  * `flac', a command-line program for encoding and decoding files
  * `metaflac', a command-line program for viewing and editing FLAC
    metadata
  * player plugin for XMMS
  * user and API documentation

The libraries (libFLAC, libFLAC++) are
licensed under Xiph.org's BSD-like license (see COPYING.Xiph).  All other
programs and plugins are licensed under the GNU General Public License
(see COPYING.GPL).  The documentation is licensed under the GNU Free
Documentation License (see COPYING.FDL).


===============================================================================
FLAC - 1.3.3 - Contents
 
 gcc flac_OpenAL.c -l flac_OpenAL  `pkg-config flac --cflags --libs`
 gcc flac_OpenAL.c -l flac_OpenAL  -lFLAC
 gcc flac_OpenAL.c -o flac_OpenAL  -lalut -Iflac-master/include/ `pkg-config flac --cflags --libs` `pkg-config openal --cflags --libs` && flac_OpenAL  drHouse.flac 

 Si gaste el InitAL de openal_soft argv[0] [-device nom] fixer.flac
 
 https://mackron.github.io/dr_flac
 dr_flac is an open source library for decoding FLAC files. It's written in C in a single file with no dependencies except for the C standard library, and is released into the public domain.
 

 
 About audio formats FLAC and MP3
 https://www.eclassical.com/pages/24-bit-faq.html
 
 
 https://filesamples.com/formats/flac
 Symphony No.6 (1st movement).flac
 
 [Ludwig van B.]
 Beethoven: Symphony No. 6 "Pastoral", first movement - Itzhak Perlman
 https://www.youtube.com/watch?v=hvbRwb8twiA
*/



#include <stdio.h>
#include <stdlib.h>
#include <unistd.h> // usleep


// #include "share/compat.h"
// #include "FLAC/stream_decoder.h"

 #define DR_FLAC_IMPLEMENTATION
#include "dr_flac.h"


// #include <string.h>
// #include "AL/al.h"
// #include "AL/alc.h"
// #include "AL/alext.h"
#include <AL/alut.h>



ALuint flacCreateBufferFromFile( char *argv );
int openal_info (int argc, char **argv);


// main ALUT + drFlac    
int main(int argc, char **argv) {
    ALuint source, bufferOpenAL;
//    ALfloat offset;
    //ALenum state;
//     ALenum err, format;
    ALint state;
    // ../midi_en_OpenAL/wildmidi_openal.c
    static char spinner[] = "|/-\\";
    static int spinpoint = 0;

    
    if (argc <= 1)     {
        openal_info(argc, argv);
                printf("Usage: %s <filename>\n", argv[0]);
        return -1;
    }
    
    
  	// Initialize OpenAL
    printf("obrir OpenAL?\n");
    alutInit (&argc, argv); 
     
   
	// Load the sound into a buffer. 
    bufferOpenAL = flacCreateBufferFromFile( argv[1] );  
    if( !bufferOpenAL )     {
        printf("No bufferOpenAL?\n");
        alutExit ();
        return( 1 );
    }
    printf("He tornat de flacCreateBufferFromFile\n");


   alGenSources(1, &source);
   alSourcei(source, AL_BUFFER, bufferOpenAL);

   printf("play!!!!\n");
 
   alSourcePlay(source);
   state = AL_PLAYING;
   alSourcef(source, AL_GAIN, 0.5);
   while ( state == AL_PLAYING ) {
       printf("%13d %c\r", spinpoint, spinner[spinpoint++ % 4]); fflush(stdout);       
       usleep(1000); // microsecond
       alGetSourcei(source,AL_SOURCE_STATE,&state);
   }
   
   printf("S'ha acabat! \n");
    
    
   alDeleteSources(1, &source);
   alDeleteBuffers(1, &bufferOpenAL);
   alutExit ();
   return EXIT_SUCCESS;

} // main_ALUT_drFlac




ALuint flacCreateBufferFromFile( char *fitxer ) {
    ALuint bufferOpenAL;
    ALenum err, format;
    
    drflac* pFlac = drflac_open_file(fitxer, NULL);
    if (pFlac == NULL) {
        printf("No puc llegir %s\n", fitxer );
        return -1;
    }
    printf("%s: totalPCMFrameCount %llu channels %d \n", 
           fitxer, pFlac->totalPCMFrameCount, pFlac->channels );

    int16_t* pSampleData = (int16_t*)malloc((size_t)pFlac->totalPCMFrameCount * pFlac->channels * sizeof(int16_t));
    drflac_read_pcm_frames_s16(pFlac, pFlac->totalPCMFrameCount, pSampleData);
//     int32_t* pSampleData = (int32_t*)malloc((size_t)pFlac->totalPCMFrameCount * pFlac->channels * sizeof(int32_t));
    //drflac_read_pcm_frames_s32(pFlac, pFlac->totalPCMFrameCount, pSampleData);
    // At this point pSampleData contains every decoded sample as signed 32-bit PCM.
   if (pFlac->channels == 2)
            format = AL_FORMAT_STEREO16;
        else
            format = AL_FORMAT_MONO16;
            
 	alGenBuffers(1, &bufferOpenAL);
    err = alGetError(); 
    if (err) {
        printf("%s\n", alutGetErrorString(err));
        return -1;
    }
    printf("Asignar PCM a buffer\n");
    alBufferData(bufferOpenAL, format, pSampleData, pFlac->totalPCMFrameCount,  44100 ); 
// Dona igual    alBufferData(bufferOpenAL, format, pSampleData, pFlac->totalPCMFrameCount* pFlac->channels * sizeof(int32_t),  44100 ); 
//     printf("alutCreateBufferFromFile --> %s\n", "drHouse.wav");
//     bufferOpenAL = alutCreateBufferFromFile( "drHouse.wav" );

    drflac_free(pSampleData, NULL);
    drflac_close(pFlac);

   
//     unsigned int channels;
//     unsigned int sampleRate;
//     drflac_uint64 totalPCMFrameCount;
//     drflac_int32* pSampleData = drflac_open_file_and_read_pcm_frames_s16(fitxer, &channels, &sampleRate, &totalPCMFrameCount, NULL);
//  
//     if (channels == 2)
//             format = AL_FORMAT_STEREO16;
//         else
//             format = AL_FORMAT_MONO16;
//             
//  	alGenBuffers(1, &bufferOpenAL);
//     err = alGetError(); 
//     if (err) {
//         printf("%s\n", alutGetErrorString(err));
//         return -1;
//     }
//     printf("Asignar PCM a buffer\n");
//     alBufferData(bufferOpenAL, format, pSampleData, totalPCMFrameCount,  sample_rate );  
//     
//     drflac_free(pSampleData, NULL);
// //   drflac_close(pFlac);*/

   
   //else
   return bufferOpenAL;

    
} // flacCreateBufferFromFile



// ~/docencia/2k20-2k21/SMII/practiques/practica3_Audio/practica03a_introOpenAL/openal_info.c
int openal_info (int argc, char **argv)
{
  int i;
  const ALchar *pDeviceList, *llista;

  //const ALchar *versio;
  //const ALchar *extensions;

  alutInit (&argc, argv);
    
  //alutSleep( segonsEnPosiblePartDecimal );
  //usleep( 1000000 ); printf(" \r"); fflush( stdout );
  
  printf("Versió OpenAL: %s\n", alGetString( AL_VERSION ) );
  //versio = alGetString(AL_VERSION);
  //printf("OpenAL Version is '%s' \n", versio); 

  printf("OpenAL Renderer is '%s'\n", alGetString(AL_RENDERER) );
  printf("OpenAL Version is '%s'\n", alGetString(AL_VERSION) );		    
  printf("OpenAL Vendor is '%s'\n", alGetString(AL_VENDOR) );
  //
  printf("OpenAL Extensions supported are :\n%s\n", alGetString(AL_EXTENSIONS) );
  llista = alGetString(AL_EXTENSIONS);
  i=1;
  if (llista)
    {
      printf("\nExtensions disponibles per separat:");
      printf("\n%2d - ", i);
      while (*llista)
        {
          if (*llista != ' ' )
	    printf("%c", *llista);
	  else
	    printf("\n%2d - ", ++i);
	  llista++;
        }
    }
  printf("\n");
  //
  printf("OpenAL hardware presente:\n %s\n", alcGetString(NULL, ALC_DEVICE_SPECIFIER) );
  // Get list of available Capture Devices (Tret de Capture.c)
  pDeviceList = alcGetString(NULL, ALC_DEVICE_SPECIFIER);
  i=1;
  if (pDeviceList)
    {
      printf("\nDrivers disponibles d'audio:\n");

      while (*pDeviceList)
        {
          printf("%2d - %s\n", i, pDeviceList);
          pDeviceList += strlen(pDeviceList) + 1;
	  i++;
        }
    }

 printf("Per defecte:\n%s\n", alcGetString(NULL, ALC_DEFAULT_DEVICE_SPECIFIER) );
 // Al lab. diu:   '((sampling-rate 44100) (device '(native))

  // Get list of available Capture Devices (Tret de Capture.c)
  //printf(" Llistat de dispositius de captura (entrada de audio, micròfons):\n");
  pDeviceList = alcGetString(NULL, ALC_CAPTURE_DEVICE_SPECIFIER);
  i=1;
  if (pDeviceList)
    {
      printf("\nAvailable Capture Devices are:\n");

      while (*pDeviceList)
        {
          printf("%2d - %s\n", i, pDeviceList);
          pDeviceList += strlen(pDeviceList) + 1;
	  i++;
        }
    }


  printf("\n");
  printf("\n");

  printf("Versió ALUT: %d.%d\n", alutGetMajorVersion(), alutGetMinorVersion() );
  
  printf("\n");
  
  alutExit();
  return EXIT_SUCCESS;
} // fi de openal_info
