	.586p
	ifdef ??version
	if    ??version GT 500H
	.mmx
	endif
	endif
	model flat
	ifndef	??version
	?debug	macro
	endm
	endif
	?debug	S "..\routines\LoadImage.cpp"
	?debug	T "..\routines\LoadImage.cpp"
_TEXT	segment dword public use32 'CODE'
_TEXT	ends
_DATA	segment dword public use32 'DATA'
_DATA	ends
_BSS	segment dword public use32 'BSS'
_BSS	ends
 ?debug  C FB0C0100600000
DGROUP	group	_BSS,_DATA
_DATA	segment dword public use32 'DATA'
	align	4
_files	label	dword
	dd	0
_DATA	ends
_TEXT	segment dword public use32 'CODE'
@InitLoadImage$qp10FileOpener	segment virtual
	align	2
@@InitLoadImage$qp10FileOpener	proc	near
?live16385@0:
@1:
	push      ebp
	mov       ebp,esp
	push      ebx
	mov       ebx,dword ptr [ebp+8]
?live16385@16: ; EBX = _files
	test      ebx,ebx
	jne       short @3
	push      21
	push      offset s@+15
	push      offset s@
	call      @__assert
	add       esp,12
@3:
	mov       dword ptr [_files],ebx
?live16385@48: ; 
@4:
	pop       ebx
	pop       ebp
	ret 
@@InitLoadImage$qp10FileOpener	endp
@InitLoadImage$qp10FileOpener	ends
_TEXT	ends
_DATA	segment dword public use32 'DATA'
@_$DCLCC$@LoadImageA$q60std@%basic_string$c19std@%char_traits$c%17std@%allocator$c%%	segment virtual
	align	2
@@_$DCLCC$@LoadImageA$q60std@%basic_string$c19std@%char_traits$c%17std@%allocator$c%%	label	dword
	dd	@@$xt$60std@%basic_string$c19std@%char_traits$c%17std@%allocator$c%%
	dd	4
	dd	8
	dd	0
	dd	@@$xt$60std@%basic_string$c19std@%char_traits$c%17std@%allocator$c%%
	dd	4
	dd	-16
	dd	0
	dd	@@$xt$60std@%basic_string$c19std@%char_traits$c%17std@%allocator$c%%
	dd	4
	dd	?t2
	dd	0
@_$DCLCC$@LoadImageA$q60std@%basic_string$c19std@%char_traits$c%17std@%allocator$c%%	ends
_DATA	ends
_DATA	segment dword public use32 'DATA'
@_$ECTMCC$@LoadImageA$q60std@%basic_string$c19std@%char_traits$c%17std@%allocator$c%%	segment virtual
	align	2
@@_$ECTMCC$@LoadImageA$q60std@%basic_string$c19std@%char_traits$c%17std@%allocator$c%%	label	dword
	dd	0
	dd	-52
	dw	0
	dw	5
	dd	0
	dd	@@_$DCLCC$@LoadImageA$q60std@%basic_string$c19std@%char_traits$c%17std@%allocator$c%%
	dw	8
	dw	5
	dd	1
	dd	@@_$DCLCC$@LoadImageA$q60std@%basic_string$c19std@%char_traits$c%17std@%allocator$c%%+16
	dw	20
	dw	5
	dd	1
	dd	@@_$DCLCC$@LoadImageA$q60std@%basic_string$c19std@%char_traits$c%17std@%allocator$c%%+32
	dw	32
	dw	5
	dd	0
	dd	0
	dw	20
	dw	5
	dd	0
	dd	0
	dw	8
	dw	5
	dd	0
	dd	0
@_$ECTMCC$@LoadImageA$q60std@%basic_string$c19std@%char_traits$c%17std@%allocator$c%%	ends
_DATA	ends
_TEXT	segment dword public use32 'CODE'
@LoadImageA$q60std@%basic_string$c19std@%char_traits$c%17std@%allocator$c%%	segment virtual
	align	2
@@LoadImageA$q60std@%basic_string$c19std@%char_traits$c%17std@%allocator$c%%	proc	near
?live16386@0:
@5:
	push      ebp
	mov       ebp,esp
	add       esp,-184
	mov       eax,offset @@_$ECTMCC$@LoadImageA$q60std@%basic_string$c19std@%char_traits$c%17std@%allocator$c%%
	push      ebx
	push      esi
	push      edi
	call      @__InitExceptBlockLDTC
	mov       dword ptr [ebp-24],1
	mov       word ptr [ebp-36],8
	mov       edx,dword ptr [_files]
	test      edx,edx
	jne       short @7
	push      26
	push      offset s@+55
	push      offset s@+41
	call      @__assert
	add       esp,12
@7:
	mov       word ptr [ebp-36],20
	push      0
	add       esp,-16
	mov       word ptr [ebp-36],32
	xor       ecx,ecx
	mov       dword ptr [ebp-60],ecx
	mov       eax,dword ptr [ebp-60]
	mov       dword ptr [esp+8],eax
	mov       edx,dword ptr [ebp+16]
	mov       ecx,dword ptr [edx-12]
	inc       ecx
	test      ecx,ecx
	jle       short @8
	mov       eax,dword ptr [ebp+16]
	mov       ecx,esp
	mov       dword ptr [ebp-64],eax
	mov       edx,dword ptr [ebp-64]
	mov       dword ptr [esp+8],edx
	mov       dword ptr [ebp-68],ecx
	mov       eax,dword ptr [ebp-68]
	mov       edx,dword ptr [eax+8]
	mov       dword ptr [ebp-128],edx
	mov       esi,dword ptr [esp+8]
	add       esi,-12
	cmp       esi,offset @@std@%basic_string$c19std@%char_traits$c%17std@%allocator$c%%@__nullref
	je        short @9
	inc       dword ptr [esi]
	jmp       short @9
