/*
 * 
 * Introduint idees de 
 * magusti@verdet:~/docencia/2k21-2k22/docenciaEnXarxa/articles/pintarElEspectreDeFrequencies$ kate openal-soft_alstream_2011.c
 * 
 * Ací, "openal_stream_wave.c", estic revisant el còdic de la FFT i he aillat ací l'exemple que estic construïnt de reproducció en streaming de fitxers WAVE mitjançant libsndfile
 * 
 * Fonts lLlegir un wav a trocets
 *** http://www.mega-nerd.com/libsndfile/
 *** https://github.com/libsndfile/libsndfile/
 **** https://github.com/kcat/openal-soft/blob/master/examples/alstream.c
 *  Ensenya que lliges en sf_readf_short en #mostres i en alBufferData indiques en #bytes

 * 
 * 
 * 
 * gcc openal_stream_wave.c -o openal_stream_wave -lalut `pkg-config openal --cflags --libs` -lsndfile && openal_stream_wave escala.wav
 * 
 * drHouse.wav: RIFF (little-endian) data, WAVE audio, Microsoft PCM, 16 bit, stereo 44100 Hz
 * Duració: 00:28
 * 1278542 mostres
 * escala.wav:  RIFF (little-endian) data, WAVE audio, Microsoft PCM, 16 bit, mono 44100 Hz
 * Duració: 1:45
 * 4672512 mostres
 *  ME DONA PROBLEMES INCLUS CARREGANT-LO en alutCreateBufferFromFile ... A voltes ...
 *  
 * KDE-Im-Phone-Ring.wav: RIFF (little-endian) data, WAVE audio, Microsoft PCM, 16 bit, stereo 48000 Hz
 * oldphone-mono.wav:     RIFF (little-endian) data, WAVE audio, Microsoft PCM, 16 bit, mono 44100 Hz
 */
#include <stdio.h>
#include <stdlib.h> //EXIT_SUCCESS
#include <math.h>
//#include <AL/al.h>
//#include <AL/alc.h>  // Ja els porten darrere en alut.h
#include <AL/alut.h>

#include <sndfile.h>
#include <string.h> //memset

/* Include this header file to use functions from libsndfile. */
#include	<sndfile.h>


/*	This will be the length of the buffer used to hold.frames while
 * *	we process them.
 */
#define		BUFFER_LEN	2*1024
// Baixar d'este a 1*1024 me dona problemes en KDE-Im-Phone-Ring
//Encara que atres números serveixen, a l'exemple d'ALURE gasta 8192
/* Define the number of buffers and buffer size (in milliseconds) to use. 4
 * buffers with 8192 samples each gives a nice per-chunk size, and lets the
 * queue last for almost one second at 44.1khz. */
//#define NUM_BUFFERS 4
//#define BUFFER_SAMPLES 8192

//512
// 1024*1024
//48000
//1024

/* libsndfile can handle more than 6 channels but we'll restrict it to 1 (artxius monofònics). */
#define		MAX_CHANNELS	2


/* This is a buffer of double precision floating point values --> byte? short! per la forma de treball de la crida de libsndfile
 * * which will hold our data while we process it.
 */
//	static double data [BUFFER_LEN] ;
//static int data [BUFFER_LEN] ;
//static short data[BUFFER_LEN];
//ALbyte data[BUFFER_LEN];
short *data; 

/* A SNDFILE is very much like a FILE in the Standard C library. The
 * * sf_open function return an SNDFILE* pointer when they sucessfully
 ** open the specified file.
 */
SNDFILE	*infile; //, *outfile ;

/* A pointer to an SF_INFO struct is passed to sf_open.
 * * On read, the library fills this struct with information about the file.
 ** On write, the struct must be filled in before calling sf_open.
 */
SF_INFO		sfinfo ;
int			readcount ;
char	infilename[128] = "escala.wav" ;


