/*
 * 
gcc wildmidi_openal.c -o wildmidi_openal -lWildMidi  `pkg-config openal --cflags --libs` && wildmidi_openal midis/bluesIntro.mid 
 * 
 * Extraido de 
 * wildmidi.c -- Midi Player using the WildMidi Midi Processing Library
 *
 * Copyright (C) WildMidi Developers 2001-2016
 *
 */


#include <stdio.h>
#include <stdlib.h>

#   ifndef __APPLE__
#   include <al.h>
#   include <alc.h>
#   else
#   include <OpenAL/al.h>
#   include <OpenAL/alc.h>
#   endif

#include "wildmidi_lib.h"


// struct _midi_test {
//     uint8_t *data;
//     uint32_t size;
// };


// static int midi_test_max = 1;

// static char midi_file[1024];

static unsigned int rate = 32072; //65535; //11050; //22100; //32072;  Interval: 11025 - 65535


#define NUM_BUFFERS 4

static ALCdevice *device;
static ALCcontext *context;
static ALuint sourceId = 0;
static ALuint buffers[NUM_BUFFERS];
static ALuint frames = 0;

// #define open_audio_output open_openal_output

// static void pause_output_openal(void) {
//     alSourcePause(sourceId);
// }

static int write_openal_output(int8_t *output_data, int output_size) {
    ALint processed, state;
    ALuint bufid;

    if (frames < NUM_BUFFERS) { /* initial state: fill the buffers */
        alBufferData(buffers[frames], AL_FORMAT_STEREO16, output_data,
                     output_size, rate);

        /* Now queue and start playback! */
        if (++frames == NUM_BUFFERS) {
            alSourceQueueBuffers(sourceId, frames, buffers);
            alSourcePlay(sourceId);
        }
        return 0;
    }

    /* Get relevant source info */
    alGetSourcei(sourceId, AL_SOURCE_STATE, &state);
    if (state == AL_PAUSED) { /* resume it, then.. */
        alSourcePlay(sourceId);
        if (alGetError() != AL_NO_ERROR) {
            fprintf(stderr, "\nError restarting playback\r\n");
            return (-1);
        }
    }

    processed = 0;
    while (processed == 0) { /* Wait until we have a processed buffer */
        alGetSourcei(sourceId, AL_BUFFERS_PROCESSED, &processed);
    }

    /* Unqueue and handle each processed buffer */
    alSourceUnqueueBuffers(sourceId, 1, &bufid);

    /* Read the next chunk of data, refill the buffer, and queue it
     * back on the source */
    alBufferData(bufid, AL_FORMAT_STEREO16, output_data, output_size, rate);
    alSourceQueueBuffers(sourceId, 1, &bufid);
    if (alGetError() != AL_NO_ERROR) {
        fprintf(stderr, "\nError buffering data\r\n");
        return (-1);
    }

    /* Make sure the source hasn't underrun */
    alGetSourcei(sourceId, AL_SOURCE_STATE, &state);
    /*printf("STATE: %#08x - %d\n", state, queued);*/
    if (state != AL_PLAYING) {
        ALint queued;

        /* If no buffers are queued, playback is finished */
        alGetSourcei(sourceId, AL_BUFFERS_QUEUED, &queued);
        if (queued == 0) {
            fprintf(stderr, "\nNo buffers queued for playback\r\n");
            return (-1);
        }

        alSourcePlay(sourceId);
    }

    return (0);
}

static void close_openal_output(void) {
    if (!context) return;
    printf("Shutting down sound output\r\n");
    alSourceStop(sourceId);         /* stop playing */
    alSourcei(sourceId, AL_BUFFER, 0);  /* unload buffer from source */
    alDeleteBuffers(NUM_BUFFERS, buffers);
    alDeleteSources(1, &sourceId);
    alcDestroyContext(context);
    alcCloseDevice(device);
    context = NULL;
    device = NULL;
    frames = 0;
}

static int open_openal_output(void) {
    /* setup our audio devices and contexts */
    device = alcOpenDevice(NULL);
    if (!device) {
        fprintf(stderr, "OpenAL: Unable to open default device.\r\n");
        return (-1);
    }

    context = alcCreateContext(device, NULL);
    if (context == NULL || alcMakeContextCurrent(context) == ALC_FALSE) {
        if (context != NULL)
            alcDestroyContext(context);
        alcCloseDevice(device);
        context = NULL;
        device = NULL;
        fprintf(stderr, "OpenAL: Failed to create the default context.\r\n");
        return (-1);
    }
    /* setup our sources and buffers */
    alGenSources(1, &sourceId);
    alGenBuffers(NUM_BUFFERS, buffers);

    return (0);
}


static char config_file[1024];

