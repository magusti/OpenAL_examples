/*
 * 
 *  gcc openal__alc_al.c -o openal__alc_al $(pkg-config openal --cflags --libs)  -lm && openal__alc_al
 *  Ací estic replegant les funcions d'inicialització i tancament en el API de baix nivell d'OpenAL mitjançant ALC+AL
 * 
 *  COMPROVA-HO DELS EXEMPLES D'OpenAL i de la documentació
 * 
 $ gcc openal_init_info_alc_al.c -o openal_init_info_alc_al -lalut $(pkg-config openal --cflags --libs) -lm && openal_init_info_alc_al
 * 
 * 
 * 
 * 
 * 
 * Fuentes:
 * OpenAL Programmer's Guide. OpenAL Versions 1.0 and 1.1 [[pàg. 10-11]]
 * 
// Initializing/Exiting
// As described above, the first step to initializing OpenAL is to open a device. Once that is
// successfully done, then a context is opened on that device. Now the fundamental OpenAL
// objects can be managed ? the listener, various sources, and various buffers.
// To generate a set of buffers for use, use alGetError to reset the error state, call alGenBuffers to
// generate the number of buffers desired, and then use alGetError again to detect if an error was
// generated.
// Fill the buffers with PCM data using alBufferData.
// To generate a set of sources for use, use alGetError to reset the error state, call alGenSources to
// generate the number of sources desired, and then use alGetError again to detect if an error was
// generated.
// Buffers are attached to sources using alSourcei.
// Once a buffer has been attached to a source, the source can play the buffer using alSourcePlay.
// Source and Listener properties can be updated dynamically using property set and get calls such
// as alGetListenerfv, alListener3f, alSourcei, and alGetSource3f.



 * Threaeam. Reading Line-OUT (2011) <DevMaster.net Forums > Programming & Development > Sound and Music Programming > OpenAL>
 **  No més que gasta el micro, té errors en la configuració del micro i en el còdic de openal_fft
 *
 * Vaig a pasar-ho a fitxer per comprovar en tonos simples que funciona la FFT
 ** ~/docencia/2k20-2k21/SMII/practiques/practica3_Audio/practica03a_introOpenAL/openal_fitxer.c
 ** Revisar el còdic de la FFT en el de numerical recipies i el del treball de Carles i RAfa
 ** Llegir un wav a trocets
 *** http://www.mega-nerd.com/libsndfile/
 *** https://github.com/libsndfile/libsndfile/
 
 
 g++ openal_fft__streaming.c -o openal_fft__streaming -lalut `pkg-config openal --cflags --libs` -lglut -lGL -lsndfile && openal_fft__streaming tone4400.wav

 
  //UINT8_MAX UINT16_MAX https://en.cppreference.com/w/c/types/integer
 
 
 
 *$ pkg-config openal --cflags --libs
-I/usr/include/AL -lopenal
 $ g++ openal_fft.c -o openal_fft -lalut `pkg-config openal --cflags --libs` -lglut -lGL
Sobre el còdic original cal mesclar-lo en el de streaming, reajustar l'escala i revisar el càlcul de la FFT. 
 Llevar el pintar de la onda? Deixar-ho per a un article a on es veja como a resultat de l'ecualització?

  $ g++ `pkg-config openal --cflags --libs` -lalut -lopenal -L/usr/X11R6/lib -lglut -lGLU -lGL -lXmu -lXt -lSM -lICE -lXext -lX11 -lXi -lXext -lX11 -lm fft.cpp -o fft

 $ g++ `pkg-config openal --cflags --libs` `pkg-config freealut --cflags --libs` -L/usr/X11R6/lib -lglut -lGLU -lGL -lXmu -lXt -lSM -lICE -lXext -lX11 -lXi -lX11 -lm  openal_fft.c -o openal_fft

*/

#include <stdio.h>
#include <stdlib.h> //EXIT_SUCCESS
#include <math.h>
// #include <vector>
// #include <GL/glut.h>
//#include <AL/al.h>
//#include <AL/alc.h>  // Ja els porten darrere en alut.h
#include <AL/alut.h>
#include <string.h> //strlen
#include <unistd.h> // sleep
#include <limits.h> // UCHAR_MAX


