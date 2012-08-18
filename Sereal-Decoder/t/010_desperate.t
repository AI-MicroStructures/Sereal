#!perl
use strict;
use warnings;
use Sereal::Decoder qw(decode_sereal);
use Sereal::Constants qw(:all);

# These tests are extraordinarily basic, badly-done and really just
# for basic sanity testing during development.

use Test::More;

sub varint {
  my $n = shift;
  my $out = '';
  while ($n > 0x80) {
    $out .= chr( ($n & 0x7f) | 0x80 );
      $n >>= 7;
    }
    $out .= chr($n);
  return $out;
}

my $hdr = SRL_MAGIC_STRING . chr(SRL_PROTOCOL_VERSION) . chr(0);

my @basic_tests = (
  # warning: this hardcodes the POS/NEG headers
  [1, chr(0b0000_0001), "decode 1"],
  [0, chr(0b0000_0000), "decode 0"],
  [-1, chr(0b0001_0000), "decode -1"],
  [undef, chr(SRL_HDR_UNDEF), "decode undef"],
  ["", chr(0b0100_0000), "decode empty string"],
  ["1", chr(0b0100_0001) . "1", "decode string '1'"],
  ["91a", chr(0b0100_0011) . "91a", "decode string '91a'"],
  [\1, chr(SRL_HDR_REF).chr(0b0000_0001), "scalar ref to int"],
  [[], chr(SRL_HDR_ARRAY).varint(0), "empty array ref"],
  [[1,2,3], chr(SRL_HDR_ARRAY).varint(3).chr(0b0000_0001).chr(0b0000_0010).chr(0b0000_0011), "array ref"],
  [1000, chr(SRL_HDR_VARINT).varint(1000), "large int"],
  [ [1..1000],
    chr(SRL_HDR_ARRAY).varint(1000).join("", map chr, (1..SRL_POS_MAX_SIZE))
                                   .join("", map chr(SRL_HDR_VARINT).varint($_), ((SRL_POS_MAX_SIZE+1) .. 1000)),
    "array ref with big ints"
  ],
  [{}, chr(SRL_HDR_HASH).varint(0), "empty hash ref"],
  [{foo => "baaaaar"}, chr(SRL_HDR_HASH).varint(1).chr(0b0100_0011)."foo".chr(0b0100_0111)."baaaaar", "simple hash ref"],
);

foreach my $bt (@basic_tests) {
  my ($in, $exp, $name) = @$bt;
  $exp = "$hdr$exp";
  my $out = decode_sereal($in);
  ok(defined $out, "defined: $name");
  #is(length($out), length($exp));
  is($out, $exp, "correct: $name");
}

done_testing();

