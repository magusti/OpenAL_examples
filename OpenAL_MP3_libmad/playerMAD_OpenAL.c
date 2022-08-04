
 /*
Implementing MP3 player in C
https://lauri.xn--vsandi-pxa.com/2013/12/implementing-mp3-player.en.html

gcc -o player player.c -lpulse -lpulse-simple -lmad -g
sudo apt-get install libpulse-dev libmad0-dev libpulse0 libmad

Cal canviar l'eixia a Pulse Audio per OpenAL

magusti@verdet:~/docencia/2k20-2k21/docenciaEnXarxa/articles/mp3_en_computador_o_en_3DS$ gcc playerMAD_OpenAL.c -o playerMAD_OpenAL -lmad `pkg-config openal --cflags --libs` && playerMAD_OpenAL drH.mp3

// En ALUT
$ gcc playerMAD_OpenAL.c -o playerMAD_OpenAL -lmad -lalut `pkg-config openal --cflags --libs` && playerMAD_OpenAL drH.mp3


Les funcions d'inicialitzar i tancar OpenAL són de openalSoft? Sí!!!!!! Compara-les en les que tens en 
/home/magusti/docencia/2k17-2k18/docenciaEnRed/articlesDocents/OpenAL_SDL_MP3/alplay/openal-soft-master 

https://programmerclick.com/article/28571350752/
MP3: un múltiplo de 576 (dos canales 1152 * 1000/44100 = 26.122ms)

$file drH.mp3
.../drH.mp3: Audio file with ID3 version 2.4.0, contains:MPEG ADTS, layer III, v1, 128 kbps, 44.1 kHz, Stereo


https://www.underbit.com/products/mad/
MAD: MPEG Audio Decoder

http://m.baert.free.fr/contrib/docs/libmad/doxy/html/index.html
libmad - MPEG audio decoder library
0.15.2b
Author:
Except where otherwise noted, all code was authored by: Robert Leslie <rob at underbit com>
Copyright (C) 2000-2008 Underbit Technologies, Inc.
See the file CREDITS for a list of contributors.
Introduction
MAD (libmad) is a high-quality MPEG audio decoder. It currently supports MPEG-1 and the MPEG-2 extension to Lower Sampling Frequencies, as well as the so-called MPEG 2.5 format. All three audio layers (Layer I, Layer II, and Layer III a.k.a. MP3) are fully implemented.
MAD does not yet support MPEG-2 multichannel audio (although it should be backward compatible with such streams) nor does it currently support AAC.

MAD has the following special features:

24-bit PCM output
100% fixed-point (integer) computation
completely new implementation based on the ISO/IEC standards
distributed under the terms of the GNU General Public License (GPL)
Because MAD provides full 24-bit PCM output, applications using MAD are able to produce high quality audio. Even when the output device supports only 16-bit PCM, applications can use the extra resolution to increase the audible dynamic range through the use of dithering or noise shaping.

Because MAD uses integer computation rather than floating point, it is well suited for architectures without a floating point unit. All calculations are performed with a 32-bit fixed-point integer representation.

Because MAD is a new implementation of the ISO/IEC standards, it is unencumbered by the errors of other implementations. MAD is NOT a derivation of the ISO reference source or any other code. Considerable effort has been expended to ensure a correct implementation, even in cases where the standards are ambiguous or misleading.

Because MAD is distributed under the terms of the GPL, its redistribution is not generally restricted, so long as the terms of the GPL are followed. This means MAD can be incorporated into other software as long as that software is also distributed under the GPL. (Should this be undesirable, alternate arrangements may be possible by contacting Underbit.)


MAD: MPEG Audio Decoder
https://www.underbit.com/products/mad/
MAD is a high-quality MPEG audio decoder. It currently ...


Example de High Level API <http://m.baert.free.fr/contrib/docs/libmad/doxy/html/high-level.html>

minimad.c
This is perhaps the simplest example use of the MAD high-level API.
Standard input is mapped into memory via mmap(), then the high-level API is invoked with three callbacks: input, output, and error. The output callback converts MAD's high-resolution PCM samples to 16 bits, then writes them to standard output in little-endian, stereo-interleaved format.

http://m.baert.free.fr/contrib/docs/libmad/doxy/html/minimad_8c-source.html
https://github.com/njh/madjack/blob/master/src/minimad.c


MP3
https://cast.readme.io/docs/mp3
Té figures a propòsit del interior d'un MP3


https://stackoverflow.com/questions/39803572/libmad-playback-too-fast-if-read-in-chunks

Programming of MP3 music player based on Libmad library under Linux
https://programmersought.com/article/20221850163/
Ho tira directament al dsp

*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <unistd.h> // usleep

//#include <fcntl.h>
#include <stdint.h> // uint32_t
// #include <sys/types.h>



#include <mad.h>
//#include <pulse/simple.h>
//#include <pulse/error.h>
#include <assert.h>

// #include "AL/al.h"
// #include "AL/alc.h"
// #include "AL/alext.h"
#include <AL/alut.h>

//pa_simple *device = NULL;
// int ret = 1;
// int error;
struct mad_stream mad_stream;
struct mad_frame mad_frame;
struct mad_synth mad_synth;

// http://m.baert.free.fr/contrib/docs/libmad/doxy/html/decoder_8h-source.html
#define LAYER_A_STR(v) (v==3? "Layer III" : (v==2? "Layer II" : "Layer I"))
#define MODE_A_STR(v) (v==0? "SINGLE CHANNEL" : (v==2? "DUAL CHANNEL" : (v==3? "JOINT STEREO" : "STEREO")))

//static char stream[1152*4];
#define TAMANY_STREAM 1152*4*1024
// #define TAMANY_STREAM 1024*1024*4
static unsigned char stream[TAMANY_STREAM];
void output(struct mad_header const *header, struct mad_pcm *pcm);

// // Prototips per a OpenAL sense ALUT
// int InitAL(char ***argv, int *argc);
// void CloseAL(void);

static ALuint madCreateBufferFromFile( char *filename ) {
  int resultat;
  // Com averigües que és Signed 16bits, 2 canals, 44100
  ALenum err, format;
  ALuint bufferOpenAL;
  uint32_t slen; //Uint32 slen;
  int nMostres = 0;
  
  
    // Filename pointer
    //char *filename = argv[1];

    printf("Obrint %s\n", filename);
    // File pointer
    FILE *fp = fopen(filename, "r");
    if ( !fp ){
        printf("No put obrir el fitxer %s\n", filename);
        return( (ALuint)0 );
    }
    int fd = fileno(fp);

    // Fetch file size, etc
    struct stat metadata;
    if (fstat(fd, &metadata) >= 0) {
        printf("File size %d bytes\n", (int)metadata.st_size);
    } else {
        printf("Failed to stat %s\n", filename);
        fclose(fp);
        return( (ALuint)0 ); // 254;
    }

    // Exemple de Low Level API libmad <http://m.baert.free.fr/contrib/docs/libmad/doxy/html/low-level.html>
    // Initialize MAD library
    mad_stream_init(&mad_stream);
    mad_synth_init(&mad_synth);
    mad_frame_init(&mad_frame);

//     //Propietats MP3? Ací encara no!
//     //http://m.baert.free.fr/contrib/docs/libmad/doxy/html/frame_8h-source.html
    
    printf("Mapejant\n");
    // Let kernel do all the dirty job of buffering etc, map file contents to memory
    char *input_stream = mmap(0, metadata.st_size, PROT_READ, MAP_SHARED, fd, 0);

    printf("Creant stream buffer\n");
    // Copy pointer and length to mad_stream struct
    mad_stream_buffer(&mad_stream, input_stream, metadata.st_size);
    printf("mad_stream_buffer len. %u vs %lu \n", mad_stream.md_len, metadata.st_size);

    //Com puc traure les propietats globals del MP3 de la capçalera? N'hi ha de tot el fitxer?
//      while ( mad_frame_decode(&mad_frame, &mad_stream) )  {};
//      printf("mad_stream_buffer len. %u vs %d; modo %d duració %ld\n", 
//             mad_stream.md_len, metadata.st_size,
//             mad_frame.header.mode,
//             mad_frame.header.duration);
    
    // Decode frame and synthesize loop
    /* Provar a vore si aixina es pot while ( mad_frame_decode(&mad_frame, &mad_stream ) {
     *
        printf(".");
        // Synthesize PCM data of frame
        mad_synth_frame(&mad_synth, &mad_frame);
        output(&mad_frame.header, &mad_synth.pcm);
    }
    */
    
