/*
 * (C) M. Agustí. Nov. 2k8 - 2k17
 *
 * Compilar:
 * gcc helloFitxer.c  -lalut -lopenal -o helloFitxer 
 * o
 * gcc helloFitxer.c -o helloFitxer `pkg-config freealut --cflags --libs`

 * 
 * Ejecutar
 * helloFitxer  /usr/share/sounds/KDE_Startup.wav
 * for i in /usr/share/sounds/*.wav; do echo $i; helloFitxer $i; done

 *
 $ find  /usr/share/sounds/ -iname "*.wav" -exec helloFitxer  {} \;
 */

#include <stdio.h> // Per els printf
#include <stdlib.h>
#include <AL/alut.h>
 
int main (int argc, char **argv)
{
  ALuint buffer, fuente;
  int error;
  ALint sourceState;

  if (argc == 1)
  {
   printf("Para ejecutar hace falta pasar como parámetro\
un nombre de fichero de audio en formato soportado por ALUT\n\n\
Por ejemplo: %s nombreFicheroWAVE\n", argv[0]);
   return 1;
  }
  else
  {
   alutInit (&argc, argv);


   printf("Reproduint %s ...\n", argv[1]);
 
   buffer = alutCreateBufferFromFile( argv[1] );
   error = alGetError(); 
    if (error) printf("%s\n", alutGetErrorString(error));

   alGenSources (1, &fuente);
   alSourcei(fuente, AL_BUFFER, buffer);

   alSourcePlay (fuente);

   // Mentres n'hi haja datos que no pare
   alGetSourcei( fuente, AL_SOURCE_STATE, &sourceState);
   while (sourceState == AL_PLAYING)
    alGetSourcei( fuente, AL_SOURCE_STATE, &sourceState);


   alDeleteSources(1, &fuente);
   alDeleteBuffers(1, &buffer);
   alutExit ();
   return EXIT_SUCCESS;

  } // if (argc == 1)

}// main
