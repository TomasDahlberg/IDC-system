	ifndef	??version
?debug	macro
	endm
$comm	macro	name,dist,size,count
	comm	dist name:BYTE:count*size
	endm
	else
$comm	macro	name,dist,size,count
	comm	dist name[size]:BYTE:count
	endm
	endif
	?debug	S "win3208.c"
	?debug	C E92E4A091D0977696E333230382E63
	?debug	C E90008A41413433A5C54435C494E434C5544455C6469722E68
	?debug	C E90008A41415433A5C54435C494E434C5544455C737464696F2E68
	?debug	C E90008A41416433A5C54435C494E434C5544455C7374646C69622E+
	?debug	C 68
	?debug	C E90008A41416433A5C54435C494E434C5544455C737472696E672E+
	?debug	C 68
	?debug	C E90008A41415433A5C54435C494E434C5544455C636F6E696F2E68
_TEXT	segment byte public 'CODE'
_TEXT	ends
DGROUP	group	_DATA,_BSS
	assume	cs:_TEXT,ds:DGROUP
_DATA	segment word public 'DATA'
d@	label	byte
d@w	label	word
_DATA	ends
_BSS	segment word public 'BSS'
b@	label	byte
b@w	label	word
_BSS	ends
_TEXT	segment byte public 'CODE'
   ;	
   ;	getTotalSizeOfVars(char *meta)
   ;	
	assume	cs:_TEXT
_getTotalSizeOfVars	proc	near
	push	bp
	mov	bp,sp
	push	si
	push	di
   ;	
   ;	{
   ;	  int id = 1, siz = 0;
   ;	
	mov	di,1
	xor	si,si
	jmp	short @1@122
@1@50:
   ;	
   ;	  while (metaName(meta, id) > 0) {
   ;	/*    printMeta(meta, id);           */
   ;	    if (metaRemote(meta, id))
   ;	
	push	di
	push	word ptr [bp+4]
	call	near ptr _metaRemote
	pop	cx
	pop	cx
	or	ax,ax
	je	short @1@98
   ;	
   ;	      siz += 4;
   ;	
	add	si,4
@1@98:
   ;	
   ;	    siz += metaSize(meta, id);
   ;	
	push	di
	push	word ptr [bp+4]
	call	near ptr _metaSize
	pop	cx
	pop	cx
	add	si,ax
   ;	
   ;	    siz ++;
   ;	
	inc	si
   ;	
   ;	    id ++;
   ;	
	inc	di
@1@122:
	push	di
	push	word ptr [bp+4]
	call	near ptr _metaName
	pop	cx
	pop	cx
	or	ax,ax
	ja	short @1@50
   ;	
   ;	  }
   ;	/*  printf("size = %d\n", siz);      */
   ;	  return siz;
   ;	
	mov	ax,si
	jmp	short @1@170
@1@170:
   ;	
   ;	}
   ;	
	pop	di
	pop	si
	pop	bp
	ret	
_getTotalSizeOfVars	endp
   ;	
   ;	int main(int argc, char *argv[])
   ;	
	assume	cs:_TEXT