@8:
	mov       eax,dword ptr [ebp+16]
	mov       ebx,dword ptr [eax-4]
	push      ebx
	push      ebx
	lea       edx,dword ptr [esp+8]
	push      edx
	call      @@std@%basic_string$c19std@%char_traits$c%17std@%allocator$c%%@__getRep$quiui
	mov       dword ptr [ebp-76],eax
	add       esp,12
	mov       ecx,dword ptr [ebp-76]
	mov       edx,esp
	add       ecx,12
	mov       dword ptr [ebp-72],ecx
	mov       eax,dword ptr [ebp-72]
	mov       dword ptr [esp+8],eax
	mov       dword ptr [ebp-80],edx
	mov       ecx,dword ptr [ebp-80]
	mov       eax,dword ptr [ecx+8]
	mov       dword ptr [ebp-144],eax
	mov       dword ptr [ebp-84],ebx
	mov       edx,dword ptr [ebp+16]
	mov       dword ptr [ebp-88],edx
	mov       edi,dword ptr [esp+8]
	mov       ecx,dword ptr [ebp-84]
	push      ecx
	mov       eax,dword ptr [ebp-88]
	push      eax
	push      edi
	call      @_memcpy
	add       esp,12
@9:
	inc       dword ptr [ebp-24]
	mov       edx,dword ptr [_files]
	push      edx
	lea       ecx,dword ptr [ebp-16]
	push      ecx
	mov       word ptr [ebp-36],20
	dec       dword ptr [ebp-24]
	call      @@FileOpener@FilenameForRead$q60std@%basic_string$c19std@%char_traits$c%17std@%allocator$c%%o
	add       esp,28
	lea       eax,dword ptr [ebp-16]
	inc       dword ptr [ebp-24]
	mov       dword ptr [ebp-56],eax
	mov       edx,dword ptr [ebp-56]
	mov       ecx,dword ptr [edx+8]
	mov       dword ptr [ebp-92],ecx
	push      512
	push      256
	mov       eax,dword ptr [ebp-92]
	push      eax
	call      @CorOpenImage
	push      eax
	call      @CorConvertImage
	mov       ebx,eax
	dec       dword ptr [ebp-24]
	mov       edx,dword ptr [ebp-8]
	test      edx,edx
	je        short @13
	mov       ecx,dword ptr [ebp-8]
	mov       eax,dword ptr [ecx-12]
	inc       eax
	test      eax,eax
	je        short @16
	mov       esi,dword ptr [ebp-8]
	add       esi,-12
	cmp       esi,offset @@std@%basic_string$c19std@%char_traits$c%17std@%allocator$c%%@__nullref
	je        short @17
	cmp       esi,offset @@std@%basic_string$c19std@%char_traits$c%17std@%allocator$c%%@__nullref
	je        short @19
	dec       dword ptr [esi]
	mov       eax,dword ptr [esi]
	inc       eax
	jmp       short @18
@19:
	mov       eax,1
	jmp       short @18
@17:
	mov       eax,1
@18:
	test      eax,eax
	jne       short @14
@16:
	mov       edx,dword ptr [ebp-8]
	add       edx,-12
	mov       dword ptr [ebp-96],edx
	mov       edi,dword ptr [ebp-8]
	add       edi,-12
	push      edi
	call      @$bdele$qpv
	pop       ecx
	xor       eax,eax
	mov       dword ptr [ebp-100],eax
	lea       ecx,dword ptr [ebp-16]
	mov       edx,dword ptr [ebp-100]
	mov       dword ptr [ebp-8],edx
	mov       dword ptr [ebp-104],ecx
	mov       eax,dword ptr [ebp-104]
	mov       edx,dword ptr [eax+8]
	mov       dword ptr [ebp-160],edx
@14:
@15:
@13:
	mov       word ptr [ebp-36],8
?live16386@48: ; EBX = image
	test      ebx,ebx
	je        short @22
	mov       eax,ebx
	push      1
	push      eax
	call      @CorFlipImage
@23:
	push      ebx
	mov       edx,dword ptr [ebx]
	call      dword ptr [edx+12]
	and       eax,65535
?live16386@96: ; EBX = image, EAX = format
	add       eax,-513
	sub       eax,2
	jb        short @28
	je        short @26
	dec       eax
	je        short @27
	dec       eax
	je        short @26
	jmp       short @24
?live16386@112: ; EBX = image
@28:
	mov       dword ptr [ebp-108],512
	jmp       short @30
@26:
	mov       dword ptr [ebp-108],514
	jmp       short @30
@27:
	mov       dword ptr [ebp-108],513
@24:
@30:
	mov       ecx,dword ptr [ebp-108]
	cmp       ecx,512
	je        short @31
	mov       esi,dword ptr [ebp-108]
	mov       edi,ebx
	push      esi
	push      edi
	call      @CorConvertImage
	mov       ebx,eax