//     //Propietats MP3? Ací no eixirà res!
//     //Decodes a single frame from a bitstream.
//     //if (mad_frame_decode(&mad_frame, &mad_stream) > -1) {
//     mad_frame_decode(&mad_frame, &mad_stream);
//     printf("frame_decode:: Layer %s (%d) Modo/Canals %s (%d)  bitrate %lu Canals %d, frecuencia %u duració %ld (segons)\n", 
//            LAYER_A_STR( mad_frame.header.layer ), mad_frame.header.layer,
//            MODE_A_STR( mad_frame.header.mode ), mad_frame.header.mode ,  
//            mad_frame.header.bitrate/1000,  // per que eixca en kbps
//            MAD_NCHANNELS(&mad_frame.header), 
//            mad_frame.header.samplerate / 1000,  // per que siguen kHz
//            mad_frame.header.duration.seconds);
//     //}
        
    while (1) {
        //printf("Decodificant\r");
        // Decode frame from the stream
        if (mad_frame_decode(&mad_frame, &mad_stream)) {
            if (MAD_RECOVERABLE(mad_stream.error)) {
                printf("-"); fflush(stdout);
                continue;
            } else if (mad_stream.error == MAD_ERROR_BUFLEN) {
                printf("\\"); fflush(stdout);
                break; //continue;
            } else {
                printf("|"); fflush(stdout);
                break;
            }
        }        
        printf("."); fflush(stdout);
//         printf("frame_decode:: Layer %d Modo/Canals %d  bitrate %lu Canals %d, frecuencia %u duració %ld (segons)\n", 
//            mad_frame.header.layer,
//            mad_frame.header.mode, 
//            mad_frame.header.bitrate,
//            MAD_NCHANNELS(&mad_frame.header), 
//            mad_frame.header.samplerate,
//            mad_frame.header.duration.seconds);

        // Synthesize PCM data of frame
        mad_synth_frame(&mad_synth, &mad_frame);
        output(&mad_frame.header, &mad_synth.pcm);
//         alBufferData(buffer+nMostres, format, stream, 11542*4, 44100); //sample->actual.rate);
           
    }
     output(&mad_frame.header, &mad_synth.pcm);
    printf("\ndespres del while::\n%s: Layer %s (%d) Modo/Canals %s (%d)  bitrate %lu kbps Canals %d, frecuencia %3.1f kHz duració %ld (segons)\n", 
           filename, LAYER_A_STR( mad_frame.header.layer ), mad_frame.header.layer,
           MODE_A_STR( mad_frame.header.mode ), mad_frame.header.mode ,  
           mad_frame.header.bitrate/1000,  // per que eixca en kbps
           MAD_NCHANNELS(&mad_frame.header), 
           (float)(mad_frame.header.samplerate) / 1000.0,  // per que siguen kHz
           mad_frame.header.duration.seconds);
    nMostres = 0;
    bufferOpenAL = 0;
    printf("antes alBufferData\n");
    alGenBuffers(1, &bufferOpenAL);
    if (mad_synth.pcm.channels == 1)
       format = AL_FORMAT_MONO16;
    else
       format = AL_FORMAT_STEREO16;
           