_main	proc	near
	push	bp
	mov	bp,sp
	sub	sp,6
	push	si
	push	di
   ;	
   ;	{
   ;	  FILE *fp1, *fp3;
   ;	  char line[256], screen[80], var[80];
   ;	  int id, picId, lastPicId, c;
   ;	  int varSize;
   ;	  char *varChunk;
   ;	  char *varChunk2;
   ;	
   ;	  if (argc != 2) {
   ;	
	cmp	word ptr [bp+4],2
	je	short @2@74
   ;	
   ;		printf("Usage: win3208 <metavar>\n");
   ;	
	mov	ax,offset DGROUP:s@
	push	ax
	call	near ptr _printf
	pop	cx
   ;	
   ;		exit(1);
   ;	
	mov	ax,1
	push	ax
	call	near ptr _exit
	pop	cx
@2@74:
   ;	
   ;	  }
   ;	/*
   ;	!	Algorithm:
   ;	!		read metamod
   ;	!		create vars structure
   ;	!		call first screen
   ;	*/
   ;	  readMetaMod(argv[1]);
   ;	
	mov	bx,word ptr [bp+6]
	push	word ptr [bx+2]
	call	near ptr _readMetaMod
	pop	cx
   ;	
   ;	  hilo = 0x4afc;
   ;	
	mov	word ptr DGROUP:_hilo,19196
   ;	
   ;	  if (*((char *) &hilo) == 0x4a)
   ;	
	cmp	byte ptr DGROUP:_hilo,74
	jne	short @2@122
   ;	
   ;	    hilo = 1;                             /* like MC68K */
   ;	
	mov	word ptr DGROUP:_hilo,1
	jmp	short @2@146
@2@122:
   ;	
   ;	  else
   ;	    hilo = 0;                             /* maybe intel */
   ;	
	mov	word ptr DGROUP:_hilo,0
@2@146:
   ;	
   ;	  if (!hilo)
   ;	
	cmp	word ptr DGROUP:_hilo,0
	jne	short @2@194
   ;	
   ;	    swapMetaMod(metaMod);
   ;	
	push	word ptr DGROUP:_metaMod
	call	near ptr _swapMetaMod
	pop	cx
@2@194:
   ;	
   ;	/*
   ;	!	now initialize data module vars
   ;	*/
   ;	  varSize = getTotalSizeOfVars(metaMod);
   ;	
	push	word ptr DGROUP:_metaMod
	call	near ptr _getTotalSizeOfVars
	pop	cx
	mov	word ptr [bp-4],ax
   ;	
   ;	  if ((varChunk = (char *) calloc(1, varSize)) == NULL) {
   ;	
	push	word ptr [bp-4]
	mov	ax,1
	push	ax
	call	near ptr _calloc
	pop	cx
	pop	cx
	mov	word ptr [bp-6],ax
	or	ax,ax
	jne	short @2@242
   ;	
   ;	    printf("Not enough memory to allocate %d bytes (VARS).\n", varSize);
   ;	
	push	word ptr [bp-4]
	mov	ax,offset DGROUP:s@+26
	push	ax
	call	near ptr _printf
	pop	cx
	pop	cx
   ;	
   ;	    exit(errno);
   ;	
	push	word ptr DGROUP:_errno
	call	near ptr _exit
	pop	cx
@2@242:
   ;	
   ;	  }
   ;	/*
   ;	  if ((varChunk2 = (char *) calloc(1, varSize)) == NULL) {
   ;	    printf("Not enough memory to allocate %d bytes (VARS2).\n", varSize);
   ;	    exit(errno);
   ;	  }
   ;	*/
   ;	  dm = varChunk;
   ;	
	mov	ax,word ptr [bp-6]
	mov	word ptr DGROUP:_dm,ax
   ;	
   ;	
   ;	/*  dumpScr();	*/
   ;	
   ;	  doMap();
   ;	
	call	near ptr _doMap
   ;	
   ;	
   ;	  picId = _ROOT_SCREEN_POINTER - 1;	/* ??? */
   ;	
	mov	ax,word ptr DGROUP:__ROOT_SCREEN_POINTER
	dec	ax
	mov	si,ax
   ;	
   ;	  lastPicId = picId - 1;		/* not the same anyway */
   ;	
	mov	ax,si
	dec	ax
	mov	word ptr [bp-2],ax
@2@266:
   ;	
   ;	  while (1) {
   ;	/*
   ;	!	if new pic, get all its global vars
   ;	*/
   ;	    if (picId != lastPicId) {
   ;	
	cmp	si,word ptr [bp-2]
	je	short @2@314
   ;	
   ;		lastPicId = picId;
   ;	
	mov	word ptr [bp-2],si
   ;	
   ;		printTags(picId);
   ;	
	push	si
	call	near ptr _printTags
	pop	cx
@2@314:
   ;	
   ;	    }
   ;	
   ;	    for (c = 0; c < 80; c ++)
   ;	
	xor	di,di
	jmp	short @2@386
@2@338:
   ;	
   ;	      displayBuffer[c] = 0;
   ;	
	mov	bx,word ptr DGROUP:_displayBuffer
	mov	byte ptr [bx+di],0
	inc	di
@2@386:
	cmp	di,80
	jl	short @2@338
   ;	
   ;	
   ;	    (screens[picId].fcn)();
   ;	
	mov	ax,si
	mov	dx,14
	imul	dx
	mov	bx,ax
	call	word ptr DGROUP:_screens[bx+10]
   ;	
   ;	    if (memcmp(displayBuffer, oldBuffer, 80)) {
   ;	
	mov	ax,80
	push	ax
	mov	ax,offset DGROUP:_oldBuffer
	push	ax
	push	word ptr DGROUP:_displayBuffer
	call	near ptr _memcmp
	add	sp,6
	or	ax,ax
	je	short @2@458
   ;	
   ;	      memcpy(oldBuffer, displayBuffer, 80);
   ;	
	mov	ax,80
	push	ax
	push	word ptr DGROUP:_displayBuffer
	mov	ax,offset DGROUP:_oldBuffer
	push	ax
	call	near ptr _memcpy
	add	sp,6
   ;	
   ;	      printf("+----------------------------------------+\n");
   ;	
	mov	ax,offset DGROUP:s@+74
	push	ax
	call	near ptr _printf
	pop	cx
   ;	
   ;	      printf("!%40.40s!\n", displayBuffer);
   ;	
	push	word ptr DGROUP:_displayBuffer
	mov	ax,offset DGROUP:s@+118
	push	ax
	call	near ptr _printf
	pop	cx
	pop	cx
   ;	
   ;	      printf("!%40.40s!\n", &displayBuffer[40]);
   ;	
	mov	ax,word ptr DGROUP:_displayBuffer
	add	ax,40
	push	ax
	mov	ax,offset DGROUP:s@+129
	push	ax
	call	near ptr _printf
	pop	cx
	pop	cx
   ;	
   ;	      printf("+----------------------------------------+\n");
   ;	
	mov	ax,offset DGROUP:s@+140
	push	ax
	call	near ptr _printf
	pop	cx
   ;	
   ;	      printf("\n");
   ;	
	mov	ax,offset DGROUP:s@+184
	push	ax
	call	near ptr _printf
	pop	cx
@2@458:
   ;	
   ;	    }
   ;	/*
   ;	!	check if any vars have been changed
   ;	*/
   ;	
   ;	
   ;	/*
   ;	!	now check if any arrow has been pressed
   ;	*/
   ;	    if (kbhit()) {
   ;	
	call	near ptr _kbhit
	or	ax,ax
	jne	@@0
	jmp	@2@794
@@0:
   ;	
   ;	      c = getch();
   ;	
	call	near ptr _getch
	mov	di,ax
   ;	
   ;	      if (c == 0) {
   ;	
	or	di,di
	je	@@1
	jmp	@2@794
@@1:
   ;	
   ;		c = getch();
   ;	
	call	near ptr _getch
	mov	di,ax
   ;	
   ;		if (c == 72 && screens[picId].up)
   ;	
	cmp	di,72
	jne	short @2@578
	mov	ax,si
	mov	dx,14
	imul	dx
	mov	bx,ax
	cmp	word ptr DGROUP:_screens[bx+4],0
	je	short @2@578
   ;	
   ;		  picId = screens[picId].up - 1;
   ;	
	mov	ax,si
	mov	dx,14
	imul	dx
	mov	bx,ax
	mov	ax,word ptr DGROUP:_screens[bx+4]
	dec	ax
	mov	si,ax
@2@578:
   ;	
   ;		if (c == 77 && screens[picId].right)
   ;	
	cmp	di,77
	jne	short @2@650
	mov	ax,si
	mov	dx,14
	imul	dx
	mov	bx,ax
	cmp	word ptr DGROUP:_screens[bx+2],0
	je	short @2@650
   ;	
   ;		  picId = screens[picId].right - 1;
   ;	
	mov	ax,si
	mov	dx,14
	imul	dx
	mov	bx,ax
	mov	ax,word ptr DGROUP:_screens[bx+2]
	dec	ax
	mov	si,ax
@2@650:
   ;	
   ;		if (c == 80 && screens[picId].down)
   ;	
	cmp	di,80
	jne	short @2@722
	mov	ax,si
	mov	dx,14
	imul	dx
	mov	bx,ax
	cmp	word ptr DGROUP:_screens[bx+6],0
	je	short @2@722
   ;	
   ;		  picId = screens[picId].down - 1;
   ;	
	mov	ax,si
	mov	dx,14
	imul	dx
	mov	bx,ax
	mov	ax,word ptr DGROUP:_screens[bx+6]
	dec	ax
	mov	si,ax
@2@722:
   ;	
   ;		if (c == 75 && screens[picId].left)
   ;	
	cmp	di,75
	jne	short @2@794
	mov	ax,si
	mov	dx,14
	imul	dx
	mov	bx,ax
	cmp	word ptr DGROUP:_screens[bx],0
	je	short @2@794
   ;	
   ;		  picId = screens[picId].left - 1;
   ;	
	mov	ax,si
	mov	dx,14
	imul	dx
	mov	bx,ax
	mov	ax,word ptr DGROUP:_screens[bx]
	dec	ax
	mov	si,ax
@2@794:
	jmp	@2@266
   ;	
   ;	      }
   ;	    }
   ;	  }
   ;	
   ;	  free(metaModHeader);
   ;	
	push	word ptr DGROUP:_metaModHeader
	call	near ptr _free
	pop	cx
   ;	
   ;	  return 0;
   ;	
	xor	ax,ax
	jmp	short @2@842
@2@842:
   ;	
   ;	}
   ;	
	pop	di
	pop	si
	mov	sp,bp
	pop	bp
	ret	
