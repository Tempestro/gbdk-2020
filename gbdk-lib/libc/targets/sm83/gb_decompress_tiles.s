; GB-Decompress tiledata directly to VRAM
; Compatible with GBTD

        .include        "global.s"

        .title  "GB Decompress"
        .module GBDecompress

.macro WRAP_VRAM regH, ?loc
        bit     3, regH
        jr      z, loc
        res     4, regH
loc:
.endm

.macro UNWRAP_VRAM regH, ?loc
        bit     3, regH
        jr      nz, loc
        set     4, regH
loc:
.endm


        .area _CODE

_gb_decompress_bkg_data::
_gb_decompress_win_data::
        ld      b, #0x90
        ld      hl, #.LCDC
        bit     LCDCF_B_BG8000, (hl)
        jr      nz, .load_params
_gb_decompress_sprite_data::
        ld      b, #0x80

.load_params:
        ld      h, d
        ld      l, e

        ; Compute dest ptr
        swap    a       ; *16 (size of a tile)
        ld      e, a
        and     #0x0F   ; Get high bits
        add     b       ; Add base offset of target tile "block"
        ld      d, a
        ld      a, e
        and     #0xF0   ; Get low bits only
        ld      e, a
        WRAP_VRAM d

; hl = source; de = dest
gb_decompress_vram::
        push    de
1$:
        ld      a,(hl+) ; load command
        or      a
        jp      z,9$    ; exit, if last byte
        bit     7,a
        jr      nz,5$   ; string functions
        bit     6,a
        jr      nz,3$
        ; RLE byte
        and     #63     ; calc counter
        inc     a
        ld      c,a
        ld      a,(hl+)
        ld      b,a
2$:
        WAIT_STAT
        ld      a,b
        ld      (de),a
        inc     de
        WRAP_VRAM d
        dec     c
        jr      nz,2$
        jr      1$      ; next command

3$:                     ; RLE word
        and     #63
        inc     a
        ld      c, a
        ld      a,(hl+)
        ld      b, a
4$:
        WAIT_STAT
        ld      a,b     ; store word
        ld      (de),a
        inc     de
        WRAP_VRAM d
        WAIT_STAT
        ld      a,(hl)
        ld      (de),a
        inc     de
        WRAP_VRAM d
        dec     c
        jr      nz,4$
        inc     hl
        jr      1$      ; next command

5$:
        bit     6,a
        jr      nz,7$

6$:                     ; string repeat
        and     a,#63
        inc     a
        push    hl

        ld      c,(hl)
        inc     hl
        ld      b,(hl)

        ldhl    sp,#3
        bit     4,(hl)  ; check start address was above 0x9000
        jr      z, 11$

        ld      h,d
        ld      l,e
        add     hl,bc
        UNWRAP_VRAM h
        jr      12$
11$:
        ld      h,d
        ld      l,e
        add     hl,bc
12$:
        ld      c,a

14$:
        WAIT_STAT
        ld      a,(hl+)
        ld      (de),a
        WRAP_VRAM h
        inc     de
        WRAP_VRAM d
        dec     c
        jr      nz, 14$

        pop     hl
        inc     hl
        inc     hl
        jp      1$      ; next command

7$:                     ; string copy
        and     #63
        inc     a
        ld      c,a
15$:
        WAIT_STAT
        ld      a,(hl+)
        ld      (de),a
        inc     de
        WRAP_VRAM d
        dec     c
        jr      nz, 15$

        jp      1$      ; next command
9$:
        pop     de
        ret
