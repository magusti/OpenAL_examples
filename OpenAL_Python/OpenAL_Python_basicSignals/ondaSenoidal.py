# ondaSenoidal.py
# Exemple per probar OpenAL en Python sobre PyAL
#
# github https://github.com/JessicaTegner/PyAL
# Doc. <https://pythonhosted.org/PyAL/index.html>
# Instalació pip install python-openal
#
# Comprendiendo las ondas de sonido y las formas de onda
# https://stikeys.co/es/tutorials/understanding-sound-waves-and-waveforms/
#Muhammed ÇELİK. May 10, 2022. How to Generate 440 Hz A(La) Note Sin Wave with 44.1 khz Sample Rate and 16-bit Depth? How to Play Composition with these Generated Note Sin Waves? A Standard 88-key Piano Review. Hapy Birthday DoReMi?
#
#Generate 440Hz sine signal-audio with Python wave library 
#tomazas/python_wave_sample.py
#https://gist.github.com/tomazas/156c616d4fe16327c6c7c5cbc27b04d0
#
# Les formules de 
# Programming with sound
#http://hplgit.github.io/primer.html/doc/pub/diffeq/._diffeq-solarized002.html

from openal import al, alc # imports all relevant AL and ALC functions
import time
#import random
#import ctypes
import math
import matplotlib.pyplot as plt
import numpy as np
import numpy as np
import matplotlib.pyplot as plt

# Similar a alutCreateWaveform( ALUT_WAVEFORM_WHITENOISE, frec, duració, fase)
#SQUARE, 
#ALUT_WAVEFORM_SAWTOOTH
#ALUT_WAVEFORM_IMPULSE
                                        
def createBufferWaveform_sine( frequencia, tamanyMostra, freqMostreig, nSegons, fase ):

  periode = 1/frequencia
  print("onda de frec " + str (frequencia) + " té periode " + str( periode ))
  
  #delta = periode / freqMostreig
  #delta = 1.0 / (freqMostreig * nSegons)
  delta = 1.0 / (freqMostreig)
  
  bufferID = al.ALuint(0) 
  al.alGenBuffers(1, bufferID)
  
  nCanals = 1 # sempre monofonic per poder espacialitzar 
  formatmap = {
        (1, 8) : al.AL_FORMAT_MONO8,
        (2, 8) : al.AL_FORMAT_STEREO8,
        (1, 16): al.AL_FORMAT_MONO16,
        (2, 16) : al.AL_FORMAT_STEREO16,
   }
  alformat = formatmap[(nCanals, tamanyMostra)]
  nElements = int(freqMostreig * nCanals ) * nSegons 
  longitutEnBytes = int(nElements * tamanyMostra/8)
  vector = [None] * nElements
  
  angul = 0
  t = 0
  for i in range(0, nElements):
      # https://docs.python.org/3/library/stdtypes.html#bytes      
      #vector[i] =  int( round(math.sin(math.radians(angul) * 128)+127) )
      #angul = (angul+delta) % 360

      #https://gist.github.com/tomazas/156c616d4fe16327c6c7c5cbc27b04d0
      #valor = 255 * math.sin(math.radians( 2 * math.pi * frequencia * t))
      #ValueError: bytes must be in range(0, 256)
      #valor = 127* math.sin(math.radians( 2 * math.pi * frequencia * t)) + 128
      valor = 127* math.sin( fase + (2 * math.pi * frequencia * t)) + 128 
      #valor = math.sin(math.radians(2 * math.pi * frequencia * t))
      #valor = 127* math.sin(2 * math.pi * frequencia * t) + 128
      
      vector[i] =  int( round(valor) )
      t = t + delta
            
            
  print( "elements " + str(len(vector)) )
  print( "min" + str(min(vector)) + " max " + str(max(vector)) )
  t = np.arange(0, nSegons, 1 / freqMostreig)
  #x = 127* np.sin(2 * np.pi * frequencia * t) + 128
  
  #print( vector )
  # Aixina no el veus: mostra'l  
  plt.ion() # ioff()
  #plt.show()
  plt.plot( vector[0:frequencia]) #(frequencia/nSegons)] )
  #plt.plot(t[t < .01], x[t < .01])
  #plt.plot(t[t < .01], vector[t < .01])

  plt.ylabel("Onda senoidal de frec. " + str( frequencia ) )
  plt.show()
  #plt.pause(0.001)
  plt.pause( 1 )
  fs = 44100

  # https://docs.python.org/3/library/stdtypes.html#bytes
  #al.alBufferData(bufferID, alformat, vector, longitut, freqMostreig)
  #al.alBufferData(bufferID, alformat, bytes(vector), longitutEnBytes, freqMostreig)
  al.alBufferData(bufferID, alformat, bytes(vector), longitutEnBytes, freqMostreig)
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
    
    buf, alformat, longitut, freqMostreig = createBufferWaveform_sine( 440, 8, 11025, 10, 180)
    print("creat buffer de " + str(longitut) + " bytes " + formatCanalsTamanyMostra(alformat) )
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
