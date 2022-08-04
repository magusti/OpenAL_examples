/*
 Versio en C de l'exemple de 
Introduction to Ogg Vorbis [Categoria: General and Gameplay Programming]
https://www.gamedev.net/articles/programming/general-and-gameplay-programming/introduction-to-ogg-vorbis-r2031/
En 23/03/2018 no es pot baixar el fitxer original de 
http://downloads.gamedev.net/features/snippets/SimpleOGG-2031.zip
El servidor no contesta.

December 29, 2003
Anthony "TangentZ" Yuen <tangentz@hotmail.com>
gcc -g SimpleOGG.c -o SimpleOGG -lalut -lopenal -lvorbisfile -lvorbis -lm -logg


$ make
gcc SimpleOGG.c -o SimpleOGG -lalut -lopenal -lvorbisfile -lvorbis -lm -logg
$ SimpleOGG drHouse.ogg 
$ SimpleOGG /usr/share/sounds/KDE-Sys-Log-Out.ogg 


Idees per a fer una funcio propia: fonts.txt
@ultravioleta:~/docencia/2k17-2k18/docenciaEnRed/copiesCodicFont_OpenAL_FreeALUT/openal-svn-mirror-master/alut/src/alutBufferData.c
 */

#include <stdio.h>
#include <stdlib.h> // malloc
#include <AL/al.h>
#include <AL/alut.h>
#include <vorbis/vorbisfile.h>
//#include <math.h>
//#include "vorbis/codec.h"


#define BUFFER_SIZE   32768     // 32 KB buffers

//using namespace std;
#define TAMANYMOSTRES 2 // En bytes
#define SENSESIGNE    1 // Per a les mostres (samples)



char *errorString(int code)
{
    switch(code)
    {
        case OV_EREAD:
	  return (char *)("Read from media.");
        case OV_ENOTVORBIS:
            return ("Not Vorbis data.");
        case OV_EVERSION:
            return ("Vorbis version mismatch.");
        case OV_EBADHEADER:
            return ("Invalid Vorbis header.");
        case OV_EFAULT:
            return ("Internal logic fault (bug or heap/stack corruption.");
        default:
            return ("Unknown Ogg error.");
    }
}

