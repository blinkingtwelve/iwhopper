/*
 * iwhopper, a simple channel hopper for linux
 * Copyright (c) 2014 Wicher Minnaard <wicher@nontrivialpursuit.org>
 *
 * Based on 'iwconfig' code by Jean Tourrilhes.
 * Includes many parts of the GPL 'wireless-tools', by same.
 * see http://www.hpl.hp.com/personal/Jean_Tourrilhes/Linux/Tools.html
 *
 *
 * This file is released under the GPL license.
 * Copyright (c) 2014 Wicher Minnaard <wicher@nontrivialpursuit.org>
 * Copyright (c) 1997-2007 Jean Tourrilhes <jt@hpl.hp.com>
 * 
 */

#include "iwlib-private.h"
#include <time.h>
#include <fcntl.h>
#include <sys/stat.h>

/*------------------------------------------------------------------*/
/*
 * Fill iwreq struct with freq converted from
 * channel/frequency character representation
 */

static void chanchar2wrq(char *chan, struct iwreq *wrq){
	double		freq;
	char *		rest;

	freq = strtod(chan, &rest);
	if(rest == chan) {
	fprintf(stderr, "Error making sense of frequency '%s'\n", chan);
	exit(-1);
	}
	iw_float2freq(freq, &(wrq->u.freq));
	wrq->u.freq.flags = IW_FREQ_FIXED;
}

/*------------------------------------------------------------------*/
/*
 * Set frequency/channel
 */
static void set_freq_info(int skfd, char *ifname, char *chan){
	struct iwreq wrq;
	chanchar2wrq(chan, &wrq);
	if(iw_set_ext(skfd, ifname, SIOCSIWFREQ, &wrq) < 0){
	fprintf(stderr, "Error changing channel - iw_set_ext failed.\n");
	}
}

/*------------------------------------------------------------------*/
/*
 * Display help
 */
static inline void iw_usage(void){
	fprintf(stderr, "Usage: iwhopper INTERFACE DWELLTIME CHANNELS [LOCKFILE]\n\n");
	fprintf(stderr, "example: iwhopper wlan0 2.5 1,2,2,2,2,3 /tmp/somelock\n");
	fprintf(stderr, "this will spend 2.5 seconds on channel 1, 10 seconds\n");
	fprintf(stderr, "on channel 2, and 2.5 seconds on channel 3 -- then starts over.\n\n");
	fprintf(stderr, "Running multiple instances of iwhopper with the same LOCKFILE\n");
	fprintf(stderr, "will prevent them from changing channels concurrently -- use\n");
	fprintf(stderr, "when your hardware can't handle that (multiple PHYs attached to\n");
	fprintf(stderr, "the same chip.\n\n");
}

/*------------------------------------------------------------------*/
/*
 * The main
 */

int main(int argc, char ** argv){
	int skfd;		/* generic raw socket desc.*/
	float dwell = 0;
	int syncfd = 0;

	/* need devname, dwelltime, at least 1 chan */
	if(argc < 4){
		iw_usage();
		exit(-1);
	}

	char* dev = argv[1];

	/* if one is specified, open the lockfile */
	if (argc == 5){
		char* fname = argv[4];
		if ((syncfd = open(fname,O_RDWR | O_CREAT, S_IRUSR | S_IWUSR)) < 0){
			perror("while opening lockfile");
			exit(-1);
		}
	}

	/* Create a channel to the NET kernel. */
	if((skfd = iw_sockets_open()) < 0){
		perror("socket");
		exit(-1);
	}

	/* parse dwell time */
	char* rest;
	dwell = strtof(argv[2], &rest);
	if(rest == argv[2]){
	fprintf(stderr,"Couldn't make sense of dwell time '%s'\n", rest);
		iw_usage();
		exit(-1);
	}
	struct timespec dwell_ts;
	dwell_ts.tv_sec = dwell;
	dwell_ts.tv_nsec = 1000000000 * (dwell - dwell_ts.tv_sec);

	/* setup some variables for the channel changing loop */
	char* chans = argv[3];
	char delim = ',';
	char* chan;
	char* prevchan = NULL;

	/* the channel changing loop */
	while(1){
		char c_chans[strlen(chans)+1];
		strcpy(c_chans, chans); //strtok is destructive, so it gets a copy to work on
		chan = strtok(c_chans, &delim);
		while (chan){ //loop once through the channel list
			if ( (prevchan == NULL) || (strcmp(chan,prevchan) != 0)){
				/* Only actually set the freq if the channel's changed.
				* We assume we're alone in this world and no one else
				* is changing the device's channels.
				*/

				// acquire lock
				if (syncfd){
					if (lockf(syncfd, F_LOCK, 0) < 0){
						perror("while acquiring lock");
						exit(-1);
					}
				}

				// at long last, we hop
				set_freq_info(skfd, dev, chan);

				// release lock
				if (syncfd){
					if (lockf(syncfd, F_ULOCK, 0) < 0){
						perror("while releasing lock");
						exit(-1);
					}
				}
			}
			// dwell
			nanosleep(&dwell_ts,NULL);
			prevchan = chan;
			// advance in channel list
			chan = strtok(NULL, &delim);
		}
	}
	iw_sockets_close(skfd);
	return(1);
}
