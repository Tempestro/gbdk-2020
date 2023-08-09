        .include    "global.s"

        .title  "Metasprites"
        .module Metasprites

        .globl  ___render_shadow_OAM

        .area   _CODE

; void hide_sprites_range(UINT8 from, UINT8 to)

_hide_sprites_range::
        cp      #40
        ret     nc

        ld      d, a

        add     a
        add     a
        ld      c, a

        ld      a, e
        sub     d

        ret     c
        ret     z

        ld      hl, #___render_shadow_OAM
        ld      h, (hl)
        ld      l, c

        ld      de, #4

        srl     a
        jr      nc, 0$

        ld      (hl), d
        add     hl, de

        ret     z               ; z is not affected by 16-bit add

0$:
        srl     a
        jr      nc, 1$

        ld      (hl), d
        add     hl, de

        ld      (hl), d
        add     hl, de

        ret     z               ; z is not affected by 16-bit add

1$:
        ld      (hl), d
        add     hl, de

        ld      (hl), d
        add     hl, de

        ld      (hl), d
        add     hl, de

        ld      (hl), d
        add     hl, de

        dec     a
        jr      nz, 1$

        ret
