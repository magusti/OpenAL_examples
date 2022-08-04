/*
 * Compilar:
 $ make
 $ gcc helloWorld_extensions.c getch.c -o helloWorld_extensions -lalut `pkg-config openal --cflags --libs`

 * $ gcc helloWorld2.c  getch.c -lalut -lopenal -o helloWorld2 && helloWorld2
 *
 * (C) M. Agustí. Oct. 2k9
 *   - Se sustituye la función de leer con eco (getchar) por una versión sin echo (getch)
 *   - Cambian nombres de variables y mensajes en pantalla.
 *
 * (C) M. Agustí. Nov. 2k8
 *   - Nov. 2k8. Se añade un bucle para consumir caracteres por si se teclea más de uno 
 *  como respuesta del usuario.
 */

#include <stdio.h> //getchar, printf
#include <stdlib.h>
#include <AL/alut.h>
#include "utils.h"   // AND, getch
#include <string.h> // strcmp
#include <AL/alext.h>
#include <AL/efx.h> //alDeleteEffects



//Les comente per vore el minim de còdic
// Funciones tretes d' openal-soft/examples/common/alhelpers.c
//https://github.com/kcat/openal-soft/blob/master/examples/common/alhelpers.c
//
/* InitAL opens a device and sets up a context using default attributes, making
 * the program ready to call OpenAL functions. */
int InitAL(char ***argv, int *argc)
{
  const ALCchar *name;
  ALCdevice *device;
  ALCcontext *ctx;

  /* Open and initialize a device */
  device = NULL;
  if(argc && argv && *argc > 1 && strcmp((*argv)[0], "-device") == 0)
    {
      device = alcOpenDevice((*argv)[1]);
      if(!device)
	fprintf(stderr, "Failed to open \"%s\", trying default\n", (*argv)[1]);
      (*argv) += 2;
      (*argc) -= 2;
    }
  if(!device)
    device = alcOpenDevice(NULL);
  if(!device)
    {
      fprintf(stderr, "Could not open a device!\n");
      return 1;
    }

  ctx = alcCreateContext(device, NULL);
  if(ctx == NULL || alcMakeContextCurrent(ctx) == ALC_FALSE)
    {
      if(ctx != NULL)
	alcDestroyContext(ctx);
      alcCloseDevice(device);
      fprintf(stderr, "Could not set a context!\n");
      return 1;
    }

  name = NULL;
  if(alcIsExtensionPresent(device, "ALC_ENUMERATE_ALL_EXT"))
    name = alcGetString(device, ALC_ALL_DEVICES_SPECIFIER);
  if(!name || alcGetError(device) != AL_NO_ERROR)
    name = alcGetString(device, ALC_DEVICE_SPECIFIER);
  printf("Opened \"%s\"\n", name);

  return 0;
}

/* CloseAL closes the device belonging to the current context, and destroys the
 * context. */
void CloseAL(void)
{
  ALCdevice *device;
  ALCcontext *ctx;

  ctx = alcGetCurrentContext();
  if(ctx == NULL)
    return;

  device = alcGetContextsDevice(ctx);

  alcMakeContextCurrent(NULL);
  alcDestroyContext(ctx);
  alcCloseDevice(device);
}
// Fi de openal-soft/examples/common/alhelpers.c



// De info-openal.c
static const int indentation = 4;
static const int maxmimumWidth = 79;
static void printChar(int c, int *width)
{
  putchar(c);
  *width = ((c == '\n') ? 0 : ((*width) + 1));
}
static void indent(int *width)
{
  int i;
  for(i = 0; i < indentation; i++)
    printChar(' ', width);
}
static void printList(const char *header, char separator, const char *list)
{
  int width = 0, start = 0, end = 0;
  printf("%s:\n", header);
  if(list == NULL || list[0] == '\0')
    return;
  indent(&width);
  while(1)
    {
      if(list[end] == separator || list[end] == '\0')
	{
	  if(width + end - start + 2 > maxmimumWidth)
	    {
	      printChar('\n', &width);
	      indent(&width);
	    }
	  while(start < end)
	    {
	      printChar(list[start], &width);
	      start++;
	    }
	  if(list[end] == '\0')
	    break;
	  start++;
	  end++;
	  if(list[end] == '\0')
	    break;
	  printChar(',', &width);
	  printChar(' ', &width);
	}
      end++;
    }
  printChar('\n', &width);
}
static void die(const char *kind, const char *description)
{
  fprintf(stderr, "%s error %s occured\n", kind, description);
  exit(EXIT_FAILURE);
}
static void checkForErrors(void)
{
  {
    ALCdevice *device = alcGetContextsDevice(alcGetCurrentContext());
    ALCenum error = alcGetError(device);
    if(error != ALC_NO_ERROR)
      die("ALC", (const char*)alcGetString(device, error));
  }
  {
    ALenum error = alGetError();
    if(error != AL_NO_ERROR)
      die("AL", (const char*)alGetString(error));
  }
}

