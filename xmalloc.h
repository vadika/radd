#ifndef _XMALLOC_
#define _XMALLOC_
#define MALLOCDEBUG 

#ifdef MALLOCDEBUG
	void *_xmalloc(size_t,char *, int);
	void *_xrealloc(void *, size_t, char *, int);
	void *_xcalloc(size_t, size_t,char *, int);
	void _xfree(void *, char *, int);
	#define Xmalloc(x) _xmalloc(x,__FILE__,__LINE__)
	#define Xrealloc(x,y) _xrealloc(x,y,__FILE__,__LINE__)
	#define Xcalloc(x,y) _xcalloc(x,y,__FILE__,__LINE__)
	#define Xfree(x)	_xfree(x,__FILE__,__LINE__)
#else
	#define Xmalloc(x) malloc(x)
	#define Xrealloc(x,y) realloc(x,y)
	#define Xcalloc(x,y)	calloc(x,y)
	#define Xfree(x)		free(x) 
#endif

#endif
