# include "./f2s.h"

int avformat_network_init();
int decode_interrupt_cb(void);
int quit;

// The queues part
typedef struct PacketQueue{
	AVPacketList *first_pkt, *last_pkt;
	int nb_packets;// wrong Bint in the pdf--use int instead upon the web version
	int size;
	SDL_mutex *mutex;
	SDL_cond *cond;
}PacketQueue;

PacketQueue audioq;





int main (int argc, char *argv[])
{
	av_register_all(); // registed all file format
	AVFormatContext *pFormatCtx; // to define the structure *point of which being used
	// Open media(video/audio whatever) file
	if (avformat_open_input(&pFormatCtx, argv[1], NULL, NULL)!=0){//changed av_open_input_file into the new avformat_open_input, thus no use for the past argument int buf_size inside av_open_input_file.
		return -1; // couldn't open file
	avformat_input_file(&pFormatCtx);//av_close_input_file has be deprecated
	}
	// Retrieve stream information
	if (avformat_find_stream_info(pFormatCtx,NULL)<0)//also use the new one instead of int av_find_stream_info and set NULL for the AVDictionary **options. But notice that, it could be nb_streams.
		return -1; // couldn't find stream information
	// Dump information about file onto standard error
	av_dump_format (pFormatCtx, 0, argv[1], 0);//use the new av_dump_format instead of dump_format

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
	avcodec_open2(aCodecCtx,aCodec,NULL);//use the new 2 version instead of the original version
	
	packet_queue_init(&audioq);
	SDL_PauseAudio(0);

	AVPacket packet;
	while (av_read_frame(pFormatCtx, &packet)>=0){
//		if(packet.stream_index==videoStream){ //for there's not concern with the viedoStream so no use for the determination
/*		} else*/ if (packet.stream_index==audioStream){
			packet_queue_put(&audioq, &packet);
		} else {
			av_free_packet(&packet);
		}
	}


/*	SDL_Event event;
	static VideoState *global_video_state;
	static int decode_interrupt_cb(void *ctx){
		return global_video_state && global_video_state->abort_request;
	}
//	url_set_interrupt_cb(decode_interrupt_cb);//undefined in the current libavformat/avio.h version. Use above instead.
	SDL_PollEvent(&event);
	switch(event.type){
		case SDL_QUIT:
		quit = 1;
	}*/
}

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


//In Case of Fire(?)
int decode_interrupt_cb(void){
	return quit;
}

int audio_decode_frame (AVCodecContext *aCodecCtx, uint8_t *audio_buf,int buf_size){
	static AVPacket pkt;
	static uint8_t *audio_pkt_data = NULL;
	static int audio_pkt_size = 0;

	int len1, data_size;

	for (;;){
		while(audio_pkt_size > 0){
			data_size = buf_size;
			len1 = avcodec_decode_audio4(aCodecCtx, (int16_t *)audio_buf, &data_size,  audio_pkt_size);//avcodec_decode_audio2 has been replaced by avcodec_decode_audio4

			if(len1 < 0){
				/* in error, skip frame */
				audio_pkt_size = 0;
				break;
			}

			audio_pkt_data += len1;
			audio_pkt_size -= len1;

			if (data_size <= 0){
				/* No data yet, get more frame */
				continue;
			}
			/* We have data, return it and come back for more later */
			return data_size;
		}
		if (pkt.data)
			av_free_packet(&pkt);

		if (quit)
			return -1;

		if (packet_queue_get(&audioq,&pkt,1) < 0)
			return -1;
		audio_pkt_data = pkt.data;
		audio_pkt_size = pkt.size;
	}
}