int main(int argc, char **argv) {
    struct _WM_Info *wm_info;
    int i, res = 0;
    uint16_t mixer_options = 0;
    void *midi_ptr = 0;
    uint8_t master_volume = 100;
    int8_t *output_buffer;
    uint32_t perc_play;
    uint32_t pro_mins;
    uint32_t pro_secs;
    uint32_t apr_mins;
    uint32_t apr_secs;
    static char spinner[] = "|/-\\";
    static int spinpoint = 0;
    uint32_t samples = 0;
    char * ret_err = NULL;
    long libraryver;

   
    if (argc == 0) {
        fprintf(stderr, "ERROR: No midi file given\r\nUtilitzar: %s nomFitxer.mid", argv[0]);
        return( 1 );
    }



    printf("Initializing Sound System\n");
    if (open_openal_output() == -1) {
        printf("No es pot obrir OpenAL\n");
        return (1);
    }


//     libraryver = WildMidi_GetVersion();
//     printf("Initializing libWildMidi %ld.%ld.%ld\n\n",
//                         (libraryver>>16) & 255,
//                         (libraryver>> 8) & 255,
//                         (libraryver    ) & 255);
//        printf("Provant libWildMidi %ld.%ld.%ld == %ld\n", LIBWILDMIDI_VER_MAJOR, LIBWILDMIDI_VER_MINOR, LIBWILDMIDI_VER_MICRO,
//     WildMidi_GetVersion () );
      printf("Provant libWildMidi %ld.%ld.%ld\n", LIBWILDMIDI_VER_MAJOR, LIBWILDMIDI_VER_MINOR, LIBWILDMIDI_VER_MICRO);

//     printf("config_file %s, rate %d mixer_options %X\n", config_file, rate, mixer_options);
// //     if (WildMidi_Init(config_file, rate, mixer_options) == -1) {
     if (WildMidi_Init("/etc/wildmidi/wildmidi.cfg", rate, mixer_options) == -1) {
        fprintf(stderr, "%s\r\n", WildMidi_GetError());
        WildMidi_ClearError();
        return (1);
    }

    samples = 16384; // == 16K == 16*1024    Des de 4*1024 s'escolta bé
// //      samples = 512*1024; //128*1024; //64*1024; // 32*1024 = 32768;  A 1024*1024 no es pot dona error.    
    output_buffer = (int8_t *) malloc(16384);
    if (output_buffer == NULL) {
        fprintf(stderr, "Not enough memory, exiting\n");
        WildMidi_Shutdown();
        return (1);
    }

    WildMidi_MasterVolume(master_volume);
   
    WildMidi_ClearError();
    printf("Reproduint %s\n", argv[1] );
    midi_ptr = WildMidi_Open(argv[1]);
    if (midi_ptr == NULL) {
                ret_err = WildMidi_GetError();
                //printf(" No es pot llegir %s. Error:  %s\r\n",argv[1], ret_err);
                printf(" Error:  %s\r\n", ret_err);
                close_openal_output();                
                free(output_buffer);
                return 3;
    }
    printf("midi_ptr %p\n", midi_ptr);

     wm_info = WildMidi_GetInfo(midi_ptr);
        //
//      printf("Propietats \n copyright: %s\n current_sample %d\n\
// approx_total_samples %d\n mixer_options %d\n total_midi_time %d\n", 
//         wm_info->copyright, wm_info->current_sample, wm_info->approx_total_samples, 
//         wm_info->mixer_options, wm_info->total_midi_time );
        //
     apr_mins = wm_info->approx_total_samples / (rate * 60);
     apr_secs = (wm_info->approx_total_samples % (rate * 60)) / rate;
     mixer_options = wm_info->mixer_options;
     printf("\r\n[Approx %2um %2us Total]\r\n", apr_mins, apr_secs);
     fprintf(stderr, "\r");

//      samples = wm_info->approx_total_samples;
//      printf("Precarga ... (considerant %u samples\n", samples );
//     output_buffer = (int8_t *) malloc(samples);
//     if (output_buffer == NULL) {
//         fprintf(stderr, "Not enough memory, exiting\n");
//         WildMidi_Shutdown();
//         return (1);
//     }     
//      printf("Precarga: GetOutput ...\n" );     
//      res = WildMidi_GetOutput(midi_ptr, output_buffer, samples);
//      printf("Precarga: OpenAL ...\n" );     
//      if (res > 0)
//          write_openal_output(output_buffer, res); 
   

     printf("Streaming ...\n");
//      samples = 16384; // == 16K == 16*1024    Des de 4*1024 s'escolta bé
// //      samples = 512*1024; //128*1024; //64*1024; // 32*1024 = 32768;  A 1024*1024 no es pot dona error.
     printf("Samples = %d\n", samples);
     while ( res > -1) { // (1)
//             ch = 0;
//         samples = 16384;    
            res = WildMidi_GetOutput(midi_ptr, output_buffer, samples);
            if (res <= 0)
                break;

            wm_info = WildMidi_GetInfo(midi_ptr);

            perc_play = (wm_info->current_sample * 100) / wm_info->approx_total_samples;
            pro_mins = wm_info->current_sample / (rate * 60);
            pro_secs = (wm_info->current_sample % (rate * 60)) / rate;
           fprintf(stderr,
                "[%2um %2us Processed] [%2u%%] %c  \r",
                pro_mins,
                pro_secs, perc_play, spinner[spinpoint++ % 4]);

//             if (send_output(output_buffer, res) < 0) { // driver prints an error message already.
            if (write_openal_output(output_buffer, res) < 0) { // driver prints an error message already.                
                printf("\r");
                //goto end2;
            }
     } // while (res > -1)
     
     if (WildMidi_Close(midi_ptr) == -1) {
            ret_err = WildMidi_GetError();
            fprintf(stderr, "OOPS: failed closing midi handle!\r\n%s\r\n",ret_err);
     }

//     close_output();
    close_openal_output();

    free(output_buffer);
    if (WildMidi_Shutdown() == -1) {
        ret_err = WildMidi_GetError();
        fprintf(stderr, "OOPS: failure shutting down libWildMidi\r\n%s\r\n", ret_err);
        WildMidi_ClearError();
     }

    printf("\r\n");
    return (0);
} // main
