use Fatal qw(open close);
use strict;

sub show {
  my $str = shift;
  my @bytes = split //, $str;
  for my $byte (@bytes) {
    my $pic = unpack('B8', $byte);
    $pic =~ s/0/./g;
    print "$pic\n";
  }
}
#show("\x01\x02\x03\x04");

my @chars;

sub get {
  #my $fh = shift;
  my $height = shift;
  my $out = '';

  my $leader = <> or return 0;
  chomp $leader;
  die "wrong format of leader line `$leader'" 
    unless $leader =~ /^=+\s+'.+'\s+(\d+)\s+(0x([\da-fA-F]+)|(0))\s*$/;
  my $char = hex($+);			# Ignore decimal

  for my $i (1..$height) {
    my $in = <> or die "uncomplete glyph while reading '$char'";
    chomp $in;
    die "wrong width of l=$i of `$in' for ch=$char, expecting 8"
      unless length $in == 8;
    $in =~ s/\./0/g;
    $out .= pack 'B8', $in;
  }
  $chars[$char] = $out;
  return 1;
}

my $write;
$write = shift if @ARGV && $ARGV[0] eq '-w';

die <<EOU if @ARGV != 2 && @ARGV != 3;
Usage: $0 [-w] height font-file-name [char] [input-file]
  Char may be a string Perl can interpolate to a char, such as \\x8F
  If -w, reads glyphs from input-file or STDIN, writes the font-file-name,
    the ordinals of the glyphs are taken from the hex value on the leader line;
    anything but the hex value on the leader line is ignored.
  Width of chars is hardwired to be 8 pels.
EOU

my $picsize = shift;
my $file = shift;
my $contents;
my $char;
if (@ARGV && ! $write) {
  my $char = shift;
  $char = eval "qq{$char}" if length $char > 1;
  die "Need a char" unless length $char == 1;
}

$file = "> $file" if $write;
open F, $file;
binmode F;

unless ($write) {
  local $/ = undef; 
  $contents = <F>;
  close F;
}

if (defined $char) {
  show substr $contents, $picsize*ord($char), $picsize;
} elsif ($write) {
  1 while get($picsize);
  my @notdef = grep {not defined $chars[$_]} 0..0xff;
  die "Undefined glyphs @notdef" if @notdef;
  print F join '', @chars[0..0xff];
  close F;
} else {
  for $char (0..0xff) {
    $char = chr $char;
    my $strchr = $char;
    $strchr = '\r' if $char eq "\r";
    $strchr = '\n' if $char eq "\n";
    $strchr = '\t' if $char eq "\t";
    $strchr = '^Z' if $char eq "\cZ";
    printf "===== '%s' %d %#x\n", $strchr, ord $char, ord $char;
    show substr $contents, $picsize*ord($char), $picsize;
  }
}
