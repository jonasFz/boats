#ifndef _H_BUCKET
#define _H_BUCKET

#define DEFAULT_BUCKET_SIZE 8

// A "Dynamic" array, I just need a container
// that can grow automatically, usefull for stupid
// text based files, looking at you stl, that don't
// report how much space I need right off the bat!
typedef struct{
	void *data;
	unsigned int capacity;
	unsigned int count;

} Bucket;

Bucket make_bucket();
void destroy_bucket(Bucket *bucket);
void write_to_bucket(Bucket *bucket, void *d, unsigned int d_length);

#endif
