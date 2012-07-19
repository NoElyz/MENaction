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

	SDL_AudioSpec *wanted_spec;
	SDL_AudioSpec *spec;
	if(SDL_OpenAudio(&wanted_spec, &spec)<0){
			fprintf(stderr,"SDL_OpenAudio:%s\n",SDL_GetError());
			return -1;
	}
	AVCodec *aCodec;
	aCodec=avcodec_find_decoder(aCodecCtx->codec_id);
	if(!aCodec){
		fprintf(stderr,"Unsupported codec!\n");
		return -1;
	}
	avcodec_open(aCodecCtx,aCodec);
}

	// The queues part
	typedef struct PacketQueue{
		AVPacketList *first_pkt, *last_pkt;
		int nb_packets;// wrong Bint in the pdf--use int instead upon the web version
		int size;
		SDL_mutex *mutex;
		SDL_cond *cond;
	}PacketQueue;

	// Initialize the queue
	void packet_queue_init(PacketQueue *q) {
		memset (q, 0, sizeof(PacketQueue));
		q->mutex = SDL_CreateMutex();
		q->cond = SDL_CreateCond();
	}
	
	// Put stuff into the queue
	int packet_queue_put(PacketQueue  *q,AVPacket *pkt){
		AVPacketList *pkt1;
		if(av_dup_packet(pkt)<0){
			return -1;
		}
		pkt1 = av_malloc(sizeof(AVPacketList));

		if (!pkt1)
			return -1;
		pkt1->pkt = *pkt;
		pkt1->next = NULL;

		SDL_LockMutex(q->mutex);

		if (!q->last_pkt)
			q->first_pkt = pkt1;
		else
			q->last_pkt->next = pkt1;

		q->last_pkt =pkt1;
		q->nb_packets++;
		q->size += pkt1->pkt.size;
		SDL_CondSignal(q->cond);

		SDL_UnlockMutex(q->mutex);
		return 0;
	}

int quit = 0;

static int packet_queue_get(PacketQueue *q, AVPacket *pkt, int block){ // invalid storage class for function packet_queue_get appears
		AVPacketList *pkt1;
		int ret;

		SDL_LockMutex(q->mutex);

		for(;;){

			if(quit){
				ret = -1;
				break;
			}

		pkt1 = q->first_pkt;

		if (pkt1) {
			q->first_pkt = pkt1->next;

			if (!q->first_pkt)
				q->last_pkt = NULL;

			q->nb_packets--;
			q->size -= pkt1->pkt.size;
			*pkt = pkt1->pkt;
			av_free(pkt1);
			ret =1;
			break;
			} else if (!block) {
				ret = 0;
				break;
			} else {
				SDL_CondWait(q->cond, q->mutex);
			}
		}
	SDL_UnlockMutex(q->mutex);
	return ret;
	}

