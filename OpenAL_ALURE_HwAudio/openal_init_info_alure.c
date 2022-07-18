/*
 * gcc openal_init_info_alure.c -o openal_init_info_alure $(pkg-config alure openal --cflags --libs)
 */
#include <stdlib.h>
#include <stdio.h>
#include <AL/alure.h>
#include <string.h>
#include <AL/al.h>
#include <AL/alext.h>


int main(int argc, char **argv)
{
  ALuint ALURE_major, ALURE_minor;
  int i, contaor;
  const ALCchar **pDeviceList, *llista; // Tammé const ALchar ** i const char** 
  ALuint elBuffer, laFont;
  ALenum error;
  
  if(!alureInitDevice(NULL, NULL))
  {
        fprintf(stderr, "ALURE:: Failed to open OpenAL device: %s\n", alureGetErrorString());
        return( -1);
  }

  alureGetVersion(&ALURE_major, &ALURE_minor);
  printf ("La versio de ALURE instalada es %d.%d \n", ALURE_major, ALURE_minor);

  printf("OpenAL Renderer is '%s'\n", alGetString(AL_RENDERER) );
  printf("OpenAL Version is '%s'\n", alGetString(AL_VERSION) );		    
  printf("OpenAL Vendor is '%s'\n", alGetString(AL_VENDOR) );
  //
  printf("OpenAL Extensions supported are :\n%s\n", alGetString(AL_EXTENSIONS) );
 
  pDeviceList = alureGetDeviceNames( AL_TRUE, &contaor );
//  Gets an array of device name strings from OpenAL.  This encapsulates AL_ENUMERATE_ALL_EXT (if supported and ‘all’ is true) and standard enumeration, with ‘count’ being set to the number of returned device names.
    if( contaor == 0 ) //pDeviceList)
    { 
        printf(" ALURE:: error al buscar dispositius d'audio\n");
        return( -2 );
        
    }else{
      printf("\nDispositius? disponibles d'audio: %d\n", contaor);
      printf("LListar\n");         
      for( i= 0; i < contaor; i++)
        {
          printf("%2d - %s\n", i, pDeviceList[i]);
        }
    }
       

      printf("initDevice\n");    
      for( i= 0; i < contaor; i++)
      {
          printf("%2d - %s\n", i, pDeviceList[i]);
          if( alureInitDevice( pDeviceList[i], NULL ) == AL_FALSE )
          { 
           printf(" ALURE:: error al inicialitzar dispositiu %s.\n", pDeviceList[i]);
//            return( -3 );        
          }
          else printf("Init : %s\n", pDeviceList[i]); 
              
      }
      alureFreeDeviceNames( pDeviceList );
    
      printf("Inicialitzar dispositiu per defecte\n");     
      if( alureInitDevice( NULL, NULL ) == AL_FALSE )  { 
        printf(" ALURE:: error al inicialitzar dispositiu per defecte.\n");
        return( -4 );        
       } else printf("Inicialitzat disp. per defecte \n"); 

   
   // Ací podriem comprobar que sona alguna cosa ...    
    
    
    if( alureShutdownDevice() == AL_FALSE )
    { 
       printf(" ALURE:: error al lliberar el dispositiu.\n" );
       return( -5 );        
    }    
   alureFreeDeviceNames( pDeviceList );


   return 0;
}