static void printEFXInfo(void)
{
  ALCint major, minor, sends;
  ALCdevice *device;
  ALuint obj;
  int i;
  const ALenum effects[] = {
    AL_EFFECT_EAXREVERB, AL_EFFECT_REVERB, AL_EFFECT_CHORUS,
    AL_EFFECT_DISTORTION, AL_EFFECT_ECHO, AL_EFFECT_FLANGER,
    AL_EFFECT_FREQUENCY_SHIFTER, AL_EFFECT_VOCAL_MORPHER,
    AL_EFFECT_PITCH_SHIFTER, AL_EFFECT_RING_MODULATOR, AL_EFFECT_AUTOWAH,
    AL_EFFECT_COMPRESSOR, AL_EFFECT_EQUALIZER, AL_EFFECT_NULL
  };
  char effectNames[] = "EAX Reverb,Reverb,Chorus,Distortion,Echo,Flanger,"
    "Frequency Shifter,Vocal Morpher,Pitch Shifter,"
    "Ring Modulator,Autowah,Compressor,Equalizer,";
  const ALenum filters[] = {
    AL_FILTER_LOWPASS, AL_FILTER_HIGHPASS, AL_FILTER_BANDPASS,
    AL_FILTER_NULL
  };
  char filterNames[] = "Low-pass,High-pass,Band-pass,";
  char *current;
  device = alcGetContextsDevice(alcGetCurrentContext());
  if(alcIsExtensionPresent(device, (const ALCchar*)"ALC_EXT_EFX") == AL_FALSE)
    {
      printf("EFX not available\n");
      return;
    }
  alcGetIntegerv(device, ALC_EFX_MAJOR_VERSION, 1, &major);
  alcGetIntegerv(device, ALC_EFX_MINOR_VERSION, 1, &minor);
  checkForErrors();
  printf("EFX version: %d.%d\n", (int)major, (int)minor);
  alcGetIntegerv(device, ALC_MAX_AUXILIARY_SENDS, 1, &sends);
  checkForErrors();
  printf("Max auxiliary sends: %d\n", (int)sends);
  /*
    p_alGenFilters = alGetProcAddress("alGenFilters");
    p_alDeleteFilters = alGetProcAddress("alDeleteFilters");
    p_alFilteri = alGetProcAddress("alFilteri");
    p_alGenEffects = alGetProcAddress("alGenEffects");
    p_alDeleteEffects = alGetProcAddress("alDeleteEffects");
    p_alEffecti = alGetProcAddress("alEffecti");
    checkForErrors();
    if(!p_alGenEffects || !p_alDeleteEffects || !p_alEffecti ||
    !p_alGenFilters || !p_alDeleteFilters || !p_alFilteri)
    {
    printf("Missing EFX functions!\n");
    return;
    }
  */

  // He llevat la part de llistat de filtres que no la gaste ara.

  //p_alDeleteEffects(1, &obj);
  //alDeleteEffects(ALsizei n, const ALuint *effects);
  alDeleteEffects(1, &obj);
  checkForErrors();
  printList("Supported effects", ',', effectNames);
}

// Fi de info-openal.c



/*-------------------VARIABLES_PARA_EFECTOS--------------------------*/
ALCdevice *pDevice = NULL;
ALCcontext *pContextEfectos = NULL;
ALint attribsEfectos[4] = { 0 };
ALCint iSendsEfectos = 0;
/*-------------------VARIABLES_PARA_SLOTS_DE_EFECTOS-----------------*/
ALuint uiEffectSlot[4] = { 0 };
ALuint uiEffect[2] = { 0 };
ALuint uiFilter[1] = { 0 };
ALuint uiLoop;
/*-------------------------------------------------------------------*/