@31:
@32:
@22:
	mov       eax,ebx
	push      eax
	dec       dword ptr [ebp-24]
	mov       edx,dword ptr [ebp+16]
	test      edx,edx
	je        short @34
	mov       ecx,dword ptr [ebp+16]
	mov       eax,dword ptr [ecx-12]
	inc       eax
	test      eax,eax
	je        short @37
	mov       esi,dword ptr [ebp+16]
	add       esi,-12
	cmp       esi,offset @@std@%basic_string$c19std@%char_traits$c%17std@%allocator$c%%@__nullref
	je        short @38
	cmp       esi,offset @@std@%basic_string$c19std@%char_traits$c%17std@%allocator$c%%@__nullref
	je        short @40
	dec       dword ptr [esi]
	mov       eax,dword ptr [esi]
	inc       eax
	jmp       short @39
@40:
	mov       eax,1
	jmp       short @39
@38:
	mov       eax,1
@39:
	test      eax,eax
	jne       short @35
@37:
	mov       edx,dword ptr [ebp+16]
	add       edx,-12
	mov       dword ptr [ebp-112],edx
	mov       edi,dword ptr [ebp+16]
	add       edi,-12
	push      edi
	call      @$bdele$qpv
	pop       ecx
	xor       eax,eax
	mov       dword ptr [ebp-116],eax
	lea       ecx,dword ptr [ebp+8]
	mov       edx,dword ptr [ebp-116]
	mov       dword ptr [ebp+16],edx
	mov       dword ptr [ebp-120],ecx
	mov       eax,dword ptr [ebp-120]
	mov       edx,dword ptr [eax+8]
	mov       dword ptr [ebp-176],edx
@35:
@36:
@34:
	pop       eax
	mov       edx,dword ptr [ebp-52]
	mov       dword ptr fs:[0],edx
?live16386@240: ; 
@43:
@42:
	pop       edi
	pop       esi
	pop       ebx
	mov       esp,ebp
	pop       ebp
	ret 
?t2 = -216
@@LoadImageA$q60std@%basic_string$c19std@%char_traits$c%17std@%allocator$c%%	endp
@LoadImageA$q60std@%basic_string$c19std@%char_traits$c%17std@%allocator$c%%	ends
_TEXT	ends
_TEXT	segment dword public use32 'CODE'
@ResizeImageForOpenGL$qp12corona@Image	segment virtual
	align	2
@@ResizeImageForOpenGL$qp12corona@Image	proc	near
?live16390@0:
	push      ebp
	mov       ebp,esp
	add       esp,-56
	push      ebx
	push      esi
	push      edi
@44:
	mov       eax,dword ptr [ebp+8]
	push      eax
	mov       edx,dword ptr [eax]
	call      dword ptr [edx+4]
	mov       dword ptr [ebp-4],eax
	mov       ecx,dword ptr [ebp+8]
	push      ecx
	mov       eax,dword ptr [ecx]
	call      dword ptr [eax+8]
	mov       dword ptr [ebp-8],eax
	mov       edx,dword ptr [ebp+8]
	push      edx
	mov       ecx,dword ptr [edx]
	call      dword ptr [ecx+16]
	mov       dword ptr [ebp-12],eax
	mov       dword ptr [ebp-16],1
	mov       dword ptr [ebp-20],1
	mov       eax,dword ptr [ebp-16]
	mov       edx,dword ptr [ebp-4]
	cmp       eax,edx
	jge       short @46
@45:
	shl       dword ptr [ebp-16],1
	mov       ecx,dword ptr [ebp-16]
	mov       eax,dword ptr [ebp-4]
	cmp       ecx,eax
	jl        short @45
@46:
	mov       edx,dword ptr [ebp-20]
	mov       ecx,dword ptr [ebp-8]
	cmp       edx,ecx
	jge       short @48
@47:
	shl       dword ptr [ebp-20],1
	mov       eax,dword ptr [ebp-20]
	mov       edx,dword ptr [ebp-8]
	cmp       eax,edx
	jl        short @47
@48:
	mov       ecx,dword ptr [ebp-16]
	mov       eax,dword ptr [ebp-4]
	cmp       ecx,eax
	jne       short @49
	mov       edx,dword ptr [ebp-20]
	mov       ecx,dword ptr [ebp-8]
	cmp       edx,ecx
	jne       short @49
	mov       eax,dword ptr [ebp+8]
	jmp       @50
@49:
	mov       ebx,dword ptr [ebp+8]
	test      ebx,ebx
	jne       short @54
@53:
	push      27
	push      offset s@+95
	push      offset s@+81
	call      @__assert
	add       esp,12
@54:
	push      ebx
	mov       edx,dword ptr [ebx]
	call      dword ptr [edx+12]
	and       eax,65535
	cmp       eax,514
	jne       short @55
	mov       ecx,6407
	jmp       short @56
@55:
	mov       ecx,6408
@56:
	cmp       ecx,6407
	jne       short @51
	mov       eax,3
	jmp       short @52
@51:
	mov       eax,4
@52:
	mov       dword ptr [ebp-24],eax
	mov       ecx,dword ptr [ebp-16]
	mov       edx,dword ptr [ebp-4]
	sub       ecx,edx
	mov       eax,ecx
	cdq
	xor       eax,edx
	sub       eax,edx
	mov       ecx,eax
	mov       ebx,dword ptr [ebp-16]
	sar       ebx,1
	mov       eax,dword ptr [ebp-4]
	sub       ebx,eax
	mov       eax,ebx
	cdq
	xor       eax,edx
	sub       eax,edx
	cmp       ecx,eax
	jle       short @57
	sar       dword ptr [ebp-16],1
