# include "./f2s.h"

int avformat_network_init();

int main (int argc, char *argv[])
{
	av_register_all(); // registed all file format
	AVFormatContext *pFormatCtx; // to define the structure *point of which being used
	// Open media(video/audio whatever) file
	if (av_open_input_file(&pFormatCtx, argv[1], NULL, 0, NULL)!=0)
		return -1; // couldn't open file
	// Retrieve stream information
	if (av_find_stream_info(pFormatCtx)<0)
		return -1; // couldn't find stream information
	// Dump information about file onto standard error
	dump_format (pFormatCtx, 0, argv[1], 0);

	// Find the first sound stream
	int i;
	AVCodecContext *aCodecCtx;
	int audioStream=-1;
		for (i=0;i<pFormatCtx->nb_streams;i++)
			if (pFormatCtx->streams[i]->codec->codec_type==AVMEDIA_TYPE_AUDIO && audioStream < 0) //it's new one but not CODEC_TYPE_AUDIO
			{
				audioStream=i;
				break;
			}
	if(audioStream==-1)
		return -1; //didn't find a audio stream
	// Get a pointer to the codec context for the video stream
	aCodecCtx=pFormatCtx->streams[audioStream]->codec;
//	printf ("%d\n",audioStream); //for test
}