_main	endp
   ;	
   ;	static int getFileSize(char *s)
   ;	
	assume	cs:_TEXT
getFileSize	proc	near
	push	bp
	mov	bp,sp
	sub	sp,44
   ;	
   ;	{
   ;	   struct ffblk ffblk1;
   ;	   if (findfirst(s,&ffblk1,0)) {
   ;	
	xor	ax,ax
	push	ax
	lea	ax,word ptr [bp-44]
	push	ax
	push	word ptr [bp+4]
	call	near ptr _findfirst
	add	sp,6
	or	ax,ax
	je	short @3@74
   ;	
   ;	     return 0;
   ;	
	xor	ax,ax
	jmp	short @3@98
@3@74:
   ;	
   ;	   }
   ;	   return ffblk1.ff_fsize;
   ;	
	mov	ax,word ptr [bp-18]
	jmp	short @3@98
@3@98:
   ;	
   ;	}
   ;	
	mov	sp,bp
	pop	bp
	ret	
getFileSize	endp
   ;	
   ;	void readMetaMod(char *name)
   ;	
	assume	cs:_TEXT
_readMetaMod	proc	near
	push	bp
	mov	bp,sp
	push	si
	push	di
   ;	
   ;	{
   ;	  int siz;
   ;	  FILE *fp;
   ;	  if (!(siz = getFileSize(name))) {
   ;	
	push	word ptr [bp+4]
	call	near ptr getFileSize
	pop	cx
	mov	di,ax
	or	ax,ax
	jne	short @4@74
   ;	
   ;	     printf("Sorry, no such file as '%s'\n", name);
   ;	
	push	word ptr [bp+4]
	mov	ax,offset DGROUP:s@+186
	push	ax
	call	near ptr _printf
	pop	cx
	pop	cx
   ;	
   ;	     exit(1);
   ;	
	mov	ax,1
	push	ax
	call	near ptr _exit
	pop	cx
@4@74:
   ;	
   ;	  }
   ;	  if (!(metaModHeader = (char *) malloc(siz))) {
   ;	
	push	di
	call	near ptr _malloc
	pop	cx
	mov	word ptr DGROUP:_metaModHeader,ax
	or	ax,ax
	jne	short @4@122
   ;	
   ;	    printf("not enough memory\n");
   ;	
	mov	ax,offset DGROUP:s@+215
	push	ax
	call	near ptr _printf
	pop	cx
   ;	
   ;	    exit(1);
   ;	
	mov	ax,1
	push	ax
	call	near ptr _exit
	pop	cx
@4@122:
   ;	
   ;	  }
   ;	  if ((fp = fopen(name, "rb")) <= 0) {
   ;	
	mov	ax,offset DGROUP:s@+234
	push	ax
	push	word ptr [bp+4]
	call	near ptr _fopen
	pop	cx
	pop	cx
	mov	si,ax
	or	ax,ax
	ja	short @4@170
   ;	
   ;	    printf("Error opening '%s'\n", name);
   ;	
	push	word ptr [bp+4]
	mov	ax,offset DGROUP:s@+237
	push	ax
	call	near ptr _printf
	pop	cx
	pop	cx
   ;	
   ;	    exit(1);
   ;	
	mov	ax,1
	push	ax
	call	near ptr _exit
	pop	cx
@4@170:
   ;	
   ;	  }
   ;	  fread(metaModHeader, siz, 1, fp);
   ;	
	push	si
	mov	ax,1
	push	ax
	push	di
	push	word ptr DGROUP:_metaModHeader
	call	near ptr _fread
	add	sp,8
   ;	
   ;	  metaMod = metaModHeader + sizeof(struct _metaModHeader);
   ;	
	mov	ax,word ptr DGROUP:_metaModHeader
	add	ax,64
	mov	word ptr DGROUP:_metaMod,ax
   ;	
   ;	  fclose(fp);
   ;	
	push	si
	call	near ptr _fclose
	pop	cx
   ;	
   ;	}
   ;	
	pop	di
	pop	si
	pop	bp
	ret	
