package HTTPUtils;

use LWP::UserAgent;

sub new {
	$class = shift(@_);
	$self = {"name"=>"marcus"};
	bless $self, $class;
	return $self;
}

sub GetHttpData {
	my $self = shift(@_);
	my $url = shift(@_);
	print "getting data from [$url]";
	
	my $ua = LWP::UserAgent->new;
	$ua->agent("MyApp/0.1 ");
	
	my $req = HTTP::Request->new(GET => $url);
	my $res = $ua->request($req);

	if ($res->is_success) {
		return $res->content;
	}
	else {
		return "";
	}
}
1;