/*
 * opusal: A quick example of using libopusfile and OpenAL.
 *
 * Written by David Gow <david@ingeniumdigital.com>
 *
 * To the extent possible under law, the author has dedicated all copyright and
 * neighbouring rights to this software to the public domain worldwide.
 *
 * This software is distributed without any warranty.
 * 
 * gcc opusal.c  -o opusal `pkg-config openal --cflags --libs` `pkg-config opusfile --cflags --libs`
 * 
 */

#include <AL/al.h>
#include <AL/alc.h>
#include <opus/opusfile.h>
//#include <cstdio> //new
#include <stdio.h>
#include <stdlib.h> //malloc

// Convert an opus error code to a string.
const char *opus_error_to_string(int error)
{
	switch (error)
	{
	case OP_FALSE:
		return "OP_FALSE: A request did not succeed.";
	case OP_HOLE:
		return "OP_HOLE: There was a hole in the page sequence numbers.";
	case OP_EREAD:
		return "OP_EREAD: An underlying read, seek or tell operation failed.";
	case OP_EFAULT:
		return "OP_EFAULT: A NULL pointer was passed where none was expected, or an internal library error was encountered.";
	case OP_EIMPL:
		return "OP_EIMPL: The stream used a feature which is not implemented.";
	case OP_EINVAL:
		return "OP_EINVAL: One or more parameters to a function were invalid.";
	case OP_ENOTFORMAT:
		return "OP_ENOTFORMAT: This is not a valid Ogg Opus stream.";
	case OP_EBADHEADER:
		return "OP_EBADHEADER: A required header packet was not properly formatted.";
	case OP_EVERSION:
		return "OP_EVERSION: The ID header contained an unrecognised version number.";
	case OP_EBADPACKET:
		return "OP_EBADPACKET: An audio packet failed to decode properly.";
	case OP_EBADLINK:
		return "OP_EBADLINK: We failed to find data we had seen before or the stream was sufficiently corrupt that seeking is impossible.";
	case OP_ENOSEEK:
		return "OP_ENOSEEK: An operation that requires seeking was requested on an unseekable stream.";
	case OP_EBADTIMESTAMP:
		return "OP_EBADTIMESTAMP: The first or last granule position of a link failed basic validity checks.";
	default:
		return "Unknown error.";
	}
}


// Load an ogg opus file into the given AL buffer
int load_opus(ALuint buffer, const char *filename)
{
	int error = 0;
	// Open the file.
	OggOpusFile *file = op_open_file(filename, &error);
	if (error)
	{
		fprintf(stderr, "Failed to open file %s (%d: %s)\n",filename, error, opus_error_to_string(error));
		return error;
	}

	// Get the number of channels in the current link.
	int num_channels = op_channel_count(file,-1);
	// Get the number of samples (per channel) in the current link.
	int pcm_size = op_pcm_total(file,-1);

	printf("%s: %d channels, %d samples (%d seconds)\n",filename, num_channels, pcm_size, pcm_size/48000);

	// We only support stereo and mono, set the openAL format based on channels.
	// opus always uses signed 16-bit integers, unless the _float functions are called.
	ALenum format;
	if (num_channels == 1)
	{
		format = AL_FORMAT_MONO16;
	}
	else if (num_channels == 2)
	{
		format = AL_FORMAT_STEREO16;
	}
	else
	{
		fprintf(stderr, "File contained more channels than we support (%d).\n", num_channels);
		return -130; //EIMPL
	}

	// Allocate a buffer big enough to store the entire uncompressed file.
	//int16_t *buf = new int16_t[pcm_size*num_channels];
	int16_t *buf;
	
	buf = (int16_t *)malloc(pcm_size*num_channels*16);

	if (!buf)
	{
		fprintf(stderr, "Could not allocate decode buffer.\n");
		return -129; //EFAULT
	}

	int samples_read = 0;

	// Keep reading samples until we have them all.
	while (samples_read < pcm_size)
	{
		// op_read returns number of samples read (per channel), and accepts number of samples which fit in the buffer, not number of bytes.
		int ns = op_read(file, buf + samples_read*num_channels, pcm_size*num_channels, 0);
		if (ns < 0)
		{
			fprintf(stderr, "Couldn't decode at offset %d: Error %d (%s)\n",samples_read,ns,opus_error_to_string(ns));
			return ns;
		}
		samples_read += ns;
		printf("Read %d (total %d) of %d samples (per channel)\n",ns, samples_read, pcm_size);
	}

	// Close the opus file.
	op_free(file);

	// Send it to OpenAL (which takes bytes).
	alBufferData(buffer, format, buf, samples_read*num_channels*2, 48000);
	if (alGetError() == AL_NO_ERROR)
		printf("Buffered data!\n");
	else
	{
		fprintf(stderr, "Failed to buffer data!\n");
		return -129; //EFAULT
	}
}

int main(int argc, char **argv)
{

	ALuint testBuffer;
	ALuint testSource;

	ALCdevice *dev;
	ALCcontext *ctx;

	if (argc <= 1)
	{
		printf("Usage: %s <filename>\n", argv[0]);
		return -1;
	}
	

	// Open the default device.
	dev = alcOpenDevice(0);

	if (!dev)
	{
		fprintf(stderr, "Couldn't open OpenAL device.\n");
		return -1;
	}

	// We want an OpenAL context.
	ctx = alcCreateContext(dev, 0);

	alcMakeContextCurrent(ctx);

	if (!ctx)
	{
		fprintf(stderr,"Context fail\n");
		alcCloseDevice(dev);
		return -1;
	}

	// Get us a buffer and a source to attach it to.
	alGenBuffers(1, &testBuffer);
	alGenSources(1, &testSource);

	// Set position and gain for the listener.
	alListener3f(AL_POSITION, 0.f,0.f,0.f);
	alListenerf(AL_GAIN,1.f);

	// Load the file into the buffer.
	load_opus(testBuffer, argv[1]);

	// Attach the buffer to the source.
	alSourcei(testSource, AL_BUFFER, testBuffer);
	if (alGetError() != AL_NO_ERROR)
	{
		fprintf(stderr, "Could not set source buffer!\n");
	}
	// ... and set source properties.
	alSource3f(testSource, AL_POSITION, 0.f,0.f,0.f);
	alSourcef(testSource, AL_GAIN, 1.0f);

	// Play the source.
	alSourcePlay(testSource);

	if (alGetError() != AL_NO_ERROR)
	{
		fprintf(stderr, "Could not play source!\n");
	}

	// Keep playing until we're finished.
	int sourceState;
	do
	{
		alGetSourcei(testSource, AL_SOURCE_STATE, &sourceState);
	} while (sourceState != AL_STOPPED);

	// Clean up a bit.
	alDeleteSources(1, &testSource);
	alDeleteBuffers(1, &testBuffer);

	alcMakeContextCurrent(0);
	alcDestroyContext(ctx);
	alcCloseDevice(dev);

	return 0;
}


