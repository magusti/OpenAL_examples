//
// gcc visualizarFFT_OpenAL_SDL__minim.c -o visualizarFFT_OpenAL_SDL__minim -lopenal -lSDL2 -lfftw3 -lsndfile -lm && visualizarFFT_OpenAL_SDL__minim preguntant_IA/drHouse_mono.wav
//

#define _USE_MATH_DEFINES
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <SDL2/SDL.h>
#include <AL/al.h>
#include <AL/alc.h>
#include <sndfile.h>
#include <fftw3.h>

// Definir M_PI si no está disponible
#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

#define WINDOW_WIDTH 800
#define WINDOW_HEIGHT 600
#define SAMPLE_RATE 44100
#define BUFFER_SIZE 1024
#define FFT_SIZE 512

typedef struct {
    SDL_Window *window;
    SDL_Renderer *renderer;
    SDL_Texture *fft_texture;
    SDL_bool running;
    int window_width;
    int window_height;
} App;

typedef struct {
    ALCdevice *device;
    ALCcontext *context;
    ALuint source;
    ALuint *buffers;
    int num_buffers;
    SNDFILE *sndfile;
    SF_INFO sfinfo;
    float *audio_data;
    size_t audio_samples;
    size_t current_sample;
    SDL_bool playing;
} AudioSystem;

typedef struct {
    fftw_complex *in;
    fftw_complex *out;
    fftw_plan plan;
    float *magnitudes;
} FFTSystem;


// Inicializar SDL 
int init_sdl(App *app) {
    // Inicializar SDL con video
    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        fprintf(stderr, "Error al inicializar SDL: %s\n", SDL_GetError());
        return 0;
    }

    // Crear ventana redimensionable
    app->window = SDL_CreateWindow("Visualizador FFT ( OpenAL + FFTW + SDL )",
                                   SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
                                   WINDOW_WIDTH, WINDOW_HEIGHT,
                                   SDL_WINDOW_SHOWN | SDL_WINDOW_RESIZABLE);
    if (!app->window) {
        fprintf(stderr, "Error al crear ventana: %s\n", SDL_GetError());
        return 0;
    }

    // Crear renderer con aceleración por hardware si está disponible
    app->renderer = SDL_CreateRenderer(app->window, -1,
                                       SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
    // Información del renderer
    SDL_RendererInfo info;
    if (SDL_GetRendererInfo(app->renderer, &info) == 0) {
        printf("Renderer: %s\n", info.name);
    }

    app->running = SDL_TRUE;
    app->window_width = WINDOW_WIDTH;
    app->window_height = WINDOW_HEIGHT;
    app->fft_texture = NULL;

    return 1;
}

// Inicializar OpenAL
int init_openal(AudioSystem *audio) {
    audio->device = alcOpenDevice(NULL);
    if (!audio->device) {
        fprintf(stderr, "Error al abrir dispositivo OpenAL\n");
        return 0;
    }

    audio->context = alcCreateContext(audio->device, NULL);
    if (!audio->context) {
        fprintf(stderr, "Error al crear contexto OpenAL\n");
        return 0;
    }

    alcMakeContextCurrent(audio->context);

    alGenSources(1, &audio->source);
   
    // Configurar 3 buffers para streaming
    audio->num_buffers = 3;
    audio->buffers = malloc(audio->num_buffers * sizeof(ALuint));
    alGenBuffers(audio->num_buffers, audio->buffers);

    audio->playing = SDL_FALSE;
    audio->current_sample = 0;

    return 1;
}

// Inicializar sistema FFT con suavizado
int init_fft(FFTSystem *fft) {
    fft->in = fftw_malloc(sizeof(fftw_complex) * FFT_SIZE);
    fft->out = fftw_malloc(sizeof(fftw_complex) * FFT_SIZE);
    fft->magnitudes = malloc(sizeof(float) * FFT_SIZE / 2);
   
    if (!fft->in || !fft->out || !fft->magnitudes ) {
        fprintf(stderr, "Error al asignar memoria para FFT\n");
        
        return 0;
    }

    fft->plan = fftw_plan_dft_1d(FFT_SIZE, fft->in, fft->out, FFTW_FORWARD, FFTW_ESTIMATE);

    return 1;
}

