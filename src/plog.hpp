#ifndef PLOG_H
#define PLOG_H

#include <stdint.h>
#include <stddef.h>
#include <time.h>

typedef struct
{
	uint32_t id;
	struct timespec start;
	struct timespec end;

} plog_t;

typedef struct
{
	plog_t* first;
	plog_t* last;
	plog_t* current;
} plog_buffer_t;


enum plogRetCode
{
	success = 0,
	outOfSpace = -1,
};

int startPlog(plog_t *log, uint32_t id);

int endPlog(plog_t *log);

int getPlog(plog_buffer_t *buff, plog_t **log);

int initPlogBuff(size_t size, plog_buffer_t *buff);

int getStartPlog(plog_buffer_t *buff, plog_t **log, uint32_t id);

int printPlog(plog_t *log);

int printPlogBuff(plog_buffer_t *buff);

int csvAppendPlog(plog_t *log, const char *filename);

int csvAppendPlogBuff(plog_buffer_t *buff, const char *filename);




#endif