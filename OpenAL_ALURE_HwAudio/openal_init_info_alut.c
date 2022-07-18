/*
 * 
 *  Ací estic replegant les funcions d'inicialització i tancament en el API de alt nivell d'OpenAL mitjançant ALUT
 * 
 *  Tret de la pràctica de SMII
 
 $ gcc  openal_init_info_alut.c -o openal_init_info_alut $(pkg-config freealut --cflags --libs)

 
 
 ??? Fique com obtindre el context???
  
???
*$ pkg-config openal --cflags --libs
-I/usr/include/AL -lopenal
 $ g++ openal_fft.c -o openal_fft -lalut `pkg-config openal --cflags --libs` -lglut -lGL

*/


#include <stdio.h>
#include <stdlib.h>     // Per al 'exit'
#include <AL/alut.h>

#include <string.h> //strlen

//#include <unistd.h>
//#include <math.h>


int main (int argc, char **argv)
{
  int i;
  const ALchar *pDeviceList, *llista;
  ALuint elBuffer, laFont;
  ALenum error;


  alutInit (&argc, argv);
  error = alGetError(); 
  if (error) printf("Error en alutInit: %s\n", alutGetErrorString(error));
       
  //alutSleep( segonsEnPosiblePartDecimal );
  //usleep( 1000000 ); printf(" \r"); fflush( stdout );
  
  printf ("La versio de ALUT instalada es %d.%d \n", alutGetMajorVersion(), alutGetMinorVersion());

  printf("OpenAL Renderer is '%s'\n", alGetString(AL_RENDERER) );
  printf("OpenAL Version is '%s'\n", alGetString(AL_VERSION) );		    
  printf("OpenAL Vendor is '%s'\n", alGetString(AL_VENDOR) );
  
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


  // Que sone algo, no?
  printf("\n\n");
  
  // elBuffer = alutCreateBufferHelloWorld ();
  // Per que la comparació siga més justa
  elBuffer = alutCreateBufferWaveform(ALUT_WAVEFORM_SINE, 440.0, 0.0, 1.0);
  // ??? frecMostreig, tamany de mostra, canals???
  
  alGenSources (1, &laFont);
  alSourcei (laFont, AL_BUFFER, elBuffer);
  alSourcePlay (laFont);
  alutSleep (1);
 
  printf("\n");
  
  alutExit();
  return EXIT_SUCCESS;
} // fi de main