int main (int argc, char **argv)
{
  ALuint buffers[6], fuente, fuenteConEfecto;
  char c;
  float valor = 0.0f;

  /* 
  // Print out usage if no arguments were specified: de moment, "helloWorld"
  if(argc < 2)
  {
  fprintf(stderr, "Usage: %s [-device <name] <filename>\n", argv[0]);
  return 1;
  }
  */
  
  /*
  // Initialize OpenAL, 
  argv++; argc--;
  if(InitAL(&argv, &argc) != 0)
  return 1;
  */      
  alutInit( &argc, argv );

  // Són iguals alutContext i el que es obri el alcOpenDevice?
  // No són iguals!!!
  pDevice =  alcGetContextsDevice(alcGetCurrentContext());
  //  printf ("pDevice =  alcGetContextsDevice(alcGetCurrentContext()); %p\n", pDevice );
  //alcCloseDevice( pDevice );

  //Tutorial 1: Initializing OpenAL and the Effects Extension
  //pDevice = alcOpenDevice(NULL);
  //printf ("pDevice = alcOpenDevice(NULL); %p\n", pDevice );

  /* Confirmar?
     if (*alutContext == *pDevice )
     printf( "alutContext == pDevice\n");
     else
     printf( "alutContext != pDevice\n");
  */
  if (!pDevice) 
    {
      printf(" pDevice no topetat!\n");
      return 1;
    }


  
  //and check for EFX support.
  //  if(alcIsExtensionPresent(alcGetContextsDevice(alcGetCurrentContext()), "ALC_EXT_EFX"))
  if(alcIsExtensionPresent(pDevice, "ALC_EXT_EFX"))
    {
      printf( "Podem gastar les extensions EFX!\n" );
      ///printEFXInfo( );
    }
  else
    {
      fprintf(stderr, "Error: EFX not supported\n");
      //      CloseAL();
      alcCloseDevice( pDevice ); 
      alutExit();
      return 1;
    }


  /* Use Context creation hint to request 4 Auxiliary */  
  /* Sends per Source */
  attribsEfectos[0] = ALC_MAX_AUXILIARY_SENDS;
  attribsEfectos[1] = 4;
  pContextEfectos = alcCreateContext(pDevice, attribsEfectos);

  if (!pContextEfectos)
    {
      printf(" pContextEfectos no creat!\n");
      return 2;
    }

  /* Activate the context */
  alcMakeContextCurrent(pContextEfectos);

  /* Retrieve the actual number of Aux Sends */
  /* available on each Source */
  alcGetIntegerv(pDevice, ALC_MAX_AUXILIARY_SENDS, 1, &iSendsEfectos);

  printf("Device supports %d Aux Sends per Source\n", iSendsEfectos);

    
  
  c = 'h';

  if (argc > 1)
    buffers[0] = alutCreateBufferFromFile( argv[1] );
  else
    buffers[0] = alutCreateBufferHelloWorld ();

  buffers[1] = alutCreateBufferWaveform(ALUT_WAVEFORM_SINE, 440.0, 0.0, 1.0);
  buffers[2] = alutCreateBufferWaveform(ALUT_WAVEFORM_SQUARE,  440.0, 0.0, 1.0);
  buffers[3] = alutCreateBufferWaveform(ALUT_WAVEFORM_SAWTOOTH,  440.0, 0.0, 1.0);
  buffers[4] = alutCreateBufferWaveform(ALUT_WAVEFORM_WHITENOISE,   440.0, 0.0, 1.0);
  buffers[5] = alutCreateBufferWaveform(ALUT_WAVEFORM_IMPULSE, 440.0, 0.0, 1.0);


   

  /*-----------------CREANDO-FUENTES-------------------*/
  alGenSources (1, &fuente);
 
  /*creamos la fuente que faltaba */
  alGenSources(1, &fuenteConEfecto);
  if (alGetError() != AL_NO_ERROR)
    printf("Error al crear la fuente Con Reverb\n");


  /*-----------------------2-CREANDO_SLOTS-------------------------*/
  /*Creamos: slots para efectos auxiliares, efectos, y filtros*/


  /* Try to create 4 Auxiliary Effect Slots */
  alGetError();
  for (uiLoop = 0; uiLoop < 4; uiLoop++)
    {
      alGenAuxiliaryEffectSlots(1, &uiEffectSlot[uiLoop]);
      if (alGetError() != AL_NO_ERROR)
        break;
    }
  printf("Generated %d Aux Effect Slots\n", uiLoop);

  for (uiLoop = 0; uiLoop < 2; uiLoop++)
    {
      alGenEffects(1, &uiEffect[uiLoop]);
      if (alGetError() != AL_NO_ERROR)
	break;
    }
  printf("Generated %d Effects\n", uiLoop);

  /* Set first Effect Type to Reverb and change Decay Time */
  alGetError();
  if (alIsEffect(uiEffect[0]))
    {
      
      alEffecti(uiEffect[0], AL_EFFECT_TYPE, AL_EFFECT_REVERB);
      if (alGetError() != AL_NO_ERROR)
        printf("Reverb Effect not supported\n");
      else{
        alEffectf(uiEffect[0], AL_REVERB_DECAY_TIME, 15.0f);
        if (alGetError() != AL_NO_ERROR)
          printf("Error al establecer AL_REVERB_DECAY_TIME\n");
        alEffectf(uiEffect[0], AL_REVERB_ROOM_ROLLOFF_FACTOR, 10.0f);
        if (alGetError() != AL_NO_ERROR)
          printf("Error al establecer AL_REVERB_ROOM_ROLLOFF_FACTOR\n");
        //habitacion muy grande
        alEffectf(uiEffect[0], AL_REVERB_REFLECTIONS_DELAY, 10.3f);
        if (alGetError() != AL_NO_ERROR)
          printf("Error al establecer AL_REVERB_REFLECTIONS_DELAY\n");
        alEffectf(uiEffect[0], AL_REVERB_REFLECTIONS_GAIN, 3.0f);
        if (alGetError() != AL_NO_ERROR)
          printf("Error al establecer AL_REVERB_REFLECTIONS_GAIN\n");
      }
      /*
      
       alEffecti(uiEffect[0],AL_EFFECT_TYPE, AL_EFFECT_ECHO);
      if (alGetError() != AL_NO_ERROR)
	printf("Echo effect not supported\n");
      else {
	alEffecti(uiEffect[0], AL_ECHO_DELAY, 0.9); //0.206);
	if (alGetError() != AL_NO_ERROR)
	  printf("Error al establecer AL_ECHO_DELAY\n");
	alEffecti(uiEffect[0], AL_ECHO_LRDELAY, 0.403);
	if (alGetError() != AL_NO_ERROR)
	  printf("Error al establecer AL_ECHO_LRDELAY\n");
	alEffecti(uiEffect[0], AL_ECHO_DAMPING, 0.8);
	if (alGetError() != AL_NO_ERROR)
	  printf("Error al establecer AL_ECHO_DAMPING\n");
	alEffecti(uiEffect[0], AL_ECHO_FEEDBACK, 0.5); // 0.5f);
	if (alGetError() != AL_NO_ERROR)
	  printf("Error al establecer AL_ECHO_FEEDBACK\n");
	alEffecti(uiEffect[0], AL_ECHO_SPREAD, 0.0);
	if (alGetError() != AL_NO_ERROR)
	  printf("Error al establecer AL_ECHO_SPREAD\n");
      }
      */
    


    }



  /*-----------------3-ADJUNTANDO_EFECTOS_A_SLOTS------------------*/
  /*Adjuntamos un efecto a un slot de efecto auxiliar*/
 
  /* Attach Effect to Auxiliary Effect Slot */
  /* uiEffectSlot[0] is the ID of an Aux Effect Slot */
  /* uiEffect[0] is the ID of an Effect */

  alAuxiliaryEffectSloti(uiEffectSlot[0],                      //reverb
                         AL_EFFECTSLOT_EFFECT, uiEffect[0]);
  if (alGetError() == AL_NO_ERROR)
    printf("Successfully loaded effect into effect slot\n");

  
  /*-------4-CONFIGURANDO_ENVIOS_AUXILIARES_DE_LA_FUENTE------------*/
  /*Establecemos la manera en que se configuran en la fuente envia los
    "envios auxiliares" para "conectar/alimentar" las "ranuras auxiliares
    de efectos"*/
  
  /* Configure Source Auxiliary Effect Slot Sends */
  /* uiEffectSlot[0] and uiEffectSlot[1] are Auxiliary */
  /* Effect Slot IDs */
  /* uiEffect[0] is an Effect ID */
  /* uiFilter[0] is a Filter ID */
  /* sourceName is a Source ID */

  /* Set Source Send 0 to feed uiEffectSlot[0] without */
  /* filtering */

  alSource3i(fuenteConEfecto, AL_AUXILIARY_SEND_FILTER,
             uiEffectSlot[0],           /*id del slot de efecto auxiliar Reverb*/
             0, AL_FILTER_NULL);        /*numero de envio auxiliar, id de filtro opcional (NULL puede dar problemas, mejor AL_FILTER_NULL)*/

  if (alGetError() != AL_NO_ERROR)
    printf("Failed to configure Source Send 0 (reverb)\n");



  /*----------------5-ADJUNTANDO_FILTROS_A_LA_FUENTE--------------------*/
  /*Establecemos la manera en que los filtros pueden ser usados en las
    fuentes para filtrar la senyal directa (dry path) y tambien la senyal
    enviada (wet path) tras el filtrado*/


  /*Reverb*/
  //  alSourcei(fuenteConEfecto, AL_DIRECT_FILTER, uiFilter[0]);        /*DRY, viene a ser salida directa*/
  alSourcei(fuenteConEfecto, AL_BUFFER, buffers[0]);
  alSourcei(fuenteConEfecto, AL_LOOPING, AL_FALSE);                /*1=true*/


   
  printf("'h'ellow, 's'ine, sq'u'are, sa'w'tooh, white'n'oise, 'i'mpulse ('q' para salir): \n");   
  
  do
    {
      switch ( c )
	{
	case 'h':  alSourcei (fuente, AL_BUFFER, buffers[0]);
	  alSourcei (fuenteConEfecto, AL_BUFFER, buffers[0]);
	  break;
	     
	case 's': alSourcei (fuente, AL_BUFFER, buffers[1]);
	  alSourcei (fuenteConEfecto, AL_BUFFER, buffers[1]);
	  break;
	case 'u':  alSourcei (fuente, AL_BUFFER, buffers[2]);
	  alSourcei (fuenteConEfecto, AL_BUFFER, buffers[2]);
	  break;
	case 'w':  alSourcei (fuente, AL_BUFFER, buffers[3]);
	  alSourcei (fuenteConEfecto, AL_BUFFER, buffers[3]);
	  break;
	case 'n':  alSourcei (fuente, AL_BUFFER, buffers[4]);
	  alSourcei (fuenteConEfecto, AL_BUFFER, buffers[4]);
	  break;
	case 'i':  alSourcei (fuente, AL_BUFFER, buffers[5]);
	  alSourcei (fuenteConEfecto, AL_BUFFER, buffers[5]);
	  break;   
	}// switch
   
      if (c == 'r' ) {
	alSourcePlay (fuenteConEfecto);

	//
	// No sé com fer variar un paràmetre, me donar error
	for (valor = 0; valor < 3.0; valor+= 0.2)
	  {
	    alutSleep( 1 );
	    printf("valor %f\r", valor); fflush(stdout);
	    //	   alEffecti(uiEffect[0], AL_ECHO_DELAY, valor); 
	    alEffecti(uiEffect[0], AL_REVERB_REFLECTIONS_GAIN, valor); 
	    if (alGetError() != AL_NO_ERROR)
	      printf("Error al establecer AL_REVERB_REFLECTIONS_GAIN\n");

	    // No influeix repetir esta següent asignació
	    alAuxiliaryEffectSloti(uiEffectSlot[0], AL_EFFECTSLOT_EFFECT, uiEffect[0]);

	    alSourcePlay (fuenteConEfecto);
	  }
      }
      else
	alSourcePlay (fuente);
    
      alutSleep (2);
      c = getch();
      alSourceStop( fuenteConEfecto );
      alSourcePlay( fuente );
      printf("%c\r", c); fflush(stdout);
    } while ((c != 'q') && (c != 'Q'));

  printf("\nTerminando ... \n");

  alDeleteSources(1, &fuente);
  alDeleteBuffers(6, buffers);

  alDeleteAuxiliaryEffectSlots(4, uiEffectSlot);
  alDeleteEffects(2, uiEffect);
  alDeleteFilters(1, uiFilter);

  //  alcMakeContextCurrent(NULL);
  alcDestroyContext( pContextEfectos );
  alcCloseDevice( pDevice );
  
  alutExit();
  return EXIT_SUCCESS;
}
 
