#include "plog.hpp"
#include <stdlib.h>
#include <stdio.h>

int startPlog(plog_t* log, uint32_t id)
{
	if(!log)
	{
		return -1;
	}

	log->id = id;
	clock_gettime(CLOCK_REALTIME, &(log->start));
	return 0;
}

int getStartPlog(plog_buffer_t *buff, plog_t **log, uint32_t id)
{
	getPlog(buff, log);
	startPlog(*log, id);
	return 0;
}

int endPlog(plog_t *log)
{	
	if(!log)
	{
		return -1;
	}

	clock_gettime(CLOCK_REALTIME, &(log->end));
	return 0;
}

int getPlog(plog_buffer_t *buff, plog_t **log)
{
	//not garunteed atomic but close
	*log = (buff->current)++;

	if(*log > buff->last)
	{
		*log = 0;
		return -1;
	}

	return 0;
}

int initPlogBuff(size_t size, plog_buffer_t *buff)
{
	if(!buff)
	{
		return -1; 
	}

	buff->first = (plog_t *) malloc(size * sizeof(plog_t));
	
	if(!(buff->first))
	{
		return -1; 
	}

	buff->current = buff->first;
	buff->last = buff->first + size - 1;
	
	return 0;
}

int printPlog(plog_t *log)
{
	printf("%d, %ld.%09ld, %ld.%09ld\n", log->id, (log->start).tv_sec, (log->start).tv_nsec, (log->end).tv_sec, (log->end).tv_nsec);
	return 0;
}

int printPlogBuff(plog_buffer_t *buff)
{
	plog_t *curr = buff->first;
	plog_t *last;

	if(buff->last > buff->current)
	{
		last = buff->current;
	}
	else
	{
		last = buff->last;
	}

	for(; curr <= last; curr++)
	{
		printPlog(curr);
	}

	return 0;
}

int csvAppendNPlog(plog_t *log, FILE *f)
{
	fprintf(f,"%d, %ld.%09ld, %ld.%09ld\n", log->id, (log->start).tv_sec, (log->start).tv_nsec, (log->end).tv_sec, (log->end).tv_nsec);
	return 0;
}

int csvAppendPlog(plog_t *log, const char *filename)
{
	FILE *fptr;
	fptr = fopen(filename,"a");
	csvAppendNPlog(log,fptr);
	fclose(fptr);
	return 0;
}

int csvAppendPlogBuff(plog_buffer_t *buff, const char *filename)
{
	plog_t *curr = buff->first;
	plog_t *last;

	if(buff->last > buff->current)
	{
		last = buff->current;
	}
	else
	{
		last = buff->last;
	}

	FILE *fptr;
	fptr = fopen(filename,"a");

	for(; curr <= last; curr++)
	{
		csvAppendNPlog(curr,fptr);
	}

	fclose(fptr);

	return 0;
}