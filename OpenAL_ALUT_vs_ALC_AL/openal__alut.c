/*
 * 
 *  Ací estic replegant les funcions del API de alt nivell d'OpenAL: ALUT
 * 
 *  Tret de la pràctica de SMII
  
 $ pkg-config openal --cflags --libs
-I/usr/include/AL -lopenal + -lalut
 $ gcc openal__alut.c -o openal__alut  $(pkg-config freealut openal --cflags --libs)
*/


#include <stdio.h>
#include <stdlib.h>     // Per al 'exit'
#include <AL/alut.h>

// #include <string.h> //strlen

//#include <unistd.h>
//#include <math.h>


int main (int argc, char **argv)
{
//   int i;
//   const ALchar *pDeviceList, *llista;
  ALuint elBuffer[3], laFont;
  ALenum error;
  ALint sourceState;


  alutInit (&argc, argv);
  error = alGetError(); 
  if (error) printf("Error en alutInit: %s\n", alutGetErrorString(error));
       
  //alutSleep( segonsEnPosiblePartDecimal );
  //usleep( 1000000 ); printf(" \r"); fflush( stdout );
  
  printf ("La versio de ALUT instalada es %d.%d \n", alutGetMajorVersion(), alutGetMinorVersion());

  printf("OpenAL Renderer is '%s'\n", alGetString(AL_RENDERER) );
  printf("OpenAL Version is '%s'\n", alGetString(AL_VERSION) );		    
  printf("OpenAL Vendor is '%s'\n", alGetString(AL_VENDOR) );

  printf("ALUT MIME types para ALUT_LOADER_BUFFER: %s\n", alutGetMIMETypes(ALUT_LOADER_BUFFER) );
//   ALUT MIME types para ALUT_LOADER_BUFFER: audio/basic,audio/x-raw,audio/x-wav
  printf("ALUT MIME types para ALUT_LOADER_MEMORY: %s\n", alutGetMIMETypes(ALUT_LOADER_MEMORY) );
//   ALUT MIME types para ALUT_LOADER_MEMORY: audio/basic,audio/x-raw,audio/x-wav


  // Que sone algo, no?
  alGenSources (1, &laFont);
//   alGenBuffers(3,&elBuffer);
  
//   // Sonit guardat en un vector ... 
  elBuffer[0] = alutCreateBufferHelloWorld ();
  error = alGetError(); 
  if (error) printf("Hello World: %s\n", alutGetErrorString(error));
  alSourcei( laFont, AL_BUFFER, elBuffer[0]);
  alSourcePlay( laFont );
  alutSleep( 1 );
//   alGetSourcei( laFont, AL_SOURCE_STATE, &sourceState);
//    while (sourceState == AL_PLAYING){
//     alutSleep( 1 );
//     alGetSourcei( laFont, AL_SOURCE_STATE, &sourceState);
//    }
//   alDeleteBuffers( 1, &elBuffer );
  printf("Hello World.\n");
  alSourceStop( laFont );
 
  // Sonit generat en temps d'execució 
  // Per que la comparació siga més justa
//   alGenBuffers(1,&elBuffer);
  elBuffer[1] = alutCreateBufferWaveform(ALUT_WAVEFORM_SINE, 440.0, 0.0, 1.0);
  // ??? frecMostreig, tamany de mostra, canals???
  error = alGetError(); 
  if (error) printf("Waveform: %s\n", alutGetErrorString(error));
  alSourcei (laFont, AL_BUFFER, elBuffer[1]);
  alSourcePlay( laFont );
  alutSleep( 1 );
//   alGetSourcei( laFont, AL_SOURCE_STATE, &sourceState);
//    while (sourceState == AL_PLAYING){
//     alutSleep( 1 );
//     alGetSourcei( laFont, AL_SOURCE_STATE, &sourceState);
//    }
//   alDeleteBuffers(1, &elBuffer);
  printf("Senyals bàsiques.\n");
  alSourceStop( laFont );

  // Sonit tret d'un fitxer 
//   alGenBuffers(1,&elBuffer);
  elBuffer[2] = alutCreateBufferFromFile( "footsteps-4.wav" ); //  RIFF (little-endian) data, WAVE audio, Microsoft PCM, 16 bit, stereo 48000 Hz   
  error = alGetError(); 
  if (error) printf("FromFile: %s\n", alutGetErrorString(error));
    
  alSourcei(laFont, AL_BUFFER, elBuffer[2]);
  alSourcePlay (laFont);
  // Mentres n'hi haja datos que no pare
  alGetSourcei( laFont, AL_SOURCE_STATE, &sourceState);
  while (sourceState == AL_PLAYING){
    alutSleep( 1 );
    alGetSourcei( laFont, AL_SOURCE_STATE, &sourceState);
  }
  printf("Fitxer.\n");

  alDeleteBuffers(3, elBuffer);
  alDeleteSources(1, &laFont);


  alutExit();
  return EXIT_SUCCESS;
} // fi de main
