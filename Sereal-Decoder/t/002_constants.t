#!perl
use strict;
use warnings;
use Sereal::Decoder qw(decode_sereal);
use Sereal::Constants qw(:all);

# Test a couple of the basic constants

use Test::More tests => 2;

is(SRL_MAGIC_STRING, "srl", "check magic string");
is(SRL_HDR_ASCII, 0b0100_0000, "check arbitrary header constant");