//     alBufferData(bufferOpenAL, format, stream, TAMANY_STREAM, mad_synth.pcm.samplerate );    
    
    printf("\n");
    printf("Ha generat un PCM de samplerate %u canals %u mostresXcanal %u\n",
           mad_synth.pcm.samplerate, mad_synth.pcm.channels, mad_synth.pcm.length );
    printf("En stream n'hi han %ld bytes == %u mostresXcanalX4 %d \n", 
            sizeof( stream ), mad_synth.pcm.length*4, 11542*4 );

//      int i=0;
//     while (i < sizeof( stream ) ) printf("%d ", stream+i);
// printf("Ficant bufferOpenALvalors en stream ... per si es que no n'hi ha res\n");
//     while (i < sizeof( stream ) ) stream[i++] = 0xE4; 
   
    alBufferData(bufferOpenAL, format, stream, sizeof(stream), mad_synth.pcm.samplerate ); 
    //11542*4=46168 és el tamanay de cada "frame" no frec. -->  44100 o samplerate); 
    //free( stream );  No pq ho tinc asignat estàtic com a global
    // Close
    fclose(fp);

//     printf("Lliberant recursos MAD\n");
//     // Free MAD structs
//     mad_synth_finish(&mad_synth);
//     mad_frame_finish(&mad_frame);
//     mad_stream_finish(&mad_stream);

    
    // Buffer the audio data into a new buffer object, then free the data and
    // close the file. 
