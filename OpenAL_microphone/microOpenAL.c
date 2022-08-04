/*
  Asegurar-se de tindre instalà la libsndfile (binari i per desenrrotllament): instalar 
  $ apt-get install libsndfile-dev libsndfile

  Compilar en a
  $ gcc microOpenAL.c  -o microOpenAL -lalut -lopenal -lsndfile

  Pots vore-ho en Pulse Audio i en atres en ferramentes similars:
  $ pavumeter 

*/


#include <stdio.h>
#include <math.h>
#include <malloc.h>
#include <sndfile.h>
#include <AL/al.h>
#include <AL/alc.h>
#include <AL/alut.h>
#include <termios.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>

int kbhit();

#define	OUTPUT_WAVE_FILE		"Capture.wav"
#define	OUTPUT_WAVE_FILE_RAW		"CaptureCrudo"
#define BUFFERSIZE				4410


const int SRATE = 22050;
#define MAXNUMDISPOSITIUS 50

ALint nMostres;



int main(int argc, char *argv[]) {
//   double freq = SRATE;
  //double duracion = 5;
  //long numFrames = duracion * SRATE;
  const ALCchar	*szDefaultCaptureDevice;
  FILE *pFile;
  //double Buffer[BUFFERSIZE];

  // Per fer-ho depenent del temps
  //ALCvoid* Buffer = malloc(numFrames * sizeof(double));
  //if (Buffer == NULL) {
  //    fprintf(stderr, "Fallo al crear buffer para salida.\n");
  //  }
  // Ara, estàtic: lleig un buffer a un buffer i grave a disc. Si tot va prou ràpid no es notarà.
  ALchar* Buffer[BUFFERSIZE];

  int i;
  const ALchar *nomsDispositius[MAXNUMDISPOSITIUS];
  int nDispositius;
  ALuint buffer, fuente;
  int error;
  ALint sourceState;





  if (!alutInit(&argc, argv))
    {
      printf("Fallo al iniciar openAL.\n");
      return 0;
    }

  printf ("La version de OpenAL instalada es %s \n\n Las diferentes extensiones de OpenAL son: %s \n\n\n", alGetString(AL_VERSION), alGetString(AL_EXTENSIONS));
  printf ("La versión de ALUT instalada es %d.%d \n", alutGetMajorVersion(), alutGetMinorVersion());

  ALCcontext *pContext = alcGetCurrentContext();
  ALCdevice *pDevice = alcGetContextsDevice(pContext);
  if (alcIsExtensionPresent(pDevice, "ALC_EXT_CAPTURE") == AL_FALSE)
    {
      printf("Fallo al detectar extension de captura.\n");
      return 0;
    }

  // Lista de dispositivos de captura disponibles
  const ALchar *pDeviceList = alcGetString(NULL, ALC_CAPTURE_DEVICE_SPECIFIER);

  nDispositius = -1;
  for (i= nDispositius+1; i < MAXNUMDISPOSITIUS; i++)
      nomsDispositius[ i ] = NULL;
  if (pDeviceList)
    {
      printf("\nDispositivos de captura disponibles:\n");

      while (*pDeviceList)
	{
	  nDispositius++;
	  printf("(%d) %s\n", nDispositius, pDeviceList);
	  nomsDispositius[ nDispositius ] = pDeviceList;
	  pDeviceList += strlen(pDeviceList) + 1;
	}
    }
  //for (i= nDispositius+1; i < MAXNUMDISPOSITIUS; i++)
  //    nomsDispositius[ i ] = NULL;

  szDefaultCaptureDevice = alcGetString(NULL, ALC_CAPTURE_DEFAULT_DEVICE_SPECIFIER);
  printf("\nDispositivo de captura por defecto es '%s'\n\n", szDefaultCaptureDevice);

  //  ALCdevice *device = alcCaptureOpenDevice(szDefaultCaptureDevice, freq, AL_FORMAT_MONO16, BUFFERSIZE);
  ALCdevice *device;

  printf("Quin dispositiu vols provar? ");
  scanf("%d", &i);
  printf("\nHas triat %d\n", i);
  device  = alcCaptureOpenDevice(nomsDispositius[i], freq, AL_FORMAT_MONO16, BUFFERSIZE);
  printf("Dipositivo de captura: '%s' esta abierto\n\n", alcGetString(device, ALC_CAPTURE_DEVICE_SPECIFIER));

  //  while (getchar() != EOF);
  printf("Pulse intro para comenzar a grabar. Pulse cualquier  tecla para finalizar la grabacion.\n");
  getchar();

  alcCaptureStart(device);

  //pFile = FILE *abrir_RAW( char nom );
  //Fichero para datos en crudo
  pFile = fopen(OUTPUT_WAVE_FILE_RAW, "wb");

  //sndFile = SF_INFO *abrir_WAV( char *nom );
  //Preferencias para el archivo wav usando librearia libsndfile.
  SF_INFO info;
  info.format = SF_FORMAT_WAV | SF_FORMAT_PCM_16;
  info.channels = 1;
  info.samplerate = SRATE;
  // Abrimos fichero wav en modo escritura.
  SNDFILE *sndFile = sf_open(OUTPUT_WAVE_FILE, SFM_WRITE, &info);
  if (sndFile == NULL) {
    fprintf(stderr, "Error al abrir el fichero");
    //free(Buffer);
    return -1;
  }



  float bloq = info.channels * 16/8;
  while (!kbhit()) {
    alcGetIntegerv(device, ALC_CAPTURE_SAMPLES, BUFFERSIZE, &nMostres);
    printf("nMostress: %d\n", nMostres);

    if (nMostres > (BUFFERSIZE / bloq))
      {

	alcCaptureSamples(device, Buffer, BUFFERSIZE/bloq);

	sf_writef_short(sndFile, (short*)Buffer, BUFFERSIZE/bloq);
	fwrite(Buffer,BUFFERSIZE , 1, pFile);

      }

    // No falta un 
    //alcCaptureSamples(device, (ALCvoid *)Buffer, nMostres);
    // per si n'hi han menos de (BUFFERSIZE / bloq) ?

  }



  //cerramos todo.
  alcCaptureStop(device);
  alcCaptureCloseDevice(device);


  //cerrar_WAV( sndFile);
  sf_write_sync(sndFile);
  sf_close(sndFile);
  printf("\nSaved captured audio data to '%s'\n", OUTPUT_WAVE_FILE);

  // cerrar_RAW( pFile );
  fclose(pFile);


 
   // Podria reproduir el WAV, encara que siga ...
   // reproduirFitxer( char *nom );
   buffer = alutCreateBufferFromFile( OUTPUT_WAVE_FILE );
   error = alGetError(); 
    if (error) printf("Error: %s\n", alutGetErrorString(error));

   alGenSources (1, &fuente);
   alSourcei(fuente, AL_BUFFER, buffer);
   alSourcePlay (fuente);
   // Mentres n'hi haja datos que no pare
   alGetSourcei( fuente, AL_SOURCE_STATE, &sourceState);
   while (sourceState == AL_PLAYING)
    alGetSourcei( fuente, AL_SOURCE_STATE, &sourceState);

   alDeleteSources(1, &fuente);
   alDeleteBuffers(1, &buffer);




  //Tanca sense la corresponent cridà d'ALUT
  // No faria el mateis
  alutExit ();

  //  alcMakeContextCurrent(NULL);
  //  alcDestroyContext(pContext);
  //  alcCloseDevice(pDevice);
  return EXIT_SUCCESS; //EXIT_FAILURE (stdlib.h)
}

//Funcion para salir de la grabaciÃ³n al pulsar cualquier tecla.
int kbhit()
{
  struct termios oldt, newt;
  int ch;
  int oldf;

  tcgetattr(STDIN_FILENO, &oldt);
  newt = oldt;
  newt.c_lflag &= ~(ICANON | ECHO);
  tcsetattr(STDIN_FILENO, TCSANOW, &newt);
  oldf = fcntl(STDIN_FILENO, F_GETFL, 0);
  fcntl(STDIN_FILENO, F_SETFL, oldf | O_NONBLOCK);

  ch = getchar();

  tcsetattr(STDIN_FILENO, TCSANOW, &oldt);
  fcntl(STDIN_FILENO, F_SETFL, oldf);

  if(ch != EOF)
    {
      ungetc(ch, stdin);
      return 1;
    }

  return 0;
}