_readMetaMod	endp
   ;	
   ;	unsigned short swapword(unsigned short w)
   ;	
	assume	cs:_TEXT
_swapword	proc	near
	push	bp
	mov	bp,sp
   ;	
   ;	{
   ;	  return ((w & 0xff) << 8) | ((w >> 8) & 0xff);
   ;	
	mov	ax,word ptr [bp+4]
	and	ax,255
	mov	cl,8
	shl	ax,cl
	mov	dx,word ptr [bp+4]
	mov	cl,8
	shr	dx,cl
	and	dx,255
	or	ax,dx
	jmp	short @5@50
@5@50:
   ;	
   ;	}
   ;	
	pop	bp
	ret	
_swapword	endp
   ;	
   ;	unsigned long swaplong(unsigned long l)
   ;	
	assume	cs:_TEXT
_swaplong	proc	near
	push	bp
	mov	bp,sp
	push	si
	push	di
   ;	
   ;	{
   ;	  unsigned short w1, w2;
   ;	  w1 = (l >> 16) & 0xffff;
   ;	
	mov	ax,word ptr [bp+6]
	and	ax,65535
	mov	di,ax
   ;	
   ;	  w2 = l & 0xffff;
   ;	
	mov	ax,word ptr [bp+4]
	and	ax,65535
	mov	si,ax
   ;	
   ;	  return (((unsigned long) swapword(w2)) << 16) | swapword(w1);
   ;	
	push	si
	call	near ptr _swapword
	pop	cx
	push	ax
	xor	ax,ax
	push	ax
	push	di
	call	near ptr _swapword
	pop	cx
	mov	dx,ax
	pop	ax
	or	ax,dx
	pop	dx
	or	dx,0
	jmp	short @6@50
@6@50:
   ;	
   ;	}
   ;	
	pop	di
	pop	si
	pop	bp
	ret	