@57:
	mov       ecx,dword ptr [ebp-20]
	mov       eax,dword ptr [ebp-8]
	sub       ecx,eax
	mov       eax,ecx
	cdq
	xor       eax,edx
	sub       eax,edx
	mov       ecx,eax
	mov       ebx,dword ptr [ebp-20]
	sar       ebx,1
	mov       eax,dword ptr [ebp-8]
	sub       ebx,eax
	mov       eax,ebx
	cdq
	xor       eax,edx
	sub       eax,edx
	cmp       ecx,eax
	jle       short @58
	sar       dword ptr [ebp-20],1
@58:
	mov       ecx,dword ptr [ebp+8]
	push      ecx
	mov       eax,dword ptr [ecx]
	call      dword ptr [eax+12]
	mov       ebx,eax
	mov       esi,dword ptr [ebp-20]
	mov       edi,dword ptr [ebp-16]
	push      0
	push      ebx
	push      esi
	push      edi
	call      @CorCreateImageWithPixels
	mov       dword ptr [ebp-28],eax
	mov       edx,dword ptr [ebp-28]
	push      edx
	mov       ecx,dword ptr [edx]
	call      dword ptr [ecx+16]
	mov       dword ptr [ebp-32],eax
	pushad	
	mov	     eax,        [dword ptr [ebp-24]]
	mov	     [dword ptr [ebp-52]],    eax
	mov	     eax,        [dword ptr [ebp-24]]
	mul	     [dword ptr [ebp-4]]
	mov	     [dword ptr [ebp-56]],    eax
	xor	     edx,        edx             
	mov	     eax,        [dword ptr [ebp-4]]
	div	     [dword ptr [ebp-16]]
	push	    edx                         
	mul	     [dword ptr [ebp-52]]
	mov	     [dword ptr [ebp-36]],    eax
	pop	     edx
	xor	     eax,        eax             
	div	     [dword ptr [ebp-16]]						
	mov	     [dword ptr [ebp-40]],   eax
	xor	     edx,        edx             
	mov	     eax,        [dword ptr [ebp-8]]
	div	     [dword ptr [ebp-20]]
	push	    edx                         
	mul	     [dword ptr [ebp-56]]
	mov	     [dword ptr [ebp-44]],    eax
	pop	     edx
	xor	     eax,        eax             
	div	     [dword ptr [ebp-20]]					
	mov	     [dword ptr [ebp-48]],   eax
	mov	     esi,        [dword ptr [ebp-12]]       
	mov	     edi,        [dword ptr [ebp-32]]        
	xor	     ebx,        ebx             
	xor	     edx,        edx             
	mov	     ecx,        [dword ptr [ebp-20]]        
	jecxz     @59
@60:
YLoop:
	push	    ecx
	push	    esi
	mov	     ecx,        [dword ptr [ebp-16]]
	jecxz     @61
	mov	     eax,        3
	cmp	     eax,        [dword ptr [ebp-24]]
	jne       @62
@63:
XLoop3:
	mov	     ax,         [esi]           
	mov	     [edi],      ax
	mov	     al,         [esi + 2]
	mov	     [edi + 2],  al
	add	     edi,        3
	add	     ebx,        [dword ptr [ebp-40]]
	jae       short @64
	add	     esi,        [dword ptr [ebp-52]]
@64:
NoXCarry3:
	add	     esi,        [dword ptr [ebp-36]]
	dec	     ecx
	jne       @63
	jmp       @61
@62:
XLoop4:
	mov	     eax,        [esi]           
	mov	     [edi],      eax
	add	     edi,        4
	add	     ebx,        [dword ptr [ebp-40]]
	jae       short @65
	add	     esi,        [dword ptr [ebp-52]]
@65:
NoXCarry4:
	add	     esi,        [dword ptr [ebp-36]]
	dec	     ecx
	jne       @62
@61:
XLoopDone:
	pop	     esi
	pop	     ecx
	add	     edx,        [dword ptr [ebp-48]]
	jae       short @66
	add	     esi,        [dword ptr [ebp-56]]
@66:
NoYCarry:
	add	     esi,        [dword ptr [ebp-44]]
	dec	     ecx
	jne       @60
@59:
YLoopDone:
	popad	
	mov       ebx,dword ptr [ebp+8]
	test      ebx,ebx
	je        short @67
	mov       esi,ebx
	push      esi
	mov       eax,dword ptr [esi]
	call      dword ptr [eax]
@67:
@68:
	mov       eax,dword ptr [ebp-28]
@69:
@50:
	pop       edi
	pop       esi
	pop       ebx
	mov       esp,ebp
	pop       ebp
	ret 
@@ResizeImageForOpenGL$qp12corona@Image	endp
@ResizeImageForOpenGL$qp12corona@Image	ends
_TEXT	ends
_TEXT	segment dword public use32 'CODE'
@$xt$60std@%basic_string$c19std@%char_traits$c%17std@%allocator$c%%	segment virtual
	align	2
