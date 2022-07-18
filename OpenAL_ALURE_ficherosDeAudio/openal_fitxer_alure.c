/*
 * gcc openal_fitxer_alure.c -o openal_fitxer_alure $(pkg-config alure openal --cflags --libs)
 */
#include <stdlib.h>
#include <stdio.h>
#include <AL/alure.h>
#include <string.h>
#include <AL/al.h>
#include <AL/alext.h>

// void load_i_play__alure( char * rutaFitxer )
// {
//     ALuint source, buffer;
//     ALenum state;
//     ALfloat offset;
// 
//     buffer = 0;
//     alGenBuffers(1, &buffer);    
//     buffer = alureCreateBufferFromFile( rutaFitxer );
//     alGenSources(1, &source);
//     alSourcei(source, AL_BUFFER, (ALint)buffer);
//     
//     alSourcePlay(source);
//     do {
//         //al_nssleep(10000000);
//         alureSleep( 0.1 );
//         alGetSourcei(source, AL_SOURCE_STATE, &state);
// 
//         /* Get the source offset. */
//         alGetSourcef(source, AL_SEC_OFFSET, &offset);
//         printf("\rOffset: %f  ", offset);
//         fflush(stdout);
//     } while(alGetError() == AL_NO_ERROR && state == AL_PLAYING);
//     printf("\n");
// 
//     /* All done. Delete resources, and close down OpenAL. */
//     alDeleteSources(1, &source);
//     alDeleteBuffers(1, &buffer);
// } // load_i_play__alure( char * rutaFitxer )
// 

int main(int argc, char **argv)
{
  ALuint ALURE_major, ALURE_minor;
  ALuint source, buffer;
  ALenum state, error;
  ALfloat offset;
  
  if (argc == 1)  {
     printf("Falta un parámetro\n: %s nombreFichero\n", argv[0] );
     return( -1 );
  }
  
  if(!alureInitDevice(NULL, NULL))   {
        fprintf(stderr, "ALURE:: Failed to open OpenAL device: %s\n", alureGetErrorString());
        return( -1);
  }

   
  printf("Reprodueix %s\n", argv[1] );
  buffer = 0;
  alGenBuffers(1, &buffer);    
  buffer = alureCreateBufferFromFile( argv[1] );
  alGenSources(1, &source);
  alSourcei(source, AL_BUFFER, (ALint)buffer);
    
  alSourcePlay(source);
  do {
        alureSleep( 0.1 );
        alGetSourcei(source, AL_SOURCE_STATE, &state);

        /* Get the source offset. */
        alGetSourcef(source, AL_SEC_OFFSET, &offset);
        printf("\rOffset: %f  ", offset);
        fflush(stdout);
  } while(alGetError() == AL_NO_ERROR && state == AL_PLAYING);
  printf("\n");

  /* All done. Delete resources, and close down OpenAL. */
  alDeleteSources(1, &source);
  alDeleteBuffers(1, &buffer);
    
  // Comprobem que sona alguna cosa ...
  /*printf("Reprodueix WAV.\n");  load_i_play__alure("drHouse.wav");
  printf("Reprodueix OGG.\n");  load_i_play__alure("drHouse.ogg");
  printf("Reprodueix FLAC.\n"); load_i_play__alure("drHouse.flac");
  printf("Reprodueix MP3.\n");  load_i_play__alure("drHouse.mp3");     
  */  
    
  if( alureShutdownDevice() == AL_FALSE )  { 
       printf(" ALURE:: error al lliberar el dispositiu.\n" );
       return( -5 );        
  }    

  return 0;
}
