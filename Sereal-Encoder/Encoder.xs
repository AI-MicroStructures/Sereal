#define PERL_NO_GET_CONTEXT
#include "EXTERN.h"
#include "perl.h"
#include "XSUB.h"

#include "ppport.h"

#include "srl_encoder.h"

/* Generated code for exposing C constants to Perl */
#include "srl_protocol.h"
#include "const-c.inc"


MODULE = Sereal::Encoder        PACKAGE = Sereal::Encoder
PROTOTYPES: DISABLE

void
encode_sereal(src, opt = newHV())
    SV *src;
    HV *opt;
  PREINIT:
    srl_encoder_t *enc;
  PPCODE:
    enc = build_encoder_struct(aTHX_ opt);
    srl_write_header(aTHX_ enc);
    srl_dump_sv(aTHX_ enc, src);
    /* FIXME optimization: avoid copy by stealing string buffer if
     *                     it is not too large. */
    ST(0) = sv_2mortal(newSVpvn(enc->buf_start, (STRLEN)(enc->pos - enc->buf_start)));
    XSRETURN(1);


MODULE = Sereal::Encoder        PACKAGE = Sereal::Constants
PROTOTYPES: DISABLE

INCLUDE: const-xs.inc