// Cargar archivo de audio
int load_audio_file(const char *filename, AudioSystem *audio) {
    audio->sndfile = sf_open(filename, SFM_READ, &audio->sfinfo);
    if (!audio->sndfile) {
        fprintf(stderr, "Error al abrir archivo: %s\n", sf_strerror(NULL));
        return 0;
    }

    printf("Archivo cargado:\n");
    printf("  Canales: %d\n", audio->sfinfo.channels);
    printf("  Sample rate: %d\n", audio->sfinfo.samplerate);
    printf("  Frames: %ld\n", audio->sfinfo.frames);
    printf("  Duración: %.2f segundos\n", (float)audio->sfinfo.frames / audio->sfinfo.samplerate);

    // Cargar todos los datos de audio en memoria
    audio->audio_samples = audio->sfinfo.frames * audio->sfinfo.channels;
    audio->audio_data = malloc(audio->audio_samples * sizeof(float));
   
    if (!audio->audio_data) {
        fprintf(stderr, "Error al asignar memoria para audio\n");
        return 0;
    }

    sf_read_float(audio->sndfile, audio->audio_data, audio->audio_samples);
   
    return 1;
}

// Llenar buffer de OpenAL con datos de audio
void fill_buffer(AudioSystem *audio, ALuint buffer) {
    const size_t samples_per_buffer = BUFFER_SIZE;
    float *buffer_data = malloc(samples_per_buffer * sizeof(float));
   
    // Convertir a mono si es necesario y copiar datos
    for (size_t i = 0; i < samples_per_buffer; i++) {
        if (audio->current_sample >= audio->audio_samples) {
            buffer_data[i] = 0.0f; // Silencio al final
        } else {
            if (audio->sfinfo.channels == 1) {
                buffer_data[i] = audio->audio_data[audio->current_sample];
            } else {
                // Convertir a mono promediando canales
                float sum = 0.0f;
                for (int ch = 0; ch < audio->sfinfo.channels; ch++) {
                    sum += audio->audio_data[audio->current_sample + ch];
                }
                buffer_data[i] = sum / audio->sfinfo.channels;
            }
            audio->current_sample += audio->sfinfo.channels;
        }
    }

    // Convertir a 16-bit para OpenAL
    ALshort *al_buffer = malloc(samples_per_buffer * sizeof(ALshort));
    for (size_t i = 0; i < samples_per_buffer; i++) {
        float sample = buffer_data[i];
        // Clipping suave
        if (sample > 1.0f) sample = 1.0f;
        if (sample < -1.0f) sample = -1.0f;
        al_buffer[i] = (ALshort)(sample * 32767.0f);
    }

    alBufferData(buffer, AL_FORMAT_MONO16, al_buffer,
                 samples_per_buffer * sizeof(ALshort), audio->sfinfo.samplerate);

    free(buffer_data);
    free(al_buffer);
}

// Iniciar reproducción
void start_playback(AudioSystem *audio) {
    // Llenar buffers iniciales
    for (int i = 0; i < audio->num_buffers; i++) {
        fill_buffer(audio, audio->buffers[i]);
    }

    alSourceQueueBuffers(audio->source, audio->num_buffers, audio->buffers);
    alSourcePlay(audio->source);
    audio->playing = SDL_TRUE;
}

// Actualizar reproducción (streaming)
void update_playback(AudioSystem *audio) {
    if (!audio->playing) return;

    ALint processed = 0;
    alGetSourcei(audio->source, AL_BUFFERS_PROCESSED, &processed);

    while (processed > 0) {
        ALuint buffer;
        alSourceUnqueueBuffers(audio->source, 1, &buffer);
        fill_buffer(audio, buffer);
        alSourceQueueBuffers(audio->source, 1, &buffer);
        processed--;
    }

    // Verificar si la fuente sigue reproduciéndose
    ALint state;
    alGetSourcei(audio->source, AL_SOURCE_STATE, &state);
    if (state != AL_PLAYING && audio->current_sample < audio->audio_samples) {
        alSourcePlay(audio->source);
    }
}

