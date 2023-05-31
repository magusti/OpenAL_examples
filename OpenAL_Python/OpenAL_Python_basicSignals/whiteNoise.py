# whiteNoise.py
# Exemple per probar OpenAL en Python sobre PyAL
#
# github https://github.com/JessicaTegner/PyAL
# Doc. <https://pythonhosted.org/PyAL/index.html>
# Instalació pip install python-openal
#
# Comprendiendo las ondas de sonido y las formas de onda
# https://stikeys.co/es/tutorials/understanding-sound-waves-and-waveforms/

from openal import al, alc # imports all relevant AL and ALC functions
import time
import random
#import ctypes

# Similar a alutCreateWaveform( ALUT_WAVEFORM_WHITENOISE, frec, duració, fase)
def createBufferWaveform_whiteNoise( tamanyMostra, freqMostreig, nSegons ):
  bufferID = al.ALuint(0) 
  al.alGenBuffers(1, bufferID)
  
  #vector = []
  #tamanyMostra = 8 # bits
  nCanals = 1 # sempre monofonic per poder espacialitzar
  formatmap = {
        (1, 8) : al.AL_FORMAT_MONO8,
        (2, 8) : al.AL_FORMAT_STEREO8,
        (1, 16): al.AL_FORMAT_MONO16,
        (2, 16) : al.AL_FORMAT_STEREO16,
   }
  alformat = formatmap[(nCanals, tamanyMostra)]
    #alformat = al.AL_FORMAT_MONO8 # encoding of the data part (8-bit ISDN u-law)
   #freqMostreig = 11025 # number of samples per second 
   #nCanals = 1 # number of interleaved channels
  nElements = int(freqMostreig * nCanals ) * nSegons 
  longitutEnBytes = int(nElements * tamanyMostra/8)
  vector = [None] * nElements
  random.seed()
  for i in range(0, nElements):
      #vector[i] =  random.randbytes(1) # v 3,9???
      vector[i] =  random.randrange(0, 256, 1)
  print( len(vector) )
  print( vector )
  #vector = random.randbytes(longitut)
  #printf( len(vector) )
  
  # https://docs.python.org/3/library/stdtypes.html#bytes
  al.alBufferData(bufferID, alformat, bytes(vector), longitutEnBytes, freqMostreig)
  #
  #array_type = ctypes.c_void_p * len(vector)
  #al.alBufferData(bufferID, alformat, array_type(*vector), longitut, freqMostreig)
  #al.alBufferData(bufferID, alformat, bytearray(helloWorldSample), longitut, freqMostreig)
  return bufferID, alformat, longitutEnBytes, freqMostreig



def formatCanalsTamanyMostra(alformat):
    if alformat == al.AL_FORMAT_MONO8:
        return "AL_FORMAT_MONO8"
    else:
        if alformat == al.AL_FORMAT_STEREO8:
          return "AL_FORMAT_STEREO8"
        else:
          if alformat == al.AL_FORMAT_MONO16:
           return "AL_FORMAT_MONO16"
          else:
           if alformat == al.AL_FORMAT_STEREO16:
            return "AL_FORMAT_STEREO16"
    
        


import sys
import os

def main():

        
    source = al.ALuint()
    # alutInit
    device = alc.alcOpenDevice(None)
    if not device:
        error = alc.alcGetError()
        # do something with the error, which is a ctypes value
        return -1
    # Omit error checking
    context = alc.alcCreateContext(device, None)
    if not context:
        error = alc.alcGetError()
        return -2
    
    resultat = alc.alcMakeContextCurrent(context)
    if not resultat:
         alc.alcDestroyContext(context)
         alc.alcCloseDevice(device)
         return -3
     #fi de alutInit
    
    # Do more things
    al.alGenSources(1, source)
    al.alSourcef(source, al.AL_PITCH, 1)
    al.alSourcef(source, al.AL_GAIN, 1)
    al.alSource3f(source, al.AL_POSITION, 10, 0, 0)
    al.alSource3f(source, al.AL_VELOCITY, 0, 0, 0)
    al.alSourcei(source, al.AL_LOOPING, 1)
    
    
    #buf = al.ALuint(0)
    #al.alGenBuffers(1, buf)
    #al.alBufferData(buf, alformat, wavbuf, len(wavbuf), samplerate)
    
    #buf, alformat, longitut, freqMostreig = createBufferHelloWorld()
    
    buf, alformat, longitutEnBytes, freqMostreig = createBufferWaveform_whiteNoise( 8, 11025, 10 )
    print("creat buffer de " + str(longitutEnBytes) + " bytes " + formatCanalsTamanyMostra(alformat) )
    #??? No funciona?? al.alSourcei(source, al.AL_BUFFER, buf) #ctypes.ArgumentError: argument 3: <class 'TypeError'>: wrong type
    al.alSourceQueueBuffers(source, 1, buf)
    
    #source.play()
    al.alSourcePlay(source)
    #time.sleep( 10 )
    #al.alSleep( 10 )
    state = al.ALint(0)
    al.alGetSourcei(source, al.AL_SOURCE_STATE, state)
    z = 10
    while z > -10:
        time.sleep(1)
        al.alSource3f(source, al.AL_POSITION, z, 0, 0)
        print("playing at %r" % ([z, 0, 0]))
        z -= 1
    print("done")
    
    
    al.alDeleteSources(1, source)
    #alutExit
    alc.alcDestroyContext(context)
    alc.alcCloseDevice(device)
    #fi de alutExit
    
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