/*
https://stackoverflow.com/questions/2838038/c-programming-malloc-inside-another-function
C Programming: malloc() inside another function


I need help with malloc() inside another function.

I'm passing a pointer and size to the function from my main() and I would like to allocate memory for that pointer dynamically using malloc() from inside that called function, but what I see is that.... the memory, which is getting allocated, is for the pointer declared within my called function and not for the pointer which is inside the main().

How should I pass a pointer to a function and allocate memory for the passed pointer from inside the called function?

You need to pass a pointer to a pointer as the parameter to your function.
*/  
//void LoadOGG(char *fileName, char *buffer, ALenum *format, ALsizei *freq)
long LoadOGG(char *fileName, char **buffer, ALenum *format, ALsizei *freq)
{
  int endian = 0;             // 0 for Little-Endian, 1 for Big-Endian
  int bitStream;
  long bytesLlegits;
  char array[BUFFER_SIZE];    // Local fixed size array
  FILE *f;
  char *p;
  long tamanyAudioEnBytes;
  long tamanyAudioEnBytesLlegit;
  //Next, we declare some variables that the Ogg Vorbis SDK uses.
  vorbis_info *pInfo;
  OggVorbis_File oggFile;
  int result;
  long nMostres;
  long  nFramesLlegits;

  
  printf( "Obrint %s\n", fileName );
  // Open for binary reading
  f = fopen(fileName, "rb");
  if (f == NULL)
  {
   printf("Could not open Ogg file.\n");
   exit( 1 );
  }

  //Up to this point, things should look very familiar. The function simply uses the fopen() function to open the given file for binary reading.
  //Opening file for decoding
  //Then comes the act of passing control over to the SDK. Note that there is no need to call fclose() anymore once this is done.

  if ((result = ov_open(f, &oggFile, NULL, 0)) < 0 )
  {
   fclose(f);      
   printf("Could not open Ogg stream: %s\n", errorString(result));
   exit( 2 );
  }

  //Information retrieval
  //After opening the file for decoding, we can extract a little bit of information about the compressed audio data in the file. At the very least, we need to know the number of channels (1 for mono and 2 for stereo) and the sampling frequency of the audio data. We can do it like this:

  // Get some information about the OGG file
  pInfo = ov_info(&oggFile, -1);

  // Check the number of channels... always use 16-bit samples
  if (pInfo->channels == 1)
    *format = AL_FORMAT_MONO16;
  else
    *format = AL_FORMAT_STEREO16;
  // end if

  // The frequency of the sampling rate
   *freq = pInfo->rate;

  //
  // Mostrar informacio
  //
  printf( "%s: 16 bits sempre? (sí, si no operes en floats); canals %d frequencia %ld mostres %ld\n",
	  fileName, pInfo->channels, pInfo->rate, 
          (long)ov_pcm_total(&oggFile, -1) );
  //https://xiph.org/vorbis/doc/vorbisfile/ov_pcm_total.html
  //tamanyAudioEnBytes = (long)ov_pcm_total(&oggFile, -1) * TAMANYMOSTRES * pInfo->channels; // * 100000;
  tamanyAudioEnBytes = ((long)pInfo->channels * 2 * (long)ov_pcm_total(&oggFile,-1));
    //    tamanyAudioEnBytes = ((long)ov_pcm_total(&oggFile,-1));
  
	  // * pInfo->channels * pInfo->rate,
  printf("malloc de %ld == %ld pcmlengths[0] o %f timeTotal (en segons) \n", 
         tamanyAudioEnBytes,
         oggFile.pcmlengths[0],
         ov_time_total(&oggFile,-1) );
  //https://xiph.org/vorbis/doc/vorbisfile/OggVorbis_File.html
  //https://xiph.org/vorbis/doc/vorbisfile/fileinfo.html

  //	  pInfo->channels * pInfo->rate * TAMANYMOSTRES, ); 
  fflush( stdout ); 
  //buffer = (char *)malloc( tamanyAudioEnBytes * sizeof(char) );
  *buffer = (char *)malloc( tamanyAudioEnBytes );
  /* Podria haver anant fent realloc, pero ho veig mes directe aixina
   */

  if (*buffer == NULL)
  {
    printf("LoadOGG no ha pogut reservar espai per a %ld\n", tamanyAudioEnBytes);
    *format = 0;
    *freq = 0;
    exit( 3 );
  }
  printf("decodificar\n"); fflush( stdout );
  //Decoding the data
  // Especificat en https://xiph.org/vorbis/doc/vorbisfile/decoding.html
  //Now we are ready to decode the OGG file and put the raw audio data into the buffer. We use a fixed size buffer and keep on reading until there is no more data left, like this:
    /*
https://xiph.org/vorbis/doc/vorbisfile/ov_read.html
long ov_read(OggVorbis_File *vf, char *buffer, int length, int bigendianp, int word, int sgned, int *bitstream);
 It returns up to the specified number of bytes of decoded PCM audio in the requested endianness, signedness, and word size. 
If the audio is multichannel, the channels are interleaved in the output buffer. If the passed in buffer is large, ov_read() will not fill it; the passed in buffer size is treated as a limit and not a request.
The output channels are in stream order and not remapped. Vorbis I defines channel order as follows:

one channel - the stream is monophonic
two channels - the stream is stereo. channel order: left, right
...

Parameters
vf        A pointer to the OggVorbis_File structure--this is used for ALL the externally visible libvorbisfile functions.
buffer    A pointer to an output buffer. The decoded output is inserted into this buffer.
length    Number of bytes to be read into the buffer. Should be the same size as the buffer. A typical value is 4096.
bigendian Specifies big or little endian byte packing. 0 for little endian, 1 for b ig endian. Typical value is 0.
word      Specifies word size. Possible arguments are 1 for 8-bit samples, or 2 or 16-bit samples. Typical value is 2.
sgned     Signed or unsigned data. 0 for unsigned, 1 for signed. Typically 1.
bitstream A pointer to the number of the current logical bitstream. 

bytes_read = ov_read(&vf, buffer, 4096,0,2,1,&current_section)
This reads up to 4096 bytes into a buffer, with signed 16-bit little-endian samples.
    */


  printf("Llegint OGG \r"); fflush( stdout );
  p = *buffer;
  nMostres = 0;
  tamanyAudioEnBytesLlegit = 0;
  nFramesLlegits = 0;
  /*
  do {
    //    printf("ovread \n"); fflush( stdout );
    // Read up to a buffer's worth of decoded sound data
    //    bytes = ov_read(&oggFile, p, BUFFER_SIZE, endian, TAMANYMOSTRES, SENSESIGNE, &bitStream);
    //    bytes = ov_read(&oggFile, p, BUFFER_SIZE, tamanyAudioEnBytes-tamanyAudioEnBytesLlegit, endian, SENSESIGNE, &bitStream);
    bytes = ov_read(&oggFile, p, BUFFER_SIZE, endian, TAMANYMOSTRES, SENSESIGNE, &bitStream);

    //printf("ovread %ld bytes\n", bytes); fflush( stdout );
    // Append to end of buffer
    //buffer.insert(buffer.end(), array, array + bytes);  
    if( (tamanyAudioEnBytesLlegit + bytes) > tamanyAudioEnBytes)
      break;

    p += bytes;
    tamanyAudioEnBytesLlegit += bytes;
    printf("Llegint OGG %ld bytes: %ld - %ld --> %ld == %ld de %ld\r", 
            (long)bytes, 
            (long)&p, (long)&buffer, (long)&p - (long)&buffer, 
	   tamanyAudioEnBytesLlegit, tamanyAudioEnBytes); 
    fflush( stdout );
    // Posible formateig??? fprintf(stderr, "\r\t[%5.1f%%]", (double)percent/2.);
  } while (bytes > 0);
  */

  // 
  // A l'estil
  // OpenAL_OGG/devmaster__OpenALLessons/lesson8__OggVorbisStreamingUsingTheSourceQueue/lesson8/
  //
   while(tamanyAudioEnBytesLlegit < tamanyAudioEnBytes)
    {
      bytesLlegits = ov_read(&oggFile, *buffer + tamanyAudioEnBytesLlegit, tamanyAudioEnBytes - tamanyAudioEnBytesLlegit, 0, 2, 1, &bitStream);
      //bytesLlegits = ov_read(&oggFile, buffer+tamanyAudioEnBytesLlegit, 4096,0,2,1,&bitStream);

        nFramesLlegits++; 
        
        if( bytesLlegits > 0 )
	  {
	    tamanyAudioEnBytesLlegit += bytesLlegits;
            printf("Frames %ld [%ld de %ld][%3.2f%%]  \r",  // Mirar com queda en  \n
		   nFramesLlegits,
		   bytesLlegits,
		   tamanyAudioEnBytes,
		   ((float)tamanyAudioEnBytesLlegit /tamanyAudioEnBytes) * 100.0  ); 
	  }
        else
            if(bytesLlegits < 0)
	      {printf("Conter: bytesLlegits < 0\n"); break; }
            else
	      {printf("Conter: bytesLlegits == 0\n"); break; }
               
    }
    
   // No:   bytes += nMostres * TAMANYMOSTRES * pInfo->channels
   printf("\n Llegits %ld de %ld: falten %ld\n\n",
	  tamanyAudioEnBytesLlegit, tamanyAudioEnBytes,
	  tamanyAudioEnBytes - tamanyAudioEnBytesLlegit );

//Clean up
  //Now all the audio data has been decoded and stuffed into the buffer. We can release the file resources (resource leaks are bad!).
//Note that there is no need to call fclose(). It is already done for us. Neat.

  ov_clear(&oggFile);

  //  return( tamanyAudioEnBytes );
  return( tamanyAudioEnBytesLlegit );
}