// Obtener datos de audio actuales para visualización
void get_current_audio_data(AudioSystem *audio, float *output, int size) {
    size_t start_sample = audio->current_sample;
   
    // Retroceder un poco para mostrar datos actuales
    if (start_sample > size * audio->sfinfo.channels) {
        start_sample -= size * audio->sfinfo.channels;
    } else {
        start_sample = 0;
    }

    for (int i = 0; i < size; i++) {
        if (start_sample + i * audio->sfinfo.channels >= audio->audio_samples) {
            output[i] = 0.0f;
        } else {
            if (audio->sfinfo.channels == 1) {
                output[i] = audio->audio_data[start_sample + i];
            } else {
                // Convertir a mono
                float sum = 0.0f;
                for (int ch = 0; ch < audio->sfinfo.channels; ch++) {
                    sum += audio->audio_data[start_sample + i * audio->sfinfo.channels + ch];
                }
                output[i] = sum / audio->sfinfo.channels;
            }
        }
    }
}

// Calcular FFT con suavizado
void calculate_fft(FFTSystem *fft, float *audio_data) {
    float real, imag;

    // Aplicar ventana y copiar datos
    for (int i = 0; i < FFT_SIZE; i++) {
        fft->in[i][0] = audio_data[i]; 
        fft->in[i][1] = 0.0;
    }

    // Ejecutar FFT
    fftw_execute(fft->plan);

    // Calcular magnitudes y aplicar suavizado
    for (int i = 0; i < FFT_SIZE / 2; i++) {
        real = fft->out[i][0];
        imag = fft->out[i][1];
        fft->magnitudes[i] = sqrtf(real * real + imag * imag);
    }
}

// Dibujar espectro FFT usando rectángulos SDL
void draw_fft_spectrum(SDL_Renderer *renderer, float *magnitudes, int size, int y_offset, int height, int width) {
    float magnitutMaxima = -FLT_MAX;
    float magnitude, normalized;
    int bar_width, bar_height;

    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255); // Color barres: negre == RGB+Alpha (opaco)

    bar_width = width / size;
    if (bar_width < 1) bar_width = 1;

    // Preparar array de rectángulos para batch rendering
    SDL_Rect *rects = malloc(size * sizeof(SDL_Rect));
    int rect_count = 0;

     // Escala lineal: determinar el màxim    
    for (int i = 0; i < size; i++) {
        if (magnitutMaxima < magnitudes[i])
            magnitutMaxima = magnitudes[i];
    }
    magnitutMaxima = magnitutMaxima + 1; // Per allò dels redondejos que en poder quedar dos molt molt prop ;-)
    
    for (int i = 0; i < size; i++)  {
        magnitude = magnitudes[i];
       
        // Escala lineal: aplicar la normalització
        normalized = magnitudes[i] / magnitutMaxima;
        bar_height = (int)(normalized * height);
       
        if (bar_height > 0) {
            rects[rect_count].x = i * bar_width;
            rects[rect_count].y = y_offset + height - bar_height;
            rects[rect_count].w = bar_width - 1;
            rects[rect_count].h = bar_height;
            rect_count++;
        }
    }
   
    // Dibujar todos los rectángulos en una sola llamada (más eficiente)
    if (rect_count > 0) {
        SDL_RenderFillRects(renderer, rects, rect_count);
    }
   
    free(rects);
}

