/*
 * Ejemplo de aplicación usando las funciones de libsndfile para la carga de audio desde fichero
 * API <-- https://libsndfile.github.io/libsndfile/api.html
 
$ gcc openal_libsdnfile__precarga.c  -o openal_libsdnfile__precarga -lalut $(pkg-config openal sndfile  --cflags --libs) 
o
$ gcc openal_libsdnfile__precarga.c  -o openal_libsdnfile__precarga -lalut -I/usr/include/AL -lopenal -lsndfile
$ openal_libsdnfile__precarga escala.wav 
*/

#include <stdio.h>
#include <stdlib.h> // EXIT_SUCCESS
#include <math.h>
#include <GL/glut.h>
//#include <AL/al.h>
//#include <AL/alc.h>  // Ja els porten darrere en alut.h
#include <AL/alut.h>
#include <string.h> //memset
/* Include this header file to use functions from libsndfile. */
#include	<sndfile.h>

// /* libsndfile can handle more than 6 channels but we'll restrict it to 1 (artxius monofònics). */
// #define		MAX_CHANNELS	1


int main(int argc, char *argv[])
{
  ALuint bufferID;            // The OpenAL sound buffer ID
  ALuint sourceID;            // The OpenAL sound source
  char *datosAudio;           // The sound buffer data from file
  long tamanyBuffer;
  ALenum error;
  ALenum formatoAudio; //mono o stereo (1 o 2 canales)
  ALint nFrames;
  int i, 
      numCanales, frecuenciaMuestreo,
      bitsMuestra; // tamanyMostra
  char barraActivitat[4] = {'|', '/', '-', '\\'}; 
  	/* A SNDFILE is very much like a FILE in the Standard C library. The
	** sf_open function return an SNDFILE* pointer when they sucessfully
	** open the specified file.
	*/
  SNDFILE	*infile;

	/* A pointer to an SF_INFO struct is passed to sf_open.
	** On read, the library fills this struct with information about the file.
	** On write, the struct must be filled in before calling sf_open.
	*/
   SF_INFO		sfinfo ;
	//int			readcount ;
   char	infilename[128] = "escala.wav" ;

   
    if( argc > 1) {
        printf("argv[1] = %s\n", argv[1]);
        strcpy(infilename, argv[1]);
        printf("infilename = %s\n", infilename);
    }
   
    
  if (!alutInit(&argc, argv))   {
      printf("Fallo al iniciar openAL.\n");
      return 0;
    }

   printf ("La versio de OpenAL instalada es %s \n", alGetString(AL_VERSION) );
   //printf ("Las diferentes extensiones de OpenAL son: %s \n", alGetString(AL_EXTENSIONS));
   printf ("La versio de ALUT instalada es %d.%d \n", alutGetMajorVersion(), alutGetMinorVersion());

   printf ("La versio de libsndfile instalada es %s \n", sf_version_string());
  
  
   /* The SF_INFO struct must be initialized before using it.
	*/
   memset (&sfinfo, 0, sizeof (sfinfo)) ;

   /* Here's where we open the input file. We pass sf_open the file name and
	** a pointer to an SF_INFO struct.
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
	if (! (infile = sf_open (infilename, SFM_READ, &sfinfo)))
	{	/* Open failed so print an error message. */
		printf ("Not able to open input file %s.\n", infilename) ;
		/* Print the error message from libsndfile. */
		puts (sf_strerror (NULL)) ;
		return 1 ;
		} ;