// Me la invente
// file:///home/magusti/docencia/2k21-2k22/docenciaEnXarxa/articles/pintarElEspectreDeFrequencies/openal-1.1-specification.pdf
// 2.7. AL Errors
void DisplayALError( char *titolMensatge, ALenum error) {
//     char descripcioError[1024];
//     sprintf(descripcioError, "%s", error);
    printf("%s %s\n", titolMensatge, alGetString( error ) );
}// fi de DisplayALError

//  aib / openal-surround-test
//https://github.com/aib/openal-surround-test
#define TAMANY_BUFFER  100*1024
#define SAMPLERATE     22050
// #define SAMPLESPERBUF  (SAMPLERATE / 30)
#define BYTESPERSAMPLE 1
#define NCANALS        1
// Tens atres idees en https://github.com/vancegroup/freealut/blob/master/src/alutWaveform.c
void fill_buffer(void *audioBuffer, size_t size, int frequency ) {
        char *dest = (char *)audioBuffer;

        for (int i=0; i<size; i++) {
                dest[i] = UCHAR_MAX * sin(frequency*(2*M_PI)*i/SAMPLERATE);
                // INT8_MAX en atres casos, etc   https://sites.uclouvain.be/SystInfo/usr/include/limits.h.html              
        }
}

/*
Tony Tavener 
Friday, 10 August 2012
OpenAL Soft -- demonstration of binaural 3D audio 
http://cranialburnout.blogspot.com/2012/08/openal-soft-demonstration-of-binaural.html
For the footsteps.raw, I did this:
Got the file from http://www.soundjay.com/footsteps/footsteps-4.wav
Then processed it using 'sox':
> sox footsteps-4.wav -b 16 footsteps.raw channels 1 rate 44100
*/

/* load a file into memory, returning the buffer and
 * setting bufsize to the size-in-bytes */
void* load( char *fname, long *bufsize ){
    FILE* fp = fopen( fname, "rb" );
    printf("A l'obrir %s, dona %p\n", fname, fp);
    
    fseek( fp, 0L, SEEK_END );
    long len = ftell( fp );
    rewind( fp );
    void *buf = malloc( len );
    printf("Reservant %ld bytes en %p\n", len, buf);
    fread( buf, 1, len, fp );
    fclose( fp );
    *bufsize = len;
    return buf;
}