int main(int argc, char *argv[])
{
    ALenum error;
    ALint buffersLibres;
    ALuint bufferID;
    sf_count_t samples = 0, mostresLLegides = 0, numTotalMostres = 0;
    ALuint alsource;
    ALenum formatoAudio; //mono o stereo (1 o 2 canales)
    int numCanales;
    int bitsMuestra;
    int frecuenciaMuestreo;
    size_t frame_size;
    ALsizei mostresEnBytes;
    ALint queued;
    
    #define NUM_BUFFERS 3
    ALuint buffers[NUM_BUFFERS];
    
    char barraActivitat[4] = {'|', '/', '-', '\\'};
    
    // The SF_INFO struct must be initialized before using it.
    memset (&sfinfo, 0, sizeof (sfinfo)) ;
    
    if( argc > 1) {
        //         printf("argv[1] = %s\n", argv[1]);
        strcpy(infilename, argv[1]);
        //         printf("infilename = %s\n", infilename);
    }
    
    if (!alutInit(&argc, argv))
    {
        printf("Fallo al iniciar OpenAL.\n");
        return 0;
    }
    
    //   printf("\nPrimera partttttttttttttttttttttttttttttttttt?\n");
    // 
    //   {
    //     ALuint bufferID2;            // The OpenAL sound buffer ID
    //     ALuint sourceID2;            // The OpenAL sound source
    // //     alutInit (&argc, argv);
    //     //bufferID = alutCreateBufferHelloWorld ();
    // //       bufferID2 = alutCreateBufferFromFile( "escala.wav" ); // Se talla!!!
    //     bufferID2 = alutCreateBufferFromFile( "KDE-Im-Phone-Ring.wav");
    // //     bufferID2 = alutCreateBufferFromFile( "oldphone-mono.wav");
    //     alGenSources (1, &sourceID2);
    //     alSourcei (sourceID2, AL_BUFFER, bufferID2);
    //     alSourcePlay (sourceID2);
    //     alutSleep (3);
    //     alDeleteSources( 1, &sourceID2 );
    //     alDeleteBuffers( 1, &bufferID2 );
    // //     alutExit ();
    //   }
    
    printf ("La versio de OpenAL instalada es %s \n\n Las diferentes extensiones de OpenAL son: %s \n\n\n", alGetString(AL_VERSION), alGetString(AL_EXTENSIONS));
    printf ("La versio de ALUT instalada es %d.%d \n", alutGetMajorVersion(), alutGetMinorVersion());
    printf ("La versio de libsndfile instalada es %s \n", sf_version_string());
    
    //Generar buffers y fuentes de openAL (1 fuente)
    alGenBuffers(NUM_BUFFERS, buffers);
    if((error=alGetError()) != AL_NO_ERROR)
        printf("Error generando buffers\n");
    alGenSources(1, &alsource);
    if((error=alGetError()) != AL_NO_ERROR)
        printf("Error generando la fuente: %s\n", alutGetErrorString(error));
    
    
    /* Here's where we open the input file. We pass sf_open the file name and
     * * a pointer to an SF_INFO struct.
     ** On successful open, sf_open returns a SNDFILE* pointer which is used
     ** for all subsequent operations on that file.
     ** If an error occurs during sf_open, the function returns a NULL pointer.
     **
     ** If you are trying to open a raw headerless file you will need to set the
     ** format and channels fields of sfinfo before calling sf_open(). For
     ** instance to open a raw 16 bit stereo PCM file you would need the following
     ** two lines:
     **
     **		sfinfo.format   = SF_FORMAT_RAW | SF_FORMAT_PCM_16 ;
     **		sfinfo.channels = 2 ;
     */
    if (!(infile = sf_open(infilename, SFM_READ, &sfinfo)) )
    {	/* Open failed so print an error message. */
        printf ("Not able to open input file %s.\n", infilename) ;
        /* Print the error message from libsndfile. */
        puts (sf_strerror (NULL)) ;
        return 1 ;
    } ;
    
    //printf("sfinfo.channels > MAX_CHANNELS? %d > %d?\n", sfinfo.channels,  MAX_CHANNELS);   
    if (sfinfo.channels > MAX_CHANNELS)
    {	
        printf ("%s té %d canals? No anem a processar arxius en més de %d canals\n", 
                infilename, sfinfo.channels, MAX_CHANNELS) ;
                sf_close (infile) ;
                return 1 ;
    }
    
    
    printf("Format %0xd: PCM_S8 (%0xd)?: %0xd vs PCM_S16 (%0xd)?: %0xd \n", 
           sfinfo.format,
           SF_FORMAT_PCM_S8,
           (sfinfo.format & SF_FORMAT_PCM_S8),
           SF_FORMAT_PCM_16,
           (sfinfo.format & SF_FORMAT_PCM_16) );
    
    //numTotalMostres = (sfinfo.frames * sfinfo.channels) * ((sfinfo.format & SF_FORMAT_PCM_S8) > 0? 1 : 2);
    numTotalMostres = sfinfo.frames;
    //  printf("numTotalMostres %ld, sfinfo.frames %ld\n", numTotalMostres, sfinfo.frames);
    
    printf("%s: canals %d, freqMostreig %d, format %0xd [%s, %s], total (%ld mostres %ld bytes) i temps (%04f segons o %02ld : %02ld minuts:segons; frames %ld)\n", 
           argv[1], 
           sfinfo.channels,
           sfinfo.samplerate, 
           sfinfo.format, 
           ((sfinfo.format & SF_FORMAT_WAV) > 0? "WAVE": "no WAVE"),   
           ((sfinfo.format & SF_FORMAT_PCM_16) > 0? "16bits" : 
           ((sfinfo.format & SF_FORMAT_PCM_S8) > 0? "8bits" : "")),
           numTotalMostres, //sfinfo.frames, 
           (sfinfo.frames * sfinfo.channels * ((sfinfo.format & SF_FORMAT_PCM_16) > 0? 2 : 1)),
           //              (float)(sfinfo.frames * sfinfo.channels)  / (float)sfinfo.samplerate,
           //              ((float)(sfinfo.frames * sfinfo.channels)  / (float)sfinfo.samplerate)/60.0,
           (float)sfinfo.frames / (float)sfinfo.samplerate,
           (sfinfo.frames  / sfinfo.samplerate) / 60,
           (sfinfo.frames  / sfinfo.samplerate) % 60,
           (sfinfo.frames * sfinfo.channels)
    );
    
    
    // https://libsndfile.github.io/libsndfile/api.html#read
    // The SF_INFO structure is for passing data between the calling function and the library when opening a file for reading or writing. It is defined in sndfile.h as follows:
    // 
    // typedef struct
    // {       sf_count_t  frames ;     /* Used to be called samples. */
    //         int         samplerate ;
    //         int         channels ;
    //         int         format ;
    //         int         sections ;
    //         int         seekable ;
    //     } SF_INFO ;
    // Pero en File Read Functions diu: For the frames-count functions, the frames parameter specifies the number of frames. A frame is just a block of samples, one for each channel.
    
    numCanales = sfinfo.channels;
    // int bitsMuestra = ((sfinfo.format & SF_FORMAT_PCM_S8) > 0? 8 : 
    //                             ((sfinfo.format & SF_FORMAT_PCM_16) > 0? 16 : 0));
    bitsMuestra = ((sfinfo.format & SF_FORMAT_PCM_16) > 0? 16 : 
    ((sfinfo.format & SF_FORMAT_PCM_S8) > 0? 8 : 0));
    if (bitsMuestra == 0)
        printf("Error en el fichero %s: bitsMuestra = %d\n", infilename, bitsMuestra);
    
    frecuenciaMuestreo = sfinfo.samplerate;
    //    printf("-3 numTotalMostres %ld, sfinfo.frames %ld\n", numTotalMostres, sfinfo.frames);
    
    
    //Asignamos el tipo de formato para pasarselo a openAL dependiendo del número de canales y los bits de muestra
    formatoAudio = 0;
    // NO sembla que el comprovar Format 10006d: PCM_S8 (1d)?: 0d vs PCM_S16 (2d)?: 2d funcione per a detectar si les mostres són 8bits o 16 bits!!!
//       if(bitsMuestra == 8)
//       {
//        printf("Formats de 8bits i %02d canal/s\n", numCanales);
//        if(numCanales == 1)
//        formatoAudio = AL_FORMAT_MONO8;
//        else if(numCanales == 2)
//        formatoAudio = AL_FORMAT_STEREO8;
//       }
//       else if(bitsMuestra == 16)
//       {
//        printf("Formats de 16bits i %02d canal/s\n", numCanales);
//        if(numCanales == 1)
//        formatoAudio = AL_FORMAT_MONO16;
//        else if(numCanales == 2)
//        formatoAudio = AL_FORMAT_STEREO16;
//       }
//       if(!formatoAudio) {
//        printf("Formato incompatible \n");
//        exit( -1 );
//       }
    
    //     //ALURE
    //     /* Get the sound format, and figure out the OpenAL format */
    if(sfinfo.channels == 1)
        formatoAudio = AL_FORMAT_MONO16;
        else if(sfinfo.channels == 2)
            formatoAudio = AL_FORMAT_STEREO16;
            else {
                fprintf(stderr, "Unsupported channel count: %d\n", sfinfo.channels);
                //         sf_close(player->sndfile);
                //         player->sndfile = NULL;
                return 0;
            }  
            
            
            // printf("-2 numTotalMostres %ld, sfinfo.frames %ld\n", numTotalMostres, sfinfo.frames);
            
            //
            //
            //size_t frame_size;
            frame_size = (size_t)(BUFFER_LEN * sfinfo.channels) * sizeof(short);
            data = malloc(frame_size);
            printf("Reservats %ld elements en data\n", frame_size );
            //
            
            
            // Un apunt:
            // printf ("sizeof(short); es %d\n", sizeof(short) ); --> 2
            //
            
            // Anar carregant-lo a trocets
            //
            int i = 0;
            for(i = 0; i < NUM_BUFFERS; i++)
            {
                //printf("i %d -1 numTotalMostres %ld, sfinfo.frames %ld\n", i, numTotalMostres, sfinfo.frames);
                
                //samples = (int) sf_read_int(infile, data, BUFFER_LEN/numCanales);
                //samples = (int) sf_readf_short(infile, (short*)data, BUFFER_LEN/numCanales);
                //samples = (int) sf_read_short(infile, (short*)data, BUFFER_LEN);
                //     samples = sf_read_short(infile, (short*)data, BUFFER_LEN);
                //     samples = sf_read_short(infile, data, BUFFER_LEN);
                samples = sf_readf_short(infile, data, BUFFER_LEN);
                // NO en sf-read_short quan es > 1 canal
                //samples = sf_read_short(infile, data, BUFFER_LEN);
                
                if (samples > 0) {//???
                    mostresLLegides += samples;
                    printf("Ha llegit %ld mostres i en porta %ld\n", samples, mostresLLegides);
                    
                    mostresEnBytes = samples* sfinfo.channels * (sf_count_t)sizeof(short);
                    
                    alBufferData(buffers[i], formatoAudio, data, mostresEnBytes, frecuenciaMuestreo); //BUFFER_LEN, frecuenciaMuestreo);
                    printf("Ha asignat %ld elements en buffer[%d]\n", samples, i);
                }
            }
            //   printf("queued?\n");
             alGetSourcei(alsource, AL_BUFFERS_QUEUED, &queued); printf("Ara, en cola (queued): %d\n", queued);
            
            //   /* If no buffers are queued, playback is finished */
            //   printf("queued?alGetSource\n");
            //   alGetSourcei(alsource, AL_BUFFERS_QUEUED, &queued);
            //   printf("Encolar %d buffers %p\n", NUM_BUFFERS, buffers);
            
            //encolamos los buffers y empezamos a reproducir
            alSourceQueueBuffers(alsource, NUM_BUFFERS, buffers);
            alGetSourcei(alsource, AL_BUFFERS_QUEUED, &queued); printf("Ara, en cola (queued): %d\n", queued);
            if((error=alGetError()) != AL_NO_ERROR)
                printf("Error rellenando los buffers por 1a vez: %s\n", alutGetErrorString(error));
            //   printf("queued?\n");
            //   alGetSourcei(alsource, AL_BUFFERS_QUEUED, &queued); printf("Ara, en cola: %d\n", queued);
            //   
            printf("Que comence la música? \n");
            alSourcePlay(alsource);
            if((error=alGetError()) != AL_NO_ERROR)
                printf("Que comence la música? Error: %s\n", alutGetErrorString(error));
            // {
            //   ALint estado;
            //   i=0;
            //   do {
            //      alutSleep( 0.1f );
            //      printf("%c\r", barraActivitat[i++%4]); fflush( stdout );   
            //      alGetSourcei(alsource, AL_SOURCE_STATE, &estado);
            //   } while(estado == AL_PLAYING);
            // }
            
            printf("En portem %ld. Ara ha llegir les que queden de les %ld mostres\n", mostresLLegides,  numTotalMostres); //sfinfo.frames*sfinfo.channels*); 
            
            i=0;
            while (mostresLLegides < numTotalMostres) {
                //Consultamos a OpenAl si tiene buffers libres, si no tiene ninguno, continua reproduciendo.
                alGetSourcei(alsource, AL_BUFFERS_PROCESSED, &buffersLibres);
                //printf("buffers lliures %d\n", buffersLibres);
                //      if(buffersLibres <= 0)
                //      continue;
                if( buffersLibres > 0 ){
                    //printf("Tenim %d buffers lliures ara\n", buffersLibres);
                    //Cuando se libera algun buffer, se incrementa buffersLibres, y se encola el siguiente trozo
                    //a leer desde el descriptor de lectura (f)
                    while(buffersLibres > 0)
                    {              
                        //se lee el siguiente trozo 
                        //samples = (int) sf_read_int(infile, data, BUFFER_LEN/numCanales);
                        //samples = (int) sf_readf_short(infile, (short*)data, BUFFER_LEN/numCanales);
                        //         samples = (int) sf_read_short(infile, (short*)data, BUFFER_LEN);
                        //FUNCINONA EN
                        samples = sf_readf_short(infile, data, BUFFER_LEN);
                        // NO en sf-read_short quan es > 1 canal
                        //samples = sf_read_short(infile, data, BUFFER_LEN);
                        if (samples > 0) {//???
                            mostresLLegides += samples;
                            //printf("Ha de llegir %5ld mostres i en porta %7ld\n", sfinfo.frames, mostresLLegides);        
                            //lo encolamos a OpenAL        
                            alSourceUnqueueBuffers(alsource, 1, &bufferID);
                            //alGetSourcei(alsource, AL_BUFFERS_QUEUED, &queued); printf("En lleve un de la cola (queued): %d, processats/lliures %d\n", queued, buffersLibres);
                            
                            //samples *= sfinfo.channels * (sf_count_t)sizeof(short);
                            mostresEnBytes = samples* sfinfo.channels * (sf_count_t)sizeof(short);
                            
                            alBufferData(bufferID, formatoAudio, (short*)data, mostresEnBytes, frecuenciaMuestreo); //, frecuenciaMuestreo);         
                            alSourceQueueBuffers(alsource, 1, &bufferID);
                            buffersLibres--;
                            //alGetSourcei(alsource, AL_BUFFERS_QUEUED, &queued); printf("En fique un a la cola (queued): %d, processats/lliures %d\n", queued, buffersLibres);
                            //printf("."); fflush(stdout);         
                            // printf("%c\r", barraActivitat[i++%4]); fflush( stdout );      
                            printf("%7ld / %7ld  mostres \r", sfinfo.frames, mostresLLegides); fflush( stdout ); 
                            //alSourcePlay(alsource);
                            int val=0;
                            alGetSourcei(alsource, AL_SOURCE_STATE, &val);
                            if(val != AL_PLAYING) {
                                alSourcePlay( alsource );
                            }
                        }        
                        //else
                        //break;
                        //  {
                        //   ALint estado;
                        //   i=0;
                        //   do {
                        //      alutSleep( 0.1f );
                        //      printf("%c\r", barraActivitat[i++%4]); fflush( stdout );   
                        //      alGetSourcei(alsource, AL_SOURCE_STATE, &estado);
                        //   } while(estado == AL_PLAYING);
                        // }
                        
                    }//while( buffersLibres-- )
                }//if( buffersLibres > 0)
            } //while (mostresLLegides < numTotalMostres) 
            
           printf("\n"); // Pq es quede a vista el recompte de les mostres llegides         
            
            // Si encara queden buffers per fer-los sonar, t'esperes
            {
                ALint estado;
                i=0;
                do {
                    alutSleep( 0.1f );
                    printf("%c\r", barraActivitat[i++%4]); fflush( stdout );   
                    alGetSourcei(alsource, AL_SOURCE_STATE, &estado);
                } while(estado == AL_PLAYING);
            }
            //   printf("\nSegon partttttttttttttttttttttttttttttttttt?\n");
            // 
            //   {
            //     ALuint bufferID2;            // The OpenAL sound buffer ID
            //     ALuint sourceID2;            // The OpenAL sound source
            //     alutInit (&argc, argv);
            //     //bufferID = alutCreateBufferHelloWorld ();
            // //       bufferID2 = alutCreateBufferFromFile( "escala.wav" ); // Se talla!!!
            //     bufferID2 = alutCreateBufferFromFile( "KDE-Im-Phone-Ring.wav");
            // //     bufferID2 = alutCreateBufferFromFile( "oldphone-mono.wav");
            //     alGenSources (1, &sourceID2);
            //     alSourcei (sourceID2, AL_BUFFER, bufferID2);
            //     alSourcePlay (sourceID2);
            //     alutSleep (3);
            //     alDeleteSources( 1, &sourceID2 );
            //     alDeleteBuffers( 1, &bufferID2 );
            // //    alutExit ();
            //   }
            
            /* Close input and output files. */
            sf_close (infile) ;
            free( data );
            
            alDeleteSources(1, &alsource);
            alDeleteBuffers(NUM_BUFFERS, buffers);
            
            alutExit();
            
            return EXIT_SUCCESS;
}