// Renderizar interfaz usando SDL puro
void render(App *app, AudioSystem *audio, FFTSystem *fft) {
    // Inicialitza pantalla en color blanc
    SDL_SetRenderDrawColor(app->renderer, 255, 255, 255, 255);
    SDL_RenderClear(app->renderer);

    // Obtener datos de audio actuales
    float audio_data[FFT_SIZE];
    get_current_audio_data(audio, audio_data, FFT_SIZE);

    calculate_fft(fft, audio_data);
    draw_fft_spectrum(app->renderer, fft->magnitudes, FFT_SIZE / 2, 0, app->window_height, app->window_width);

    SDL_RenderPresent(app->renderer);
}


// Manejar eventos
void handle_events(App *app, AudioSystem *audio) {
    SDL_Event event;
    while (SDL_PollEvent(&event)) {
        switch (event.type) {
            case SDL_QUIT:
                app->running = SDL_FALSE;
                break;
               
            case SDL_WINDOWEVENT:
                if (event.window.event == SDL_WINDOWEVENT_RESIZED) {
                    app->window_width = event.window.data1;
                    app->window_height = event.window.data2;
                    printf("Ventana redimensionada a: %dx%d\n", app->window_width, app->window_height);
                }
                break;
               
            case SDL_KEYDOWN:
                switch (event.key.keysym.sym) {                                             
                    case SDLK_ESCAPE: // ESC --> terminar
                        {
                                app->running = SDL_FALSE;
                        }
                        break;
                       

                }
                break;
        }
    }
}

// Lliberar recursos asociats a recursos
void liberarRecursos(App *app, AudioSystem *audio, FFTSystem *fft) {
    // Lliberar recursos asociats a OpenAL
    if (audio->source) {
        alSourceStop(audio->source);
        alDeleteSources(1, &audio->source);
    }
    if (audio->buffers) {
        alDeleteBuffers(audio->num_buffers, audio->buffers);
        free(audio->buffers);
    }
    if (audio->context) {
        alcMakeContextCurrent(NULL);
        alcDestroyContext(audio->context);
    }
    if (audio->device) {
        alcCloseDevice(audio->device);
    }
    if (audio->sndfile) {
        sf_close(audio->sndfile);
    }
    if (audio->audio_data) {
        free(audio->audio_data);
    }

    // Lliberar recursos asociats a FFTW
    if (fft->plan) {
        fftw_destroy_plan(fft->plan);
    }
    if (fft->in) {
        fftw_free(fft->in);
    }
    if (fft->out) {
        fftw_free(fft->out);
    }
    if (fft->magnitudes) {
        free(fft->magnitudes);
    }

    if (app->fft_texture) {
        SDL_DestroyTexture(app->fft_texture);
    }
    if (app->renderer) {
        SDL_DestroyRenderer(app->renderer);
    }
    if (app->window) {
        SDL_DestroyWindow(app->window);
    }
    SDL_Quit();
}

int main(int argc, char *argv[]) {
    if (argc != 2) {
        printf("Uso: %s <archivo_audio>\n", argv[0]);
        printf("Formatos: WAV, FLAC, OGG, etc.\n");
        return 1;
    }

    App app = {0};
    AudioSystem audio = {0};
    FFTSystem fft = {0};

    // Inicializar sistemas
    if (!init_sdl(&app)) {
        liberarRecursos(&app, &audio, &fft);
        return 1;
    }

    if (!init_openal(&audio)) {
        liberarRecursos(&app, &audio, &fft);
        return 1;
    }

    if (!init_fft(&fft)) {
        liberarRecursos(&app, &audio, &fft);
        return 1;
    }

    if (!load_audio_file(argv[1], &audio)) {
        liberarRecursos(&app, &audio, &fft);
        return 1;
    }

    // Mostrar controles
    printf("\nInfo: \n");
    printf("ESC: Salir \n");
    printf("\nIniciando reproducción...\n");

    // Iniciar reproducción
    start_playback(&audio);

    // Loop principal
    while (app.running) {     
        handle_events(&app, &audio);
        update_playback(&audio);
        render(&app, &audio, &fft);
       
        SDL_Delay(25);
    }

    liberarRecursos(&app, &audio, &fft);
    printf("Aplicación cerrada correctamente\n");
    return 0;
}