@@$xt$60std@%basic_string$c19std@%char_traits$c%17std@%allocator$c%%	label	byte
	dd	16
	dw	3
	dw	48
	dd	-1
	dd	3
	dw	120
	dw	124
	dd	0
	dw	0
	dw	0
	dd	0
	dd	1
	dd	1
	dd	@@std@%basic_string$c19std@%char_traits$c%17std@%allocator$c%%@$bdtr$qv
	dw	1
	dw	128
	db	115
	db	116
	db	100
	db	58
	db	58
	db	98
	db	97
	db	115
	db	105
	db	99
	db	95
	db	115
	db	116
	db	114
	db	105
	db	110
	db	103
	db	60
	db	99
	db	104
	db	97
	db	114
	db	44
	db	115
	db	116
	db	100
	db	58
	db	58
	db	99
	db	104
	db	97
	db	114
	db	95
	db	116
	db	114
	db	97
	db	105
	db	116
	db	115
	db	60
	db	99
	db	104
	db	97
	db	114
	db	62
	db	44
	db	115
	db	116
	db	100
	db	58
	db	58
	db	97
	db	108
	db	108
	db	111
	db	99
	db	97
	db	116
	db	111
	db	114
	db	60
	db	99
	db	104
	db	97
	db	114
	db	62
	db	32
	db	62
	db	0
	db	0
	db	0
	db	0
	dd	0
	dd	0
	dd	0
@$xt$60std@%basic_string$c19std@%char_traits$c%17std@%allocator$c%%	ends
_TEXT	ends
_TEXT	segment dword public use32 'CODE'
@std@%basic_string$c19std@%char_traits$c%17std@%allocator$c%%@$bdtr$qv	segment virtual
	align	2
@@std@%basic_string$c19std@%char_traits$c%17std@%allocator$c%%@$bdtr$qv	proc	near
?live16391@0:
@70:
	push      ebp
	mov       ebp,esp
	add       esp,-28
	push      ebx
	push      esi
	push      edi
	mov       ebx,dword ptr [ebp+8]
	test      ebx,ebx
	je        short @71
?live16391@16: ; EBX = this
	mov       eax,dword ptr [ebx+8]
	test      eax,eax
	je        short @73
	mov       edx,dword ptr [ebx+8]
	mov       ecx,dword ptr [edx-12]
	inc       ecx
	test      ecx,ecx
	je        short @76
	mov       esi,dword ptr [ebx+8]
	add       esi,-12
	cmp       esi,offset @@std@%basic_string$c19std@%char_traits$c%17std@%allocator$c%%@__nullref
	je        short @77
	cmp       esi,offset @@std@%basic_string$c19std@%char_traits$c%17std@%allocator$c%%@__nullref
	je        short @79
	dec       dword ptr [esi]
	mov       eax,dword ptr [esi]
	inc       eax
	jmp       short @78
@79:
	mov       eax,1
	jmp       short @78
@77:
	mov       eax,1
@78:
	test      eax,eax
	jne       short @74
@76:
	mov       edx,dword ptr [ebx+8]
	add       edx,-12
	mov       dword ptr [ebp-4],edx
	mov       edi,dword ptr [ebx+8]
	add       edi,-12
	push      edi
	call      @$bdele$qpv
	pop       ecx
	xor       eax,eax
	mov       dword ptr [ebp-8],eax
	mov       edx,dword ptr [ebp-8]
	mov       dword ptr [ebx+8],edx
	mov       dword ptr [ebp-12],ebx
	mov       ecx,dword ptr [ebp-12]
	mov       eax,dword ptr [ecx+8]
	mov       dword ptr [ebp-20],eax
@74:
@75:
@73:
	test      byte ptr [ebp+12],1
	je        short @81
	push      ebx
	call      @$bdele$qpv
	pop       ecx
?live16391@32: ; 
@81:
@71:
@82:
	pop       edi
	pop       esi
	pop       ebx
	mov       esp,ebp
	pop       ebp
	ret 
@@std@%basic_string$c19std@%char_traits$c%17std@%allocator$c%%@$bdtr$qv	endp
@std@%basic_string$c19std@%char_traits$c%17std@%allocator$c%%@$bdtr$qv	ends
_TEXT	ends
_DATA	segment dword public use32 'DATA'
@std@locale@numeric	segment virtual
	align	2
@@std@locale@numeric	label	dword
	dd	128
@std@locale@numeric	ends
_DATA	ends
_DATA	segment dword public use32 'DATA'
@std@locale@__rw_ctype_category	segment virtual
	align	2
@@std@locale@__rw_ctype_category	label	dword
	dd	32
@std@locale@__rw_ctype_category	ends
_DATA	ends
_DATA	segment dword public use32 'DATA'
@std@%ctype$c%@table_size	segment virtual
	align	2
@@std@%ctype$c%@table_size	label	dword
	dd	256
@std@%ctype$c%@table_size	ends
_DATA	ends
_DATA	segment dword public use32 'DATA'
@std@%numeric_limits$i%@radix	segment virtual
	align	2
@@std@%numeric_limits$i%@radix	label	dword
	dd	2
@std@%numeric_limits$i%@radix	ends
_DATA	ends
_DATA	segment dword public use32 'DATA'
@std@%numeric_limits$i%@is_modulo	segment virtual
@@std@%numeric_limits$i%@is_modulo	label	byte
	db	0
