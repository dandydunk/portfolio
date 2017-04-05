package msnscrapper;

use warnings;
use strict;

sub new {
	my $class = shift;
	my $self = {data => ""};
	bless $self, $class;
	return $self;
}

sub Parse {
	my $self = shift;
	if(0 == length($self->{data})) {
		return;
	}

	my $index = 0;
	while(1) {
		my $needle = '<li class="b_algo"';
		my $tmpIndex = index($self->{data}, $needle, $index);
		if($tmpIndex == -1) {
			print "needle [$needle] not found\r\n";
			last;
		}
		
		$index = $tmpIndex + length($needle);
		$needle = "<a href=\"";
		$tmpIndex = index($self->{data}, $needle, $index);
		if($tmpIndex == -1) {
			print "needle [$needle] not found";
			last;
		}
		
		$index = $tmpIndex + length($needle);
		my $beginExtractIndex = $index;
		
		$needle = "\"";
		$tmpIndex = index($self->{data}, $needle, $index);
		if($tmpIndex == -1) {
			print "needle [$needle] not found";
			last;
		}
		
		$index = $tmpIndex;
		my $endExtractIndex = $index;
		
		my $href = substr($self->{data}, $beginExtractIndex, $endExtractIndex - $beginExtractIndex);
		print "$href\r\n";
	}
}

1;