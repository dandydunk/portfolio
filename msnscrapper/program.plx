use warnings;
use strict;

require msnscrapper;
require HTTPUtils;

sub a {
	print shift(@_);
}

my $msn = msnscrapper->new();

my $a = HTTPUtils->new;
$msn->{data} = $a->GetHttpData("https://www.bing.com/search?q=get+money&form=PRUSEN&mkt=en-us&httpsmsn=1&refig=995e76cb78f64e1eaa48a07cff9ee02c&sp=-1&pq=get+money&sc=8-9&qs=n&sk=&cvid=995e76cb78f64e1eaa48a07cff9ee02c");

$msn->Parse();