//Açò era per vore el valor de les constants, ... per si de cas.
//    printf("Format %0xd: PCM_S8 (%0xd)?: %0xd vs PCM_S16 (%0xd)?: %0xd \n", 
//                sfinfo.format,
//                SF_FORMAT_PCM_S8,
//                (sfinfo.format & SF_FORMAT_PCM_S8),
//                SF_FORMAT_PCM_16,
//                (sfinfo.format & SF_FORMAT_PCM_16) );

   numCanales = sfinfo.channels;
   // De qualsevol d'estes dos formes es pot trauere el tamany en bits de les mostres
   // bitsMuestra = ((sfinfo.format & SF_FORMAT_PCM_S8) > 0? 8 : 
   //                             ((sfinfo.format & SF_FORMAT_PCM_16) > 0? 16 : 0));
   bitsMuestra = ((sfinfo.format & SF_FORMAT_PCM_16) > 0? 16 : 
                            ((sfinfo.format & SF_FORMAT_PCM_S8) > 0? 8 : 0));
   frecuenciaMuestreo = sfinfo.samplerate;

   printf("%s: canals %d, freqMostreig %d, format %0x [%s, %s], frames %ld, frames Totals %ld, %ld bytes i temps (%04f segons o %02ld:%02ld minuts)\n", 
             argv[1], 
             numCanales, //sfinfo.channels,
             frecuenciaMuestreo, //sfinfo.samplerate, 
             sfinfo.format, 
             ((sfinfo.format & SF_FORMAT_WAV) > 0? "WAVE": "no WAVE"),   
             ((sfinfo.format & SF_FORMAT_PCM_16) > 0? "16bits" : 
                            ((sfinfo.format & SF_FORMAT_PCM_S8) > 0? "8bits" : "")),
             sfinfo.frames, // Quants frames té
             (sfinfo.frames * sfinfo.channels), // Quants "frames" n'hi han en total?
             (sfinfo.frames * sfinfo.channels * ((sfinfo.format & SF_FORMAT_PCM_16) > 0? 2 : 1)),
//              (float)(sfinfo.frames * sfinfo.channels)  / (float)sfinfo.samplerate,
//              ((float)(sfinfo.frames * sfinfo.channels)  / (float)sfinfo.samplerate)/60.0,
             (float)sfinfo.frames / (float)sfinfo.samplerate, // temps de duració de l'audio en segons
             (sfinfo.frames  / sfinfo.samplerate) / 60, // temps de duració de l'audio en minuts
             (sfinfo.frames  / sfinfo.samplerate) % 60  // i segons
            );


  if (bitsMuestra == 0)
    printf("Error en el fichero, bitsMuestra = %d\n", bitsMuestra);
    
  //Asignamos el tipo de formato para pasarselo a openAL dependiendo del número de canales y los bits de muestra
  formatoAudio = 0;
  if(bitsMuestra == 8)
  {
   //printf("Formats de 8bits i %02d canal/s\n", numCanales);
   if(numCanales == 1)
   formatoAudio = AL_FORMAT_MONO8;
   else if(numCanales == 2)
   formatoAudio = AL_FORMAT_STEREO8;
  }
  else if(bitsMuestra == 16)
  {
   printf("Formats de 16bits i %02d canal/s\n", numCanales);
   if(numCanales == 1)
   formatoAudio = AL_FORMAT_MONO16;
   else if(numCanales == 2)
   formatoAudio = AL_FORMAT_STEREO16;
  }
  if(!formatoAudio)
  printf("Formato incompatible \n");

  
//   printf("A llegir: canals %d frec %d, frames %ld format %d\n", sfinfo.channels, frecuenciaMuestreo, sfinfo.frames, ((sfinfo.format & SF_FORMAT_PCM_16) > 0? 2 : 1) );
//   printf("sizeof(char) %ld sizeof(short) %ld sizeof(int) %ld\n", sizeof (char), sizeof(short), sizeof(int) ); // 1, 2, i 4
//  
  // Create sound buffer and source
  alGenBuffers(1, &bufferID);
  error = alGetError(); 
  if (error) printf("Error al crear el buffer:: alGetError: %s\n", alutGetErrorString(error));

  alGenSources(1, &sourceID);
  error = alGetError(); 
  if (error) printf("Error al crear la font:: alGetError: %s\n", alutGetErrorString(error));

  tamanyBuffer = (sfinfo.frames * sfinfo.channels * ((sfinfo.format & SF_FORMAT_PCM_16) > 0? 2 : 1)) ; 
  printf("Reservant lloc per a %ld bytes\n", tamanyBuffer);
  datosAudio = (char *)malloc( tamanyBuffer );
	if (datosAudio == NULL)	{
        printf ("Error : no hay suficiente memoria para %ld bytes de audio.\n", tamanyBuffer) ;
		return( -1 );
    }
// //    else printf("datosAudio reservat en %p \n", datosAudio);        
   
  // Leer las muestras de audio con libsndfile
  // Funciona tant en 
  //nFrames = (int) sf_read_short(infile, (short *) datosAudio,
  // com en 
  nFrames = (int) sf_readf_short(infile, (short *) datosAudio, tamanyBuffer);
  printf("Hem llegit %d de %ld frames esperats.\n", nFrames, sfinfo.frames); //tamanyBuffer );
  alBufferData(bufferID, formatoAudio, datosAudio, 
                tamanyBuffer, sfinfo.samplerate); //(*frecuenciaMuestreo)/2);
  error = alGetError(); 
  if (error) printf("%s\n", alutGetErrorString(error));

  alSourcei(sourceID, AL_BUFFER, bufferID);
  error = alGetError(); 
    if (error) printf("%s\n", alutGetErrorString(error));
    
  printf("Que sone la música!\n");
  alSourcePlay( sourceID );
    
  ALint estado;
  i=0;
  do {
     alutSleep( 0.1f );
     printf("%c\r", barraActivitat[i++%4]); fflush( stdout );   
     alGetSourcei(sourceID, AL_SOURCE_STATE, &estado);
  } while(estado == AL_PLAYING);

  /* Close input and output files. */
  sf_close (infile) ;

  alDeleteSources(1, &sourceID);
  alDeleteBuffers(1, &bufferID);
  
  alutExit();
  
  return EXIT_SUCCESS;
}
