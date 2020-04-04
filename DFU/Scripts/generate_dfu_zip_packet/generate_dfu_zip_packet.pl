#!/usr/bin/perl
use strict;
use warnings;

use Config::Tiny;
use Data::Dumper qw(Dumper);

# function usage() show help
sub usage {
	print "TODO";
}

# print begin date & time
sub show_date_time {
	my ($comment) = @_;
	my ($sec, $min, $hour, $mday, $mon, $year, $wday, $yday, $isdst) = localtime(time);

	$year += 1900;
	$mon++;
	if ((defined $comment) && ($comment ne "")) {
		printf "%02u/%02u/%04u  %02u:%02u:%02u  $comment\n", $mday, $mon, $year, $hour, $min, $sec;
	}
	else {
		printf "%02u/%02u/%04u  %02u:%02u:%02u\n", $mday, $mon, $year, $hour, $min, $sec;
	}
}

# print begin time
sub show_time {
	my ($comment) = @_;
	my ($sec, $min, $hour, $mday, $mon, $year, $wday, $yday, $isdst) = localtime(time);

	$year += 1900;
	$mon++;
	if ((defined $comment) && ($comment ne "")) {
		printf "%02u:%02u:%02u  $comment\n", $hour, $min, $sec;
	}
	else {
		printf "%02u:%02u:%02u\n", $hour, $min, $sec;
	}
}

my $filename = shift or die "Usage: $0 FILENAME\n";
my $config = Config::Tiny->read( $filename, 'utf8' );

show_date_time("Script start");
`nrfutil pkg generate --application $config->{general}{hex_file} --application-version $config->{dfu}{app_version} --application-version-string "1.0.0" --hw-version 52 --sd-req $config->{dfu}{sdk_version} --key-file $config->{general}{private_key_directory} app_dfu_package.zip`;
if ($? == -1) {
	die "nrfutil pkg generate --application $config->{general}{hex_file} --application-version $config->{dfu}{app_version} --application-version-string \"1.0.0\" --hw-version 52 --sd-req $config->{dfu}{sdk_version} --key-file $config->{general}{private_key_directory} app_dfu_package.zip 2>&1 failed: $!\n";
}
elsif (($? >> 8) != 0) {
	print "nrfutil pkg generate --application $config->{general}{hex_file} --application-version $config->{dfu}{app_version} --application-version-string \"1.0.0\" --hw-version 52 --sd-req $config->{dfu}{sdk_version} --key-file $config->{general}{private_key_directory} app_dfu_package.zip return error: ".($? >> 8)."\n";
}
