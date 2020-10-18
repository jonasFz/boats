#include "bucket.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

Bucket make_bucket(){
	Bucket b;
	b.data = malloc(DEFAULT_BUCKET_SIZE);
	b.capacity = DEFAULT_BUCKET_SIZE;
	b.count = 0;

	return b;
}

void destroy_bucket(Bucket *bucket){
	if(bucket->data != NULL){
		free(bucket->data);
		bucket->data = NULL;
	}
	bucket->capacity = 0;
	bucket->count = 0;
}

void write_to_bucket(Bucket *bucket, void *d, unsigned int d_length){
	int resize = 0;
	while(d_length >= (bucket->capacity - bucket->count)){
		bucket->capacity *= 2;
		resize = 1;
	}
	if(resize){
		bucket->data = realloc(bucket->data, bucket->capacity);
	}

	memcpy(bucket->data+bucket->count, d, d_length);
	bucket->count += d_length;
}