int main(int argc, char *argv[])
{
    ALCdevice *elDispositiu;
    ALCcontext *elContext;
    //   const ALCchar	*szDefaultCaptureDevice;
    ALuint elBuffer;
    ALuint laFont;  
    ALenum format;
//     int canals, freqMostreig;
    ALenum error, g_bEAX;
    const ALchar *pDeviceList, *llista;
    ALbyte data[ TAMANY_BUFFER ];
    int i;

  
// NO pots preguntar ací encara    
//     printf ("La versio de OpenAL instalada es %s \n Las diferentes extensiones de OpenAL son: %s\n", alGetString(AL_VERSION), alGetString(AL_EXTENSIONS));
//     a versio de OpenAL instalada es (null) 
//  Las diferentes extensiones de OpenAL son: (null)

    //
    //
    
    // Initialization
    elDispositiu = alcOpenDevice(NULL); // select the "preferred device"
    if (elDispositiu) {
        elContext = alcCreateContext(elDispositiu,NULL);
        if (elContext != NULL) {
            alcMakeContextCurrent( elContext );
            error = alGetError(); 
            if (error) DisplayALError("Error en alcMakeContextCurrent:", error);
        }
        else {
        error = alGetError(); 
        if (error) DisplayALError("Error en alcCreateContext:", error);
       }
    }
    else {
     error = alGetError(); 
//      if (error) printf("Error en alcOpenDevice: %d\n", error);
     if (error) DisplayALError("Error en alcOpenDevice:", error);
    }
    
    if (elDispositiu != NULL) {
        elContext = alcCreateContext(elDispositiu,NULL); // create context
        if (elContext != NULL) {
            alcMakeContextCurrent( elContext ); // set active context
        }
    }


  //
  printf ("La versio de OpenAL instalada es %s \n Las diferentes extensiones de OpenAL son: %s\n", alGetString(AL_VERSION), alGetString(AL_EXTENSIONS));
  //
  printf("OpenAL Renderer is '%s'\n", alGetString(AL_RENDERER) );
  printf("OpenAL Version is '%s'\n", alGetString(AL_VERSION) );		    
  printf("OpenAL Vendor is '%s'\n", alGetString(AL_VENDOR) );
  //
    // Check for EAX 2.0 support
    g_bEAX = alIsExtensionPresent("EAX2.0");
//     error = alGetError(); 
//     if (error) DisplayALError("EAX2.0?", error);
//     else
//         printf("EAX2.0 presente\n");
//     printf ("alIsExtensionPresent(\"EAX2.0\") retorna %d.\n", g_bEAX );
    printf ("alIsExtensionPresent(\"EAX2.0\") retorna %s.\n", (g_bEAX == AL_TRUE? "sí": "no"));
  
    printf("OpenAL Extensions supported are :\n%s\n", alGetString(AL_EXTENSIONS) );
  llista = alGetString(AL_EXTENSIONS);
  i=1;
  if (llista)
    {
      printf("\nExtensions disponibles per separat:");
      printf("\n%2d - ", i);
      while (*llista)
        {
         if (*llista != ' ' ) printf("%c", *llista);
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


   //
   // Que sone algo, no?    
   //
      
   // Generate Buffers
   alGetError(); // clear error code
   alGenBuffers(1, &elBuffer);
   if ((error = alGetError()) != AL_NO_ERROR)    {
        DisplayALError("alGenBuffers :", error);
   }
   // Generar una senyal simple
   fill_buffer(data, TAMANY_BUFFER, 440 );
// 
//  /*   
// alBufferData
// Description
// This function fills a buffer with audio data. All the pre-defined formats are PCM data, but
// this function may be used by extensions to load other data types as well.
// void alBufferData(
// ALuint buffer,
// ALenum format,
// const ALvoid *data,
// ALsizei size,
// ALsizei freq
// );
// Parameters
// buffer
//  buffer name to be filled with data
// format
//  format type from among the following:
// AL_FORMAT_MONO8
// AL_FORMAT_MONO16
// AL_FORMAT_STEREO8
// AL_FORMAT_STEREO16
// data
//  pointer to the audio data
// size
//  the size of the audio data in bytes
// freq
//  the frequency of the audio data
//     
//     
//     8-bit PCM data is expressed as an unsigned value over the range 0 to 255, 128 being an
// audio output level of zero. 16-bit PCM data is expressed as a signed value over the
// range -32768 to 32767, 0 being an audio output level of zero. Stereo data is expressed
// in interleaved format, left channel first. Buffers containing more than one channel of data
// will be played without 3D spatialization.*/
   alBufferData( elBuffer, AL_FORMAT_MONO8, (const ALvoid*)data, TAMANY_BUFFER, SAMPLERATE );
// 

    // Generate Sources
    alGenSources(1,&laFont);
    if ((error = alGetError()) != AL_NO_ERROR)
    {
        DisplayALError("alGenSources 1 : ", error);        
    }
    // Attach buffer to source
    alSourcei(laFont, AL_BUFFER, elBuffer);
    if ((error = alGetError()) != AL_NO_ERROR)
    {
        DisplayALError("alSourcei AL_BUFFER : ", error);
    }
    
// NO:         printf("%f es %f?\n", 2*M_PI, M_2_PI); /6.283185 es 0.636620?
    alSourcePlay( laFont );
    // NBYTES / BYTERATE --> nSegons de sonit
    printf("Espera %f segons\n", (float)TAMANY_BUFFER /(float)(SAMPLERATE* BYTESPERSAMPLE * NCANALS) ); 
    sleep( round( (double)TAMANY_BUFFER /(double)(SAMPLERATE* BYTESPERSAMPLE * NCANALS) ) );
    printf("He esperat %d segons\n", 
        (int)round( (double)TAMANY_BUFFER /(double)(SAMPLERATE* BYTESPERSAMPLE * NCANALS) )  ); 
   alSourceStop( laFont );
//    alDeleteBuffers(1, elBuffer);
   alSourcei( laFont, AL_BUFFER, 0);

   // Cargar un fitxer RAW
//    alGenBuffers(1, &elBuffer);
   printf("Cargar un fitxer RAW: footsteps.raw\n");
   {
     long dataSize;
     const ALvoid* data2 = load( "footsteps.raw", &dataSize );
        /* for simplicity, assume raw file is signed-16b at 44.1kHz */
     printf("He llegit de %s %ld bytes\n", "footsteps.raw", dataSize );
     alBufferData( elBuffer, AL_FORMAT_MONO16, data2, dataSize, 44100 );
     free( (void*)data2 );
    
     alSourcei( laFont, AL_BUFFER, elBuffer);
     alSourcePlay( laFont );
    // NBYTES / BYTERATE --> nSegons de sonit
     printf("Espera %f segons\n", (float)dataSize /(float)(44100* 2 * 1) ); 
     sleep( round( (double)dataSize/(double)(44100 * 2 * 1) ) );    
   }
//   alSourceStop( laFont );
                 
    
// // Cargar un fitxer WAV    
// printf("Cargar un fitxer WAVE: footsteps.wav\n");
// https://stackoverflow.com/questions/36949957/loading-a-wav-file-for-openal
// 
// https://gamedev.net/forums/topic/573599-loading-wavs-for-openal/4660752/
//  Loading WAVs For OpenAL
// Engines and Middleware Programming
// Started by Vincent_M June 09, 2010 12:13 PM
// 3 comments, last by Gorax 11 years, 11 months ago
// 
// 
// Home
// Blogs
// Careers
// Forums
// News
// Portfolios
// Projects
// Tutorials
// New? Learn about game development
// Follow Us
// Chat in the GameDev.net Discord!
// Back to Engines and Middleware
// Loading WAVs For OpenAL
// Engines and Middleware Programming
// Started by Vincent_M June 09, 2010 12:13 PM
// 3 comments, last by Gorax 11 years, 11 months ago
// Advertisement
// Vincent_M
// Vincent_M
// Author
// 969
// June 09, 2010 12:13 PM
// I am trying to load WAV files into memory to use with OpenAL. I have done some research on the internet, and it looks like alut is used to load wavs, however, this is not an option I can use. I was able to load the WAV's riff header and both sub chunks with not problem. The only problem is getting the data read, and the proper format. I feed my data to OpenAL via alBufferData(), but I do not think the format I am feeding in (value is 1 for PCM in the WAV file) is correct because no data is played. Here is my loading code:
// 
// 
// bool LoadWAV(const char *filename, int *format, void *data, int *size, int *freq)
// {
// FILE *fp = NULL;
// RIFFHeader riff;
// WAVSubChunk1 chunk1;
// WAVSubChunk2 chunk2;
// 
// fp = fopen(filename, "rb");
// if(!fp)
// {
// printf("LoadWAV(%s): invalid file\n", filename);
// return false;
// }
// 
// // read the sub chunk data
// fread(&riff, sizeof(RIFFHeader), 1, fp);
// fread(&chunk1, sizeof(WAVSubChunk1), 1, fp);
// fread(&chunk2, sizeof(WAVSubChunk2), 1, fp);
// 
// riff.Print();
// chunk1.Print();
// chunk2.Print();
// data = malloc(chunk2.subChunkSize);
// fread(data, sizeof(chunk1.bitsPerSample / 8), chunk2.subChunkSize / (chunk1.bitsPerSample / 8), fp);
// 
// //set the data
// *format = chunk1.format;
// *size = chunk2.subChunkSize;
// *freq = chunk1.sampleRate;
// 
// return true;
// }
// 
// 
// alBufferData(buffer[0],format,data,size,freq); // parameters are globals
// 
// 
// 
// 
// Btw, aren't there code tags I can use for this? I could not find them.
// Vincent_M
// Vincent_M
// Author
// 969
// June 10, 2010 01:22 AM
// Alright, after a nap, I was able to figure out two things:
// 1) I did not have OpenAL initialized...
// 2) Allocating and writing data to void*-typed pointers caused errors when uploading to OpenAL, so I just used unsigned char arrays instead. Here is working code in case anyone needs to manually load a WAV:
// 
// 
// struct RIFFHeader
// {
// char riff[4];
// long size;
// char wave[4];
// 
// void Print()
// {
// printf("RIFFHeader::Print()\n");
// printf("riff: %c%c%c%c\n", riff[0], riff[1], riff[2], riff[3]);
// printf("size: %i\n", size);
// printf("wave: %c%c%c%c\n", wave[0], wave[1], wave[2], wave[3]);
// printf("\n");
// }
// };
// 
// 
// struct WAVSubChunk1
// {
// char subChunkId[4];
// long subChunkSize;
// short format;
// short numChannels;
// long sampleRate;
// long byteRate;
// short blockAlign;
// short bitsPerSample;
// 
// void Print()
// {
// printf("WAVSubChunk1::Print()\n");
// printf("subChunkId: %c%c%c%c\n", subChunkId[0], subChunkId[1], subChunkId[2], subChunkId[3]);
// printf("subChunkSize: %i\n", subChunkSize);
// printf("format: %i\n", format);
// printf("numChannels: %i\n", numChannels);
// printf("sampleRate: %i\n", sampleRate);
// printf("byteRate: %i\n", byteRate);
// printf("blockAlgin: %i\n", blockAlign);
// printf("bitsPerSample: %i\n", bitsPerSample);
// printf("\n");
// }
// };
// 
// 
// struct WAVSubChunk2
// {
// char subChunkId[4];
// long subChunkSize;
// 
// void Print()
// {
// printf("WAVSubChunk2::Print()\n");
// printf("subChunkId: %c%c%c%c\n", subChunkId[0], subChunkId[1], subChunkId[2], subChunkId[3]);
// printf("subChunkSize: %i\n", subChunkSize);
// printf("\n");
// }
// };
// 
// 
// unsigned char *LoadWAV(const char *filename, int *format, int *size, int *freq)
// {
// FILE *fp = NULL;
// unsigned char *data = NULL;
// RIFFHeader riff;
// WAVSubChunk1 chunk1;
// WAVSubChunk2 chunk2;
// 
// fp = fopen(filename, "rb");
// if(!fp)
// {
// printf("LoadWAV(%s): invalid file\n", filename);
// return NULL;
// }
// 
// // read the sub chunk data
// fread(&riff, sizeof(RIFFHeader), 1, fp);
// fread(&chunk1, sizeof(WAVSubChunk1), 1, fp);
// fread(&chunk2, sizeof(WAVSubChunk2), 1, fp);
// 
// // print the sound data
// riff.Print();
// chunk1.Print();
// chunk2.Print();
// 
// // check chunk identifiers
// if(riff.riff[0] != 'R' || riff.riff[1] != 'I' || riff.riff[2] != 'F' || riff.riff[3] != 'F')
// {
// printf("LoadWAV(%s): invalid 'RIFF' header\n", filename);
// return NULL;
// }
// if(riff.wave[0] != 'W' || riff.wave[1] != 'A' || riff.wave[2] != 'V' || riff.wave[3] != 'E')
// {
// printf("LoadWAV(%s): invalid 'WAVE' header\n", filename);
// return NULL;
// }
// if(chunk1.subChunkId[0] != 'f' || chunk1.subChunkId[1] != 'm' || chunk1.subChunkId[2] != 't' || chunk1.subChunkId[3] != ' ')
// {
// printf("LoadWAV(%s): invalid 'fmt ' header\n", filename);
// return NULL;
// }
// if(chunk2.subChunkId[0] != 'd' || chunk2.subChunkId[1] != 'a' || chunk2.subChunkId[2] != 't' || chunk2.subChunkId[3] != 'a')
// {
// printf("LoadWAV(%s): invalid 'data' header\n", filename);
// return NULL;
// }
// 
// data = new unsigned char[chunk2.subChunkSize];
// fread(data, 1, chunk2.subChunkSize, fp);
// 
// //set the data
// if(chunk1.numChannels == 1)
// {
// if(chunk1.bitsPerSample == 8)
// *format = AL_FORMAT_MONO8;
// if(chunk1.bitsPerSample == 16)
// *format = AL_FORMAT_MONO16;
// }
// if(chunk1.numChannels == 2)
// {
// if(chunk1.bitsPerSample == 8)
// *format = AL_FORMAT_STEREO8;
// if(chunk1.bitsPerSample == 16)
// *format = AL_FORMAT_STEREO16;
// }
// *size = chunk2.subChunkSize;
// *freq = chunk1.sampleRate;
// return data;
// }
// 
// 
// void FreeWAV(unsigned char *data)
// {
// if(data)
// {
// delete [] data;
// data = NULL;
// }
// }
// 
// 
// ...
// // here is how to upload to an OpenAL buffer:
// unsigned char *byteArray = NULL;
// byteArray = LoadWAV("Sounds/reactor.wav", &format, &size, &freq);
// alBufferData(buffer[0],format,(void*)byteArray,size,freq);
// FreeWAV(byteArray);

    
    // LLiberar recursos
    elContext = alcGetCurrentContext();
    elDispositiu = alcGetContextsDevice( elContext);
    alcMakeContextCurrent(NULL);
    alcDestroyContext( elContext );
    alcCloseDevice( elDispositiu );
    
    
       
    return EXIT_SUCCESS;
}
