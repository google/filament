# This script is obsolete. It would need to change to generate TOC.html
# in the new format and link to .html instead of .xml files.

#!/usr/bin/perl

sub Usage {
print
"Usage: maketoc xhtmldir xmldir
   where xhtmldir contains a directory full of OpenGL .xml XHTML man pages -AND-
   where xmldir contains a directory full of OpenGL .xml source XML man pages

   probably want to redirect output into a file like
   ./maketoc.pl . .. > ./toc.html
"
}

sub PrintHeader {
print '<html>
<head>
<title>EGL Reference Pages</title>
<style type="text/css">

html, body, table
{   color: #000;
	padding: 4px 4px;
	margin: 0px 0 0 0;
	text-align: center;
	font-family: Arial, Lucida, sans-serif;
	font-size: 10pt;

}

#container {
	margin: 10px;
	font-size: 14pt;
	text-decoration:none;
}

table.sample {
	border-width: 1px;
	border-spacing: 5px;
	border-style: dotted;
	border-color: black;
	border-collapse: separate;
	background-color: #F0F0F0;
}
table.sample th {
	border-width: 1px;
	padding: 5px;
	border-style: none;
}
table.sample td {
	border-width: 1px;
	padding: 1px;
	border-style: none;
}
</style>

</head>
<body>
<a name="top"></a>
<h1>EGL Reference Pages</h1>
<br/><br/>

';
}

sub PrintFooter {
print '
</body>
</html>
';
}

sub TableElementForFilename {
	my $name = shift;

	my $strippedname = $name;
	$strippedname =~ s/\.xml//;
	print "\t";
	print '<tr><td><a target="pagedisp" href="' , $name , '">';
	print "$strippedname";
	print "</a></td></tr>\n";
}

sub BeginTable {
	my $letter = shift;
	print "<a name=\"$letter\"></a><br/><br/>\n";
	print '<table width="220" align="center" class="sample">';
	print "\t<th>";
	print "$letter</th>\n";
}

sub EndTable {
	print "\t";
	print '<tr><td><center><a href="#top">Top</a></center></td></tr>';
	print "\n</table>\n\n";
}



##############
#  main
##############

if (@ARGV != 2)
{
	Usage();
	die;
}

# grab list of generated XHTML files
opendir(DIR,$ARGV[0]) or die "couldn't open directory";

@files = readdir(DIR);
close(DIR);
@files = sort @files;

PrintHeader();

my @glX;
my @glut;
my @glu;
my @egl;
my @gl;

my @realEntrypoints;
my @pageNames;

#pre-create list of all true entrypoint names

foreach (@files)
{
	if (/xml/)
	{
		$parentName = $ARGV[1] . '/' . $_;
		if (open(PARENT, $parentName))
		{
			@funcs = <PARENT>;
			@funcs = grep(/<funcdef>/, @funcs);
			foreach (@funcs)
			{
				$func = $_;
				$func =~ s/.*<function>//;
				$func =~ s/<\/function>.*\n//;

				push (@realEntrypoints, $func);
			}
			close(PARENT);
		}
	}
}

#pre-create list of page names

foreach (@files)
{
	if (/xml/)
	{
		$parentName = $ARGV[1] . '/' . $_;
		if (open(PARENT, $parentName))
		{
			my $entrypoint = $_;
			$entrypoint =~ s/\.xml//;

			push (@pageNames, $entrypoint);

			close(PARENT);
		}
	}
}

#sort the files into gl, glut, glu, EGL, and glX

foreach (@files)
{
	if (/xml/)
	{
		# filter out entrypoint variations that don't have their own man pages
		my $needIndexEntry = 0;

		# continue only if parent page exists (e.g. glColor) OR
		# different parent page exists with matching entrypoint (e.g. glEnd)
		my $entrypoint = $_;
		$entrypoint =~ s/\.xml//;

		foreach (@pageNames)
		{
			if ($_ eq $entrypoint)
			{
				# it has its own man page
				$needIndexEntry = 1;
			}
		}

		if ($needIndexEntry == 0)
		{
			foreach (@realEntrypoints)
			{
				if ($_ eq $entrypoint)
				{
					# it's a real entrypoint, but make sure not a variation
					$needIndexEntry = 1;

					foreach (@pageNames)
					{
						my $alteredEntrypoint = $entrypoint;
						$alteredEntrypoint =~ s/$_//;

						if (!($alteredEntrypoint eq $entrypoint))
						{
							$needIndexEntry = 0;
						}
					}
				}
			}
		}

		if ($needIndexEntry)
		{
			if (/^glX/)
			{
				push (@glX, $_);
			}
			elsif (/^glut/)
			{
				push (@glut, $_);
			}
			elsif (/^glu/)
			{
				push (@glu, $_);
			}
			elsif (/^egl/)
			{
				push (@egl, $_);
			}
			elsif (/^gl/)
			{
				push (@gl, $_);
			}
		}
	}
}


#output the table of contents

my @toc;

if ($#gl > 0)
{
	$currentletter = "";
	$opentable = 0;

	foreach (@gl)
	{
		$name = $_;
		$name =~ s/^gl//;
		$firstletter = substr($name, 0, 1);
		if ($firstletter ne $currentletter)
		{
			push (@toc, $firstletter);
			$currentletter = $firstletter;
		}
	}
	if ($#egl > 0) { push (@toc, "egl"); }
	if ($#glu > 0) { push (@toc, "glu"); }
	if ($#glut > 0) { push (@toc, "glut"); }
	if ($#glX > 0) { push (@toc, "glX"); }
}


print '<div id="container">';
foreach (@toc)
{
	print '<b><a href="#';
	print $_;
	print '" style="text-decoration:none"> ';
	print $_;
	print " </a></b> &nbsp; ";
}
print "</div>\n\n\n";

# output the tables

if ($#gl > 0)
{
	$currentletter = "";
	$opentable = 0;

	foreach (@gl)
	{
		$name = $_;
		$name =~ s/^gl//;
		$firstletter = substr($name, 0, 1);
		if ($firstletter ne $currentletter)
		{
			if ($opentable == 1)
			{
				EndTable();
			}
			BeginTable($firstletter);
			$opentable =1;
			$currentletter = $firstletter;
		}
		TableElementForFilename($_);
	}
	if ($opentable)
	{
		EndTable();
	}
}

if ($#egl > 0)
{
	BeginTable("egl");
	foreach (@egl)
	{
		TableElementForFilename($_);
	}
	EndTable();
}

if ($#glu > 0)
{
	BeginTable("glu");
	foreach (@glu)
	{
		TableElementForFilename($_);
	}
	EndTable();
}

if ($#glut > 0)
{
	BeginTable("glut");
	foreach (@glut)
	{
		TableElementForFilename($_);
	}
	EndTable();
}

if ($#glX > 0)
{
	BeginTable("glX");
	foreach (@glX)
	{
		TableElementForFilename($_);
	}
	EndTable();
}

PrintFooter();
