#!/bin/sh
set -eu

mkdir -p build/assets

# Linux equivalent of macOS `sips`: use ImageMagick `magick` + resize + save as BMP
# We keep behavior simple: force output sizes.
magick filemanager.png -resize 48x48! -define bmp:format=bmp -depth 24 build/assets/filemanager_48.bmp
magick internetexplorer.png -resize 48x48! -define bmp:format=bmp -depth 24 build/assets/internetexplorer_48.bmp
magick activitymanager.png -resize 48x48! -define bmp:format=bmp -depth 24 build/assets/activitymanager_48.bmp
magick paint.png -resize 48x48! -define bmp:format=bmp -depth 24 build/assets/paint_48.bmp
magick systemsettings.png -resize 48x48! -define bmp:format=bmp -depth 24 build/assets/systemsettings_48.bmp
magick systemupdates.png -resize 48x48! -define bmp:format=bmp -depth 24 build/assets/systemupdates_48.bmp
magick calculator.png -resize 48x48! -define bmp:format=bmp -depth 24 build/assets/calculator_48.bmp
magick txteditor.png -resize 48x48! -define bmp:format=bmp -depth 24 build/assets/txteditor_48.bmp
magick bootupscreenLog.png -resize 640x312! -define bmp:format=bmp -depth 24 build/assets/bootupscreenlog_640x312.bmp
magick bootupscreen.png -resize 640x312! -define bmp:format=bmp -depth 24 build/assets/bootupscreen_640x312.bmp
magick cursor.png -resize 20x20! -define bmp:format=bmp -depth 24 build/assets/cursor_20.bmp
magick cursorselected.png -resize 20x20! -define bmp:format=bmp -depth 24 build/assets/cursorselected_20.bmp
magick redsnake.png -resize 48x48! -define bmp:format=bmp -depth 24 build/assets/redsnake_48.bmp
magick cstore.png -resize 48x48! -define bmp:format=bmp -depth 24 build/assets/cstore_48.bmp



for bmp in build/assets/*.bmp; do
  raw="${bmp%.bmp}.raw"
  perl -e '
    use strict;
    use warnings;
    my ($in, $out) = @ARGV;
    open my $fh, q{<:raw}, $in or die $!;
    local $/;
    my $data = <$fh>;
    close $fh;

    my $offset = unpack("V", substr($data, 10, 4));
    my $width = unpack("l<", substr($data, 18, 4));
    my $height = unpack("l<", substr($data, 22, 4));
    my $top_down = $height < 0 ? 1 : 0;
    $height = abs($height);
    my $stride = (($width * 3 + 3) & ~3);

    open my $out_fh, q{>:raw}, $out or die $!;
    for my $y (0 .. $height - 1) {
      my $src_y = $top_down ? $y : ($height - 1 - $y);
      my $row_off = $offset + $src_y * $stride;
      for my $x (0 .. $width - 1) {
        my $pix_off = $row_off + $x * 3;
        my ($b, $g, $r) = unpack("C3", substr($data, $pix_off, 3));
        my $value;
        if ($r >= 248 && $g >= 248 && $b >= 248) {
          $value = 255;
        } else {
          my $rq = $r < 43 ? 0 : $r < 94 ? 1 : $r < 145 ? 2 : $r < 196 ? 3 : $r < 247 ? 4 : 5;
          my $gq = $g < 43 ? 0 : $g < 94 ? 1 : $g < 145 ? 2 : $g < 196 ? 3 : $g < 247 ? 4 : 5;
          my $bq = $b < 43 ? 0 : $b < 94 ? 1 : $b < 145 ? 2 : $b < 196 ? 3 : $b < 247 ? 4 : 5;
          $value = $rq * 36 + $gq * 6 + $bq;
        }
        print {$out_fh} pack("C", $value);
      }
    }
    close $out_fh;
  ' "$bmp" "$raw"
done