//     buffer = 0;
//     alGenBuffers(1, &buffer);
//     printf("antes alBufferData\n");
//     alBufferData(buffer, format, stream, 11542*4, 44100); //sample->actual.rate);
    //ad_synth.pcm.length * 4, mad_synth.pcm.samplerate );
    //(int)metadata.st_size, mad_synth.pcm.samplerate );
    printf("després alBufferData\n");

    printf("Lliberant recursos MAD\n");
    // Free MAD structs
    mad_synth_finish(&mad_synth);
    mad_frame_finish(&mad_frame);
    mad_stream_finish(&mad_stream);

    // Check if an error occured, and clean up if so. 
    err = alGetError();
//     if(err != AL_NO_ERROR) {
//         fprintf(stderr, "OpenAL Error: %s\n", alGetString(err));
//         if(buffer && alIsBuffer(buffer))
//             alDeleteBuffers(1, &buffer);
//         return 0;
//     }

    return bufferOpenAL;
} // madCreateBufferFromFile


int main(int argc, char **argv) {
    ALuint source, buffer;
    ALfloat offset;
    ALenum state;
   
    
    // Parse command-line arguments
    if (argc != 2) {
        fprintf(stderr, "Usage: %s [filename.mp3]", argv[0]);
        return 255;
    }

/*
    // Set up PulseAudio 16-bit 44.1kHz stereo output
    static const pa_sample_spec ss = { .format = PA_SAMPLE_S16LE, .rate = 44100, .channels = 2 };
    if (!(device = pa_simple_new(NULL, "MP3 player", PA_STREAM_PLAYBACK, NULL, "playback", &ss, NULL, NULL, &error))) {
        printf("pa_simple_new() failed\n");
        return 255;
    }
*/   


    // Initialize OpenAL
//     argv++; argc--;
//     if(InitAL(&argv, &argc) != 0)
//         return 1;
    alutInit (&argc, argv); 
 
    printf("Vaig a madCreateBufferFromFile en libMAD versió %s\n", mad_version);
//     printf("Vaig a madCreateBufferFromFile en libMAD versió %d.%d.%d\n", 
//             MAD_VERSION_MAJOR, MAD_VERSION_MINOR, MAD_VERSION_PATCH );

    // Load the sound into a buffer. 
    printf("madCreateBufferFromFile: %s\n", argv[1]);
    buffer = madCreateBufferFromFile( argv[1] ); // Sí, que ha fet argv++!!! i argc--
// Aseguremos que la part de reproducció d'OpenAL funciona inicialitzant el buffer a algo que saben que va
//     printf("alutCreateBufferFromFile --> %s\n", "drHouse.wav");
//     buffer = alutCreateBufferHelloWorld(); //ile( "drHouse.wav" );

    
    if (alGetError() != AL_NO_ERROR )
        printf("Falla al crearel buffer <-- madCreateBufferFromFile \n");
    if( !buffer )     {
        printf("No buffer?\n");
        alutExit(); //CloseAL();
        return( 1 );
    }
    printf("He tornat de madCreateBufferFromFile\n");

    // Create the source to play the sound with
    source = 0;
    alGenSources(1, &source);
    alSourcei(source, AL_BUFFER, buffer);
    //assert(alGetError()==AL_NO_ERROR && "Failed to setup sound source");
    if (alGetError() != AL_NO_ERROR )
        printf("Failed to setup sound source\n");
printf("Asignada a la font\n");

    // Play the sound until it finishes
    alSourcePlay(source);
    alSourcef(source, AL_GAIN, 10.0);
    do {
        usleep(1000); //0000); //al_nssleep(10000000);
        alGetSourcei(source, AL_SOURCE_STATE, &state);

        /* Get the source offset. */
        alGetSourcef(source, AL_SEC_OFFSET, &offset);
        printf("\rOffset: %f  ", offset);
        fflush(stdout);
    } while(alGetError() == AL_NO_ERROR && state == AL_PLAYING);
    printf("\n");

    /* All done. Delete resources, and close down SDL_sound and OpenAL. */
    alDeleteSources(1, &source);
    alDeleteBuffers(1, &buffer); 
    
//     CloseAL();      
    alutExit ();

    printf("Acabe ja \n");
    return EXIT_SUCCESS;
}

