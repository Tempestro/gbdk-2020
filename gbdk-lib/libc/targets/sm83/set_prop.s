	.include	"global.s"

	.area	_HOME

	;; Set properties of sprite number C to D
.set_sprite_prop::
	LD	HL,#_shadow_OAM+3	; Calculate origin of sprite info

	SLA	C		; Multiply C by 4
	SLA	C
	LD	B,#0x00
	ADD	HL,BC

	LD	(HL),D	; Set sprite properties
	RET