@std@%numeric_limits$i%@is_modulo	ends
_DATA	ends
_DATA	segment dword public use32 'DATA'
@std@%numeric_limits$b%@is_signed	segment virtual
@@std@%numeric_limits$b%@is_signed	label	byte
	db	1
@std@%numeric_limits$b%@is_signed	ends
_DATA	ends
_DATA	segment dword public use32 'DATA'
@std@%numeric_limits$c%@is_signed	segment virtual
@@std@%numeric_limits$c%@is_signed	label	byte
	db	1
@std@%numeric_limits$c%@is_signed	ends
_DATA	ends
_DATA	segment dword public use32 'DATA'
s@	label	byte
	;	s@+0:
	db	"_files != NULL",0
	;	s@+15:
	db	"..\routines\LoadImage.cpp",0
	;	s@+41:
	db	"files != NULL",0
	;	s@+55:
	db	"..\routines\LoadImage.cpp",0
	;	s@+81:
	db	"image != NULL",0
	;	s@+95:
	db	"..\routines\LoadImage.h",0
	align	4
_DATA	ends
_TEXT	segment dword public use32 'CODE'
_TEXT	ends
 extrn   __Exception_list:dword
@__assert equ __assert
 extrn   __assert:near
@@FileOpener@FilenameForRead$q60std@%basic_string$c19std@%char_traits$c%17std@%allocator$c%%o equ @FileOpener@FilenameForRead$q60std@%basic_string$c19std@%char_traits$c%17std@%allocator$c%%o
 extrn   @FileOpener@FilenameForRead$q60std@%basic_string$c19std@%char_traits$c%17std@%allocator$c%%o:near
@@std@%basic_string$c19std@%char_traits$c%17std@%allocator$c%%@__nullref equ @std@%basic_string$c19std@%char_traits$c%17std@%allocator$c%%@__nullref
 extrn   @std@%basic_string$c19std@%char_traits$c%17std@%allocator$c%%@__nullref:byte
@@std@%basic_string$c19std@%char_traits$c%17std@%allocator$c%%@__getRep$quiui equ @std@%basic_string$c19std@%char_traits$c%17std@%allocator$c%%@__getRep$quiui
 extrn   @std@%basic_string$c19std@%char_traits$c%17std@%allocator$c%%@__getRep$quiui:near
@_memcpy equ _memcpy
 extrn   _memcpy:near
@CorConvertImage equ CorConvertImage
 extrn   CorConvertImage:near
@CorOpenImage equ CorOpenImage
 extrn   CorOpenImage:near
 extrn   @$bdele$qpv:near
@CorFlipImage equ CorFlipImage
 extrn   CorFlipImage:near
 extrn   @__InitExceptBlockLDTC:near
