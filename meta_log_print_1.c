#ifdef ENABLE_RFDC

#include <metal/log.h>

#include "metal/sys.h"


void my_metal_default_log_handler(enum metal_log_level level,

			       const char *format, ...){

	char msg[1024];

	char msgOut[1048];

	char *outPtr;

	int i;


	va_list args;

	static const char *level_strs[] = {

		"metal: emergency: ",

		"metal: alert:     ",

		"metal: critical:  ",

		"metal: error:     ",

		"metal: warning:   ",

		"metal: notice:    ",

		"metal: info:      ",

		"metal: debug:     ",

	};


	va_start(args, format);

	vsnprintf(msg, sizeof(msg), format, args);

	va_end(args);


	//replace single \n with \n\r

	outPtr = msgOut;

	for(i=0; i<1024; i++) {

		// if /n/r or /r/n combo

		if ((msg[i] == '\r' && msg[i+1] == '\n') ||

				(msg[i] == '\n' && msg[i+1] == '\r')) {

			*outPtr++ = msg[i++];

		} else if(msg[i] == '\n') {

			//if first char in string is \n, then remove

			if(i==0) {

				continue;

			} else {

				*outPtr++ = '\r';

			}

		}

		*outPtr++ = msg[i];

		if(msg[i] == 0) {

			break;

		}

	}

	//if line doesn't end with \n\r, then add it

	if( (msg[i-1] != '\n') && (msg[i-1] != '\r') ) {

		*(outPtr-1) = '\r';

		*outPtr++ = '\n';

		*outPtr++ = 0;

	}


	if (level <= METAL_LOG_EMERGENCY || level > METAL_LOG_DEBUG)

		level = METAL_LOG_EMERGENCY;


	xil_printf("%s%s", level_strs[level], msgOut);

}

#endif
