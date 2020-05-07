#!/usr/bin/perl
use strict;
use utf8;
use Time::Local 'timelocal';
use Encode;
use IO::File;
use Fcntl qw(:flock);
use LWP::UserAgent;

my $caida_url = 'http://data.caida.org/datasets/routing/routeviews-prefix2as';
my $dir = '/usr/local/etc/asis';
my $config_file = $dir . "/routerview.pfx2as";

&main;
exit;

sub main {

	my $now = time();
	my ($sec, $min, $hour, $mday, $mon, $year) = localtime(($now - 86400 * 3));
	$mon += 1;
	$year += 1900;

	my $url = sprintf("%s/%04d/%02d/routeviews-rv2-%04d%02d%02d-1200.pfx2as.gz",
										$caida_url, $year, $mon, $year, $mon, $mday);
	my $save_file = sprintf("%s/routeviews-rv2-%04d%02d%02d-1200.pfx2as.gz",
										$dir, $year, $mon, $mday);

	my $ua = LWP::UserAgent->new;
	$ua->timeout(30);
	$ua->agent('Mozilla');
	my $res = $ua->mirror($url, $save_file);

	printf("%s\n%s\n", $url, $save_file);

  if ($res->is_success) {
		save_config($save_file);	
  }
}

sub save_config {
	my ($save_file) = @_;
	my $fh = IO::File->new( "zcat $save_file 2>/dev/null |" )
		or die "Can't zcat '$save_file' for reading: $!";

	my $io = IO::File->new($config_file, 'w');
	flock($io, LOCK_EX);

	while ( defined(my $line = $fh->getline()) ) {
		print $io $line;
	}
	flock($io, LOCK_UN) or die;
	$io->close;
	$fh->close or die "Can't close '$save_file' after reading: $!";
}