int main(int argc, char *argv[])
{
  ALint state;                // The state of the sound source
  ALuint bufferID;            // The OpenAL sound buffer ID
  ALuint sourceID;            // The OpenAL sound source
  ALenum format;              // The sound data format
  ALsizei freq;               // The frequency of the sound data
  //vector < char > bufferData; // The sound buffer data from file
  char  *bufferData;          // The sound buffer data from file
  //char  bufferData[1024*1024*2*10]; // The sound buffer data from file
  //char bufferData[2557094];
  long tamanyBuffer;
  char barraActivitat[4] = {'|', '/', '-', '\\'};
  int i, error;


  // Initialize the OpenAL library
  alutInit(&argc, argv);
  error = alGetError(); 
  if (error) printf("Error al inicialitzar ALUT:: alGetError: %s\n", alutGetErrorString(error));

  // Create sound buffer and source
  alGenBuffers(1, &bufferID);
  error = alGetError(); 
  if (error) printf("Error al crear el buffer:: alGetError: %s\n", alutGetErrorString(error));

  alGenSources(1, &sourceID);
  error = alGetError(); 
  if (error) printf("Error al crear la font:: alGetError: %s\n", alutGetErrorString(error));

  // Set the source and listener to the same location
  alListener3f(AL_POSITION, 0.0f, 0.0f, 0.0f);
    //  alSource3f(sourceID, AL_POSITION, 0.0f, 0.0f, 0.0f);
  error = alGetError(); 
  if (error) printf("Error al definir les posicions:: alGetError: %s\n", alutGetErrorString(error));

  /*
Decoding OGG files
Opening file for binary reading
At this point, the system is all ready to go. The one thing that is missing is the actual sound data! OK, let's write a function that can load OGG files into memory.			
  */
  tamanyBuffer = LoadOGG(argv[1], &bufferData, &format, &freq);

  if (tamanyBuffer == 0)
  {
    printf("Fitxer %s buit? Acabe aci!\n", argv[1]);
    alutExit();
    exit(4);
  }
  
  printf("Asociant bufferData %p de %ld bytes, canals %d (%d o %d), freq. %d\n",
	 bufferData, tamanyBuffer, format, AL_FORMAT_MONO16, AL_FORMAT_STEREO16, freq);
  
/*
Playing the sound
It is now time to get back to our main(). The next step is to upload the raw audio data to the OpenAL sound buffer and attach the buffer to the source.
  */

  // Upload sound data to buffer
  //  alBufferData(bufferID, format, &bufferData[0], static_cast < ALsizei > (bufferData.size()), freq);
    alBufferData(bufferID, (int)format, (char *)bufferData, tamanyBuffer, (int)freq);
  error = alGetError(); 
  if (error) printf("Error al enllaçar PCM:: alGetError: %s\n", alutGetErrorString(error));

  printf("%s associat al buffer\n", argv[1]);
  //    alDeleteBuffers(1, &bufferID);
    //  bufferID = alutCreateBufferFromFile( "drHouse.wav" );

  // Attach sound buffer to source
  alSourcei(sourceID, AL_BUFFER, bufferID);
  alSource3f(sourceID, AL_POSITION, 0.0f, 0.0f, 0.0f);

  printf("Finally! We are ready to play the sound! Let's do that!\n");

  // Finally, play the sound!!!
  alSourcePlay(sourceID);
  printf("Finallyyyyyyyyyyyyyyyyyyyyyyyyyyyy\n");
  // This is a busy wait loop but should be good enough for example purpose
  i = 0;
  do {
    alutSleep( 0.1f );
    printf("%c\r", barraActivitat[i++]); fflush( stdout );
    i = (i%4);
    // Query the state of the souce
    alGetSourcei(sourceID, AL_SOURCE_STATE, &state);
  } while (state != AL_STOPPED);
  //  } while (state == AL_PLAYING); 
  printf("\n. S'ha acabat!\n");

  //After the sound is finished playing, we should clean up.

  // Clean up sound buffer and source
  alDeleteSources(1, &sourceID);
  alDeleteBuffers(1, &bufferID);


  // Clean up the OpenAL library
  alutExit();

  if ( !bufferData )
   free( bufferData );

  return 0;
} // end of main