@CorCreateImageWithPixels equ CorCreateImageWithPixels
 extrn   CorCreateImageWithPixels:near
 ?debug  C FB0A4C6F6164496D61676500
 ?debug  C 9F757569642E6C6962
 ?debug  C 9F757569642E6C6962
	?debug	D "e:\prog\bcc55\include\gl/gl.h" 10459 10272
	?debug	D "e:\prog\bcc55\include\imm.h" 10459 10272
	?debug	D "e:\prog\bcc55\include\mcx.h" 10459 10272
	?debug	D "e:\prog\bcc55\include\winsvc.h" 10459 10272
	?debug	D "e:\prog\bcc55\include\commdlg.h" 10459 10272
	?debug	D "e:\prog\bcc55\include\oleauto.h" 10459 10272
	?debug	D "e:\prog\bcc55\include\propidl.h" 10459 10272
	?debug	D "e:\prog\bcc55\include\oaidl.h" 10459 10272
	?debug	D "e:\prog\bcc55\include\msxml.h" 10459 10272
	?debug	D "e:\prog\bcc55\include\servprov.h" 10459 10272
	?debug	D "e:\prog\bcc55\include\oleidl.h" 10459 10272
	?debug	D "e:\prog\bcc55\include\urlmon.h" 10459 10272
	?debug	D "e:\prog\bcc55\include\cguid.h" 10459 10272
	?debug	D "e:\prog\bcc55\include\objidl.h" 10459 10272
	?debug	D "e:\prog\bcc55\include\unknwn.h" 10459 10272
	?debug	D "e:\prog\bcc55\include\objbase.h" 10459 10272
	?debug	D "e:\prog\bcc55\include\ole2.h" 10459 10272
	?debug	D "e:\prog\bcc55\include\prsht.h" 10459 10272
	?debug	D "e:\prog\bcc55\include\winspool.h" 10459 10272
	?debug	D "e:\prog\bcc55\include\winsmcrd.h" 10459 10272
	?debug	D "e:\prog\bcc55\include\winioctl.h" 10459 10272
	?debug	D "e:\prog\bcc55\include\rpcnsip.h" 10459 10272
	?debug	D "e:\prog\bcc55\include\rpcndr.h" 10459 10272
	?debug	D "e:\prog\bcc55\include\wtypes.h" 10459 10272
	?debug	D "e:\prog\bcc55\include\winscard.h" 10459 10272
	?debug	D "e:\prog\bcc55\include\winefs.h" 10459 10272
	?debug	D "e:\prog\bcc55\include\wincrypt.h" 10459 10272
	?debug	D "e:\prog\bcc55\include\qos.h" 10459 10272
	?debug	D "e:\prog\bcc55\include\winsock2.h" 10459 10272
	?debug	D "e:\prog\bcc55\include\winperf.h" 10459 10272
	?debug	D "e:\prog\bcc55\include\shellapi.h" 10459 10272
	?debug	D "e:\prog\bcc55\include\rpcasync.h" 10459 10272
	?debug	D "e:\prog\bcc55\include\rpcnterr.h" 10459 10272
	?debug	D "e:\prog\bcc55\include\rpcnsi.h" 10459 10272
	?debug	D "e:\prog\bcc55\include\rpcdcep.h" 10459 10272
	?debug	D "e:\prog\bcc55\include\rpcdce.h" 10459 10272
	?debug	D "e:\prog\bcc55\include\rpc.h" 10459 10272
	?debug	D "e:\prog\bcc55\include\nb30.h" 10459 10272
	?debug	D "e:\prog\bcc55\include\mmsystem.h" 10459 10272
	?debug	D "e:\prog\bcc55\include\lzexpand.h" 10459 10272
	?debug	D "e:\prog\bcc55\include\dlgs.h" 10459 10272
	?debug	D "e:\prog\bcc55\include\ddeml.h" 10459 10272
	?debug	D "e:\prog\bcc55\include\dde.h" 10459 10272
	?debug	D "e:\prog\bcc55\include\cderr.h" 10459 10272
	?debug	D "e:\prog\bcc55\include\winnetwk.h" 10459 10272
	?debug	D "e:\prog\bcc55\include\winreg.h" 10459 10272
	?debug	D "e:\prog\bcc55\include\winver.h" 10459 10272
	?debug	D "e:\prog\bcc55\include\wincon.h" 10459 10272
	?debug	D "e:\prog\bcc55\include\winnls.h" 10459 10272
	?debug	D "e:\prog\bcc55\include\tvout.h" 10459 10272
	?debug	D "e:\prog\bcc55\include\winuser.h" 10459 10272
	?debug	D "e:\prog\bcc55\include\pshpack1.h" 10459 10272
	?debug	D "e:\prog\bcc55\include\wingdi.h" 10459 10272
	?debug	D "e:\prog\bcc55\include\winerror.h" 10459 10272
	?debug	D "e:\prog\bcc55\include\winbase.h" 10459 10272
	?debug	D "e:\prog\bcc55\include\pshpack8.h" 10459 10272
	?debug	D "e:\prog\bcc55\include\pshpack2.h" 10459 10272
	?debug	D "e:\prog\bcc55\include\poppack.h" 10459 10272
	?debug	D "e:\prog\bcc55\include\pshpack4.h" 10459 10272
	?debug	D "e:\prog\bcc55\include\guiddef.h" 10459 10272
	?debug	D "e:\prog\bcc55\include\basetsd.h" 10459 10272
	?debug	D "e:\prog\bcc55\include\winnt.h" 10459 10272
	?debug	D "e:\prog\bcc55\include\windef.h" 10459 10272
	?debug	D "e:\prog\bcc55\include\excpt.h" 10459 10272
	?debug	D "e:\prog\bcc55\include\windows.h" 10459 10272
	?debug	D "e:\prog\bcc55\include\assert.h" 10459 10272
	?debug	D "..\vm\HasErrorState.h" 12956 48906
	?debug	D "e:\prog\bcc55\include\fcntl.h" 10459 10272
	?debug	D "e:\prog\bcc55\include\io.h" 10459 10272
	?debug	D "e:\prog\bcc55\include\fstream.h" 10459 10272
	?debug	D "e:\prog\bcc55\include\rw/tree.cc" 10459 10272
	?debug	D "e:\prog\bcc55\include\algorith.cc" 10459 10272
	?debug	D "e:\prog\bcc55\include\iterator.h" 10459 10272
	?debug	D "e:\prog\bcc55\include\algorith.h" 10459 10272
	?debug	D "e:\prog\bcc55\include\rw/tree.h" 10459 10272
	?debug	D "e:\prog\bcc55\include\function.h" 10459 10272
	?debug	D "e:\prog\bcc55\include\map.h" 10459 10272
	?debug	D "e:\prog\bcc55\include\ostream.h" 10459 10272
	?debug	D "e:\prog\bcc55\include\istream.h" 10459 10272
	?debug	D "e:\prog\bcc55\include\rw/usefacet.h" 10459 10272
	?debug	D "e:\prog\bcc55\include\rw/codecvt.h" 10459 10272
	?debug	D "e:\prog\bcc55\include\rw/numbrw.h" 10459 10272
	?debug	D "e:\prog\bcc55\include\rw/rwlocale.cc" 10459 10272
	?debug	D "e:\prog\bcc55\include\float.h" 10459 10272
	?debug	D "e:\prog\bcc55\include\cfloat.h" 10459 10272
	?debug	D "e:\prog\bcc55\include\math.h" 10459 10272
	?debug	D "e:\prog\bcc55\include\rw/math.h" 10459 10272
	?debug	D "e:\prog\bcc55\include\limits.stl" 10459 10272
	?debug	D "e:\prog\bcc55\include\limits.h" 10459 10272
	?debug	D "e:\prog\bcc55\include\rw/iosbase.h" 10459 10272
	?debug	D "e:\prog\bcc55\include\rw/numeral.h" 10459 10272
	?debug	D "e:\prog\bcc55\include\rw/ctype.h" 10459 10272
	?debug	D "e:\prog\bcc55\include\rw/vendor.h" 10459 10272
	?debug	D "e:\prog\bcc55\include\rw/locvecto.h" 10459 10272
	?debug	D "e:\prog\bcc55\include\rw/locimpl.h" 10459 10272
	?debug	D "e:\prog\bcc55\include\typeinfo.h" 10459 10272
	?debug	D "e:\prog\bcc55\include\time.h" 10459 10272
	?debug	D "e:\prog\bcc55\include\ctime.h" 10459 10272
	?debug	D "e:\prog\bcc55\include\rw/rwlocale.h" 10459 10272
	?debug	D "e:\prog\bcc55\include\ios.h" 10459 10272
	?debug	D "e:\prog\bcc55\include\streambu.h" 10459 10272
	?debug	D "e:\prog\bcc55\include\sstream.h" 10459 10272
	?debug	D "e:\prog\bcc55\include\stdexcep.h" 10459 10272
	?debug	D "e:\prog\bcc55\include\iosfwd.h" 10459 10272
	?debug	D "e:\prog\bcc55\include\rw/iotraits.h" 10459 10272
	?debug	D "e:\prog\bcc55\include\_nfile.h" 10459 10272
	?debug	D "e:\prog\bcc55\include\stdio.h" 10459 10272
	?debug	D "e:\prog\bcc55\include\cstdio.h" 10459 10272
	?debug	D "e:\prog\bcc55\include\rw/traits.h" 10459 10272
	?debug	D "e:\prog\bcc55\include\rw/stdmutex.h" 10459 10272
	?debug	D "e:\prog\bcc55\include\utility.h" 10459 10272
	?debug	D "e:\prog\bcc55\include\rw/iterator.cc" 10459 10272
	?debug	D "e:\prog\bcc55\include\rw/iterator.h" 10459 10272
	?debug	D "e:\prog\bcc55\include\alloc.h" 10459 10272
	?debug	D "e:\prog\bcc55\include\except.h" 10459 10272
	?debug	D "e:\prog\bcc55\include\exceptio.h" 10459 10272
	?debug	D "e:\prog\bcc55\include\new.h" 10459 10272
	?debug	D "e:\prog\bcc55\include\_lim.h" 10459 10272
	?debug	D "e:\prog\bcc55\include\climits.h" 10459 10272
	?debug	D "e:\prog\bcc55\include\search.h" 10459 10272
	?debug	D "e:\prog\bcc55\include\stdlib.h" 10459 10272
	?debug	D "e:\prog\bcc55\include\cstdlib.h" 10459 10272
	?debug	D "e:\prog\bcc55\include\memory.stl" 10459 10272
	?debug	D "e:\prog\bcc55\include\memory.h" 10459 10272
	?debug	D "e:\prog\bcc55\include\rw/string_r.h" 10459 10272
	?debug	D "e:\prog\bcc55\include\wctype.h" 10459 10272
	?debug	D "e:\prog\bcc55\include\cwctype.h" 10459 10272
	?debug	D "e:\prog\bcc55\include\tchar.h" 10459 10272
	?debug	D "e:\prog\bcc55\include\wchar.h" 10459 10272
	?debug	D "e:\prog\bcc55\include\cwchar.h" 10459 10272
	?debug	D "e:\prog\bcc55\include\mbctype.h" 10459 10272
	?debug	D "e:\prog\bcc55\include\ctype.h" 10459 10272
	?debug	D "e:\prog\bcc55\include\cctype.h" 10459 10272
	?debug	D "e:\prog\bcc55\include\cstring.h" 10459 10272
	?debug	D "e:\prog\bcc55\include\stdarg.h" 10459 10272
	?debug	D "e:\prog\bcc55\include\rw/stdwind.h" 10459 10272
	?debug	D "e:\prog\bcc55\include\cstddef.h" 10459 10272
	?debug	D "e:\prog\bcc55\include\rw/stddefs.h" 10459 10272
	?debug	D "e:\prog\bcc55\include\rw/rwstderr.h" 10459 10272
	?debug	D "e:\prog\bcc55\include\compnent.h" 10459 10272
	?debug	D "e:\prog\bcc55\include\stdcomp.h" 10459 10272
	?debug	D "e:\prog\bcc55\include\string.stl" 10459 10272
	?debug	D "e:\prog\bcc55\include\mem.h" 10459 10272
	?debug	D "e:\prog\bcc55\include\_loc.h" 10459 10272
	?debug	D "e:\prog\bcc55\include\locale.h" 10459 10272
	?debug	D "e:\prog\bcc55\include\_str.h" 10459 10272
	?debug	D "e:\prog\bcc55\include\string.h" 10459 10272
	?debug	D "..\vm\misc.h" 12956 48906
	?debug	D "..\routines\EmbeddedFiles.h" 12996 40305
	?debug	D "e:\prog\bcc55\include\_null.h" 10459 10272
	?debug	D "e:\prog\bcc55\include\_defs.h" 10459 10272
	?debug	D "e:\prog\bcc55\include\_stddef.h" 10459 10272
	?debug	D "e:\prog\bcc55\include\stddef.h" 10459 10272
	?debug	D "..\coronalibrary\Corona.h" 12956 48893
	?debug	D "..\routines\LoadImage.h" 12956 48901
	?debug	D "..\routines\LoadImage.cpp" 13097 37028
	end