_swaplong	endp
   ;	
   ;	void swapMetaMod(char *metaMod)
   ;	
	assume	cs:_TEXT
_swapMetaMod	proc	near
	push	bp
	mov	bp,sp
	dec	sp
	dec	sp
	push	si
	push	di
   ;	
   ;	{
   ;	  int id, max;
   ;	  struct _metaEntry {
   ;	     unsigned short nameOffset, size, offset, lockOffset, type;
   ;	  } *metaEntry;
   ;	
   ;	  metaEntry = (struct _metaEntry *) metaMod;
   ;	
	mov	si,word ptr [bp+4]
   ;	
   ;	  max = metaEntry->nameOffset = swapword(metaEntry->nameOffset);
   ;	
	push	word ptr [si]
	call	near ptr _swapword
	pop	cx
	mov	word ptr [si],ax
	mov	word ptr [bp-2],ax
   ;	
   ;	  id = 1;
   ;	
	mov	di,1
	jmp	short @7@74
@7@50:
   ;	
   ;	  while (id++ <= max) {
   ;	    metaEntry++;
   ;	
	add	si,10
   ;	
   ;	
   ;	    metaEntry->nameOffset = swapword(metaEntry->nameOffset);
   ;	
	push	word ptr [si]
	call	near ptr _swapword
	pop	cx
	mov	word ptr [si],ax
   ;	
   ;	    metaEntry->size       = swapword(metaEntry->size);
   ;	
	push	word ptr [si+2]
	call	near ptr _swapword
	pop	cx
	mov	word ptr [si+2],ax
   ;	
   ;	    metaEntry->offset     = swapword(metaEntry->offset);
   ;	
	push	word ptr [si+4]
	call	near ptr _swapword
	pop	cx
	mov	word ptr [si+4],ax
   ;	
   ;	    metaEntry->lockOffset = swapword(metaEntry->lockOffset);
   ;	
	push	word ptr [si+6]
	call	near ptr _swapword
	pop	cx
	mov	word ptr [si+6],ax
   ;	
   ;	    metaEntry->type       = swapword(metaEntry->type);
   ;	
	push	word ptr [si+8]
	call	near ptr _swapword
	pop	cx
	mov	word ptr [si+8],ax
@7@74:
	mov	ax,di
	inc	di
	cmp	ax,word ptr [bp-2]
	jle	short @7@50
   ;	
   ;	  }
   ;	}
   ;	
	pop	di
	pop	si
	mov	sp,bp
	pop	bp
	ret	