// Some helper functions, to be cleaned up in the future
int scale(mad_fixed_t sample) {
     /* round */
     sample += (1L << (MAD_F_FRACBITS - 16));
     /* clip */
     if (sample >= MAD_F_ONE)
         sample = MAD_F_ONE - 1;
     else if (sample < -MAD_F_ONE)
         sample = -MAD_F_ONE;
     /* quantize */
     return sample >> (MAD_F_FRACBITS + 1 - 16);
}

static long int base = 0;
static long int cuantesFrames = 0;
void output(struct mad_header const *header, struct mad_pcm *pcm) {
// static long int base = 0;
    register int nsamples = pcm->length;
//     printf("nsamples %d\n", nsamples);
    mad_fixed_t const *left_ch = pcm->samples[0], 
                      *right_ch = pcm->samples[1];
//    static char stream[1152*4];
    /* 1152 because that's what mad has as a max; *4 because
  there are 4 distinct bytes per sample (in 2 channel case) */
static char spinner[] = "|/-\\";
  

    if (pcm->channels == 2) {
        while (nsamples--) {
            signed int sample;
            // litle-endian
//             printf("base + ((pcm->length-nsamples)*4 % d\n", base + ((pcm->length-nsamples)*4));
            sample = scale(*left_ch++);
            stream[base + ((pcm->length-nsamples)*4)] = ((sample >> 0) & 0xff);
            stream[base + ((pcm->length-nsamples)*4 +1)] = ((sample >> 8) & 0xff);
            sample = scale(*right_ch++);
            stream[base + ((pcm->length-nsamples)*4+2)] = ((sample >> 0) & 0xff);
            stream[base + ((pcm->length-nsamples)*4 +3)] = ((sample >> 8) & 0xff);
//Comprovat que no es big-endiant canviant l'ordre del desplaçaments i que no va
            //        printf("%13d %c\t\t stream[%d] = %d %d \r", nsamples, spinner[nsamples % 4], 
//               (pcm->length-nsamples)*4, ((sample >> 8) & 0xff), ((sample >> 0) & 0xff)); 
//        fflush(stdout);   
            
        }
        
//         if (pa_simple_write(device, stream, (size_t)1152*4, &error) < 0) {
//             fprintf(stderr, "pa_simple_write() failed: %s\n", pa_strerror(error));
//             return;
//         }
    } else {
        printf("Monofònics implementat\n");
        // Fer el mateix que en stereo, pero copiant el mateix als dos canals
       while (nsamples--) {
            signed int sample;
            sample = scale(*left_ch++);
//             stream[(pcm->length-nsamples)*4 ] = ((sample >> 0) & 0xff);
//             stream[(pcm->length-nsamples)*4 +1] = ((sample >> 8) & 0xff);
            stream[base + ((pcm->length-nsamples)*4) ] = ((sample >> 0) & 0xff);
            stream[base + ((pcm->length-nsamples)*4 +1)] = ((sample >> 8) & 0xff);
//             sample = scale(*right_ch++);
//             stream[(pcm->length-nsamples)*4+2 ] = ((sample >> 0) & 0xff);
//             stream[(pcm->length-nsamples)*4 +3] = ((sample >> 8) & 0xff);
        }
    }

    printf("Frame %ld en @ %ld a on guardar %ld\n", cuantesFrames, base, pcm->length);

    cuantesFrames++;
//     base += pcm->length; //--> apresa
//     base += 1152*4; --> core
//      base += 1152; 
     base += 1024*4; //--> casi
     //base += 1061* pcm->channels *2; //--> A vore si ho ajuste per vore quin pot ser el valor: si es 1062 ja va "core"
//     https://programmerclick.com/article/89561072319/ ==> massa
//     base += pcm->length* pcm->channels *2; //--> casi
//     printf("Anyadint %ld bytes %d x %d x 2.\n", base, pcm->length, pcm->channels );      
}






