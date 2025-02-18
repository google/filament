while (<>) {
	if (/version : '(.+)',/) {
		print "$1";
	}
}