_swapMetaMod	endp
_TEXT	ends
_BSS	segment word public 'BSS'
_hilo	label	word
	db	2 dup (?)
_oldBuffer	label	byte
	db	80 dup (?)
_metaMod	label	word
	db	2 dup (?)
_metaModHeader	label	word
	db	2 dup (?)
	?debug	C E9
_BSS	ends
_DATA	segment word public 'DATA'
s@	label	byte
	db	'Usage: win3208 <metavar>'
	db	10
	db	0
	db	'Not enough memory to allocate %d bytes (VARS).'
	db	10
	db	0
	db	'+----------------------------------------+'
	db	10
	db	0
	db	'!%40.40s!'
	db	10
	db	0
	db	'!%40.40s!'
	db	10
	db	0
	db	'+----------------------------------------+'
	db	10
	db	0
	db	10
	db	0
	db	'Sorry, no such file as '
	db	39
	db	'%s'
	db	39
	db	10
	db	0
	db	'not enough memory'
	db	10
	db	0
	db	'rb'
	db	0
	db	'Error opening '
	db	39
	db	'%s'
	db	39
	db	10
	db	0
_DATA	ends
_TEXT	segment byte public 'CODE'
_TEXT	ends
_getFileSize	equ	getFileSize
	extrn	_printTags:near
	public	_metaModHeader
	extrn	_fopen:near
	extrn	_displayBuffer:word
	extrn	_doMap:near
	public	_readMetaMod
	extrn	_printf:near
	extrn	_metaRemote:near
	extrn	_findfirst:near
	extrn	_metaSize:near
	extrn	_errno:word
	extrn	_kbhit:near
	extrn	_metaName:near
	extrn	_fread:near
	public	_metaMod
	extrn	_free:near
	extrn	_memcpy:near
	extrn	_malloc:near
	extrn	_calloc:near
	extrn	_memcmp:near
	public	_oldBuffer
	public	_hilo
	extrn	_getch:near
	public	_main
	public	_swapMetaMod
	extrn	__ROOT_SCREEN_POINTER:word
	public	_getTotalSizeOfVars
	extrn	_dm:word
	public	_swapword
	public	_swaplong
	extrn	__setargv__:far
	extrn	_exit:near
	extrn	_screens:word
	extrn	_fclose:near
	end