// /* InitAL opens a device and sets up a context using default attributes, making
//  * the program ready to call OpenAL functions. */
// int InitAL(char ***argv, int *argc)
// {
//     const ALCchar *name;
//     ALCdevice *device;
//     ALCcontext *ctx;
// 
//     /* Open and initialize a device */
//     device = NULL;
//     if(argc && argv && *argc > 1 && strcmp((*argv)[0], "-device") == 0)
//     {
//         device = alcOpenDevice((*argv)[1]);
//         if(!device)
//             fprintf(stderr, "Failed to open \"%s\", trying default\n", (*argv)[1]);
//         (*argv) += 2;
//         (*argc) -= 2;
//     }
//     if(!device)
//         device = alcOpenDevice(NULL);
//     if(!device)
//     {
//         fprintf(stderr, "Could not open a device!\n");
//         return 1;
//     }
// 
//     ctx = alcCreateContext(device, NULL);
//     if(ctx == NULL || alcMakeContextCurrent(ctx) == ALC_FALSE)
//     {
//         if(ctx != NULL)
//             alcDestroyContext(ctx);
//         alcCloseDevice(device);
//         fprintf(stderr, "Could not set a context!\n");
//         return 1;
//     }
// 
//     name = NULL;
//     if(alcIsExtensionPresent(device, "ALC_ENUMERATE_ALL_EXT"))
//         name = alcGetString(device, ALC_ALL_DEVICES_SPECIFIER);
//     if(!name || alcGetError(device) != AL_NO_ERROR)
//         name = alcGetString(device, ALC_DEVICE_SPECIFIER);
//     printf("Opened \"%s\"\n", name);
// 
//     return 0;
// }
// 
// /* CloseAL closes the device belonging to the current context, and destroys the
//  * context. */
// void CloseAL(void)
// {
//     ALCdevice *device;
//     ALCcontext *ctx;
// 
//     ctx = alcGetCurrentContext();
//     if(ctx == NULL)
//         return;
// 
//     device = alcGetContextsDevice(ctx);
// 
//     alcMakeContextCurrent(NULL);
//     alcDestroyContext(ctx);
//     alcCloseDevice(device);
// }
