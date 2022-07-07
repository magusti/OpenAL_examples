<h1>OpenAL examples</h1>
This repository must be considered a contribution to the community of OpenAL developers and my own way to get back the lessons that I have been learning from others.

"OpenAL examples" is a repository of examples of C source code that uses OpenAL in several ways:
<ul>
 <li>You can find approaches to using other audio formats than WAVE files as input audio to OpenAL.</li>
 <li>You can find an explanation of OpenAL concepts like static vs streaming techniques for playing audio files.</li>
 <li>Also, you can get basic visualizations related to audio waveform: I enjoy painting the audio waveform and the frequency spectrum to help visualize the relationship between the digital audio samples in time and frequency domains and the sound you can hear.</li> 
</ul>


The code has documentation (in Spanish) that remarks on the sources used to build the examples or from where I obtained them. The idea behind each use case and a little explanation of the code. Also, you can find a way to compile these examples from a Linux terminal. 

I tried to explicitly put the references from where I got the inspiration or the exact example that I reproduced, but if you miss one reference, please do not hesitate to send me an email.
<ul>
 <li> (2011). Introducción al procesado de audio mediante OpenAL. &lt;&lt;http://hdl.handle.net/10251/12694 &gt;. Keywords:
Procesado de audio , Openal

<li> (2011). Efectos de audio básicos mediante OpenAL. &lt;http://hdl.handle.net/10251/12696&gt;. Keywords:
Procesado de audio , Openal

<li> (2011). Introducción al empleo de técnicas de audio posicional mediante OpenAL. &lt;http://hdl.handle.net/10251/12697 &gt;. Keywords:
 <li> Audio posicional , Audio envolvente , Audio espacial , Audio 3d , Procesado de audio , Openal

 
 <li> (2012). Uso del micrófono para captura de audio en OpenAL. &lt;http://hdl.handle.net/10251/17547 &gt;. Keywords:
 Captura de audio , Adquisición de audio , Micrófono , Openal
 
 
 <li> (2018). Extendiendo OpenAL con OGG Vorbis. &lt;http://hdl.handle.net/10251/109210 &gt;. Keywords:
 Ogg Vorbis , OpenAL , importar Vorbis , PCM , audio comprimido.

 <li> (2018). Extendiendo OpenAL con SDL. Caso de estudio MP3. &lt;http://hdl.handle.net/10251/105383 &gt;. Keywords:
OpenAL , SDL , MP3

 <li> (2018). Extensiones para OpenAL: efectos ambientales. &lt;http://hdl.handle.net/10251/105664 &gt;. Keywords:
 OpenAL , Extensions , EFX , simulación del entorno , efectos ambientales , Reverberación , Reverb.

 <li> (2018). OpenAL y OpenGL: escuchar y ver el sonido. &lt;http://hdl.handle.net/10251/105550 &gt;. Keywords:
 Forma de ondas , pintar el sonido , PCM , OpenAL , OpenGL.

 <li> (2018). OpenAL: efecto Doppler. Posicionamiento y velocidad del sonido. &lt;http://hdl.handle.net/10251/104052 &gt;. Keywords:
Efecto Doppler. , OpenAL , posicionamiento 3D audio , velocidad del sonido.

 <li> (2018). Reproducción de ficheros Opus con OpenAL: precarga vs "streaming". &lt;http://hdl.handle.net/10251/109211 &gt;. Keywords:
 Opus , OpenAL , importar Opus , PCM , audio comprimido , precarga , streaming.

 
 <li> (2021). Extendiendo OpenAL con ficheros MP3 y libMAD. &lt;http://hdl.handle.net/10251/170185  &gt;. Keywords:
Formatos de audio , Formatos MP3 , Librería libmad , MP3 , OpenAL , Importar clip de audio en MP3.

 <li> (2021). Reproducción de ficheros FLAC con OpenAL y dr_flac. &lt;http://hdl.handle.net/10251/170187 &gt;. Keywords:
Free Lossless Audio Codec (FLAC) , Ficheros FLAC , FLAC , OpenAL , Importar clip de audio en FLAC , Dr_flac.

 <li> (2021). Reproducción de ficheros MIDI con OpenAL. &lt;http://hdl.handle.net/10251/170183 &gt;. Keywords:
 Formatos audio , formato MIDI , Musical Instrument Digital Interface (MIDI) , WildMIDI library , MIDI , OpenAL , Importar MIDI.

 <li>OpenAL_midi_libwildmidi: (2021). Reproducción de ficheros MIDI con OpenAL. &lt;http://hdl.handle.net/10251/170183 &gt;. Keywords:
  Formatos audio , formato MIDI , Musical Instrument Digital Interface (MIDI) , WildMIDI library , MIDI , OpenAL , Importar MIDI. 
  
  
 <!-- <li> nomDelSubdirectori: Referència a riunet &lt; URL &gt;. Keywords: paraules clau.</li> -->
 
 <li> (2022). Extendiendo OpenAL con ficheros MP3 y libmpg123. &lt;http://hdl.handle.net/10251/183758 &gt;. Keywords: mpg123, MP3, libmpg123, OpenAL.</li>
 
 <li> OpenAL_libsndfile_preload: (2022). </li>
 <li> Agustí Melchor, M. (2022). OpenAL: usando libsndfile para reproducción en streaming de ficheros de audio. Universitat Politècnica de València. &lt;http://hdl.handle.net/10251/183656 &gy;. Keywords: libsndfile, OpenAL, Streaming, Reproducir audio en continuo.  </li>
 
 <li> OpenAL_drawingFFT: (2023?). </li>
  
</ul>


Thanks
 To Loki Soft for create and distribute OpenAL for free. I hope it will become as free as it deserves. And also by the "OpenAL Specification and Reference" &lt;https://www.openal.org/documentation/ &gt;.

 To Garin Hiebert, Garin Hiebert, Peter Harrison and Daniel Peacock for the OpenAL documentation:  "OpenAL Programmer's Guide" &lt;https://www.openal.org/documentation/ &gt;.

 To Steve Baker and Sven Panne <http://distro.ibiblio.org/rootlinux/rootlinux-ports/more/freealut/freealut-1.1.0/doc/alut.html#alutGetMIMETypes> for the "OpenAL Utility Toolkit (ALUT) Reference Manual".

 To vancegroup <https://github.com/vancegroup/freealut/> for implementing "freealut" <http://distro.ibiblio.org/rootlinux/rootlinux-ports/more/freealut/freealut-1.1.0/doc/alut.html>.

 To Daniel Peacock, Peter Harrison, Andrea D’Orta, Valery Carpentier and Edward Cooper for the "Effects Extension Guide" &lt;https://usermanual.wiki/Pdf/Effects20Extension20Guide.90272296/html &gt;.

 To Ryan A. Pavlik <https://github.com/rpavlik/openal-svn-mirror>  for publish the original fork of OpenAL.

 
 Thanks to the community of persons that maintain the OpenAL website  &lt;http://www.openal.org&gt;, the "openal mailing list" &lt;https://openal.org/mailman/listinfo/openal&gt;, and to everybody that ask and answer in the "openal mailing list".

 And especially to Chris Robinson by his OpenAL Soft &lt;https://openal-soft.org/&gt; / &lt;https://github.com/kcat/openal-soft&gt; that makes possible that everyone can use this standard and for publish it under a LGPL license. Also for ALURE %gt;https://github.com/kcat/alure&lt;, and also for his expert and kind words in emails.




M. Agustí (2021-2022). magusti at disca.upv.es
