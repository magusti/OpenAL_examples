/*
 * $ gcc -o grabarAudio3D grabarAudio3D.c -lopenal -lsndfile -lm
 * $ grabarAudio3D Front_Center.wav 
 */


#include <AL/al.h>
#include <AL/alc.h>
#include <AL/alext.h>
#include <sndfile.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#define SAMPLE_RATE 48000
#define BUFFER_SAMPLES 4800
#define DURATION_SECONDS 10

// Función para cargar un archivo de audio
ALuint cargar_audio(const char *filename) {
    SF_INFO sfinfo;
    SNDFILE *file = sf_open(filename, SFM_READ, &sfinfo);
    if (!file) return 0;

    ALsizei size = sfinfo.frames * sfinfo.channels;
    ALshort *data = malloc(size * sizeof(ALshort));
    sf_read_short(file, data, size);
    sf_close(file);

    ALenum format = (sfinfo.channels == 1) ? AL_FORMAT_MONO16 : AL_FORMAT_STEREO16;

    ALuint buffer;
    alGenBuffers(1, &buffer);
    alBufferData(buffer, format, data, size * sizeof(ALshort), sfinfo.samplerate);

    free(data);
    return buffer;
}

int main(int argc, char *argv[]) {

    if (argc < 2) {
        printf("Falta argument: %s <artxiuAUDIO>\n", argv[0]);
        exit(1);
    }
    
    //
    // // Inicialitzar OpenAL --> No cap a l'article: abrevia! No es podrà sentir mentres es grava!!!
    //
    
    // Punteros a funciones de la extensión
    static LPALCLOOPBACKOPENDEVICESOFT alcLoopbackOpenDeviceSOFT;
    static LPALCISRENDERFORMATSUPPORTEDSOFT alcIsRenderFormatSupportedSOFT;
    static LPALCRENDERSAMPLESSOFT alcRenderSamplesSOFT;

    // Carregar les funcions de l'extensió
    alcLoopbackOpenDeviceSOFT = (LPALCLOOPBACKOPENDEVICESOFT)
                                alcGetProcAddress(NULL, "alcLoopbackOpenDeviceSOFT");
    alcIsRenderFormatSupportedSOFT = (LPALCISRENDERFORMATSUPPORTEDSOFT)
                                alcGetProcAddress(NULL, "alcIsRenderFormatSupportedSOFT");
    alcRenderSamplesSOFT = (LPALCRENDERSAMPLESSOFT)
                                alcGetProcAddress(NULL, "alcRenderSamplesSOFT");    

    if (!alcLoopbackOpenDeviceSOFT || !alcRenderSamplesSOFT) {
        fprintf(stderr, "Error: Extensión ALC_SOFT_loopback no disponible\n");
        return 1;
    }

    // Configurar dispositiu "loopback"
    ALCdevice *device = alcLoopbackOpenDeviceSOFT(NULL);
    if (!device) {
        fprintf(stderr, "Error: dispositivo loopback no disponible\n");
        return 1;
    }

    ALCint attrs[] = {
        ALC_FORMAT_CHANNELS_SOFT, ALC_STEREO_SOFT,
        ALC_FORMAT_TYPE_SOFT, ALC_SHORT_SOFT,
        ALC_FREQUENCY, SAMPLE_RATE,
        0 // NULL?
    };

    ALCcontext *context = alcCreateContext(device, attrs);
    alcMakeContextCurrent(context);

    // Configurar l'oient (listener)
    alListener3f(AL_POSITION, 0.0f, 0.0f, 0.0f);
    ALfloat listenerOri[] = {0.0f, 0.0f, -1.0f, 0.0f, 1.0f, 0.0f};
    alListenerfv(AL_ORIENTATION, listenerOri);

    // Cargar y configurar fuente de audio
    ALuint buffer = cargar_audio(argv[1]); //"sonido.wav");  // Asegúrate de tener este archivo
    if (buffer == 0) {
        fprintf(stderr, "Error: no se pudo cargar sonido.wav\n");
        return 1;
    }

    ALuint source; // La única font d'audio
    alGenSources(1, &source);
    alSourcei(source, AL_BUFFER, buffer);
    alSourcei(source, AL_LOOPING, AL_TRUE);
    alSourcef(source, AL_REFERENCE_DISTANCE, 1.0f);
    alSource3f(source, AL_POSITION, 2.0f, 0.0f, 0.0f);  // Posición inicial a la derecha

    // alSourcePlay(source); -> No cap a l'article: abrevia!

    // Configurar archivo de salida
    SF_INFO sfinfo = {0};
    sfinfo.samplerate = SAMPLE_RATE;
    sfinfo.channels = 2;
    sfinfo.format = SF_FORMAT_WAV | SF_FORMAT_PCM_16;

    SNDFILE *outfile = sf_open("grabacion_3d.wav", SFM_WRITE, &sfinfo);
    if (!outfile) {
        fprintf(stderr, "Error al crear archivo WAV\n");
        return 1;
    }

    // Buffer de captura
    short *capture_buffer = malloc(BUFFER_SAMPLES * 2 * sizeof(short));

    // Grabar mientras movemos la fuente en círculo
    int total_samples = SAMPLE_RATE * DURATION_SECONDS;
    int samples_rendered = 0;
    float angle = 0.0f;

    printf("Grabando %d segundos con fuente rotando alrededor del oyente...\n", DURATION_SECONDS);

    while (samples_rendered < total_samples) {
        // Mover la fuente en círculo
        float radius = 3.0f;
        float x = radius * cosf(angle);
        float z = radius * sinf(angle);
        alSource3f(source, AL_POSITION, x, 0.0f, z);

        angle += 0.05f;  // Velocidad de rotación

        // Renderizar audio
        alcRenderSamplesSOFT(device, capture_buffer, BUFFER_SAMPLES);

        // Escribir a archivo
        sf_writef_short(outfile, capture_buffer, BUFFER_SAMPLES);

        samples_rendered += BUFFER_SAMPLES;

        if (samples_rendered % SAMPLE_RATE == 0) {
            printf("  %d/%d segundos\n", samples_rendered / SAMPLE_RATE, DURATION_SECONDS);
        }
    }

    printf("Grabación completa: grabacion_3d.wav\n");

    // Limpieza
    free(capture_buffer);
    sf_close(outfile);
    
    alSourceStop(source);
    alDeleteSources(1, &source);
    alDeleteBuffers(1, &buffer);
    
    alcMakeContextCurrent(NULL);
    alcDestroyContext(context);
    alcCloseDevice(device);

    return 0;
}
