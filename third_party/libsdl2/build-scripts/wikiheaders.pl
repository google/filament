#!/usr/bin/perl -w

use warnings;
use strict;
use Text::Wrap;

my $srcpath = undef;
my $wikipath = undef;
my $warn_about_missing = 0;
my $copy_direction = 0;

foreach (@ARGV) {
    $warn_about_missing = 1, next if $_ eq '--warn-about-missing';
    $copy_direction = 1, next if $_ eq '--copy-to-headers';
    $copy_direction = 1, next if $_ eq '--copy-to-header';
    $copy_direction = -1, next if $_ eq '--copy-to-wiki';
    $srcpath = $_, next if not defined $srcpath;
    $wikipath = $_, next if not defined $wikipath;
}

my $wordwrap_mode = 'mediawiki';
sub wordwrap_atom {   # don't call this directly.
    my $str = shift;
    return fill('', '', $str);
}

sub wordwrap_with_bullet_indent {  # don't call this directly.
    my $bullet = shift;
    my $str = shift;
    my $retval = '';

    #print("WORDWRAP BULLET ('$bullet'):\n\n$str\n\n");

    # You _can't_ (at least with Pandoc) have a bullet item with a newline in
    #  MediaWiki, so _remove_ wrapping!
    if ($wordwrap_mode eq 'mediawiki') {
        $retval = "$bullet$str";
        $retval =~ s/\n/ /gms;
        $retval =~ s/\s+$//gms;
        #print("WORDWRAP BULLET DONE:\n\n$retval\n\n");
        return "$retval\n";
    }

    my $bulletlen = length($bullet);

    # wrap it and then indent each line to be under the bullet.
    $Text::Wrap::columns -= $bulletlen;
    my @wrappedlines = split /\n/, wordwrap_atom($str);
    $Text::Wrap::columns += $bulletlen;

    my $prefix = $bullet;
    my $usual_prefix = ' ' x $bulletlen;

    foreach (@wrappedlines) {
        $retval .= "$prefix$_\n";
        $prefix = $usual_prefix;
    }

    return $retval;
}

sub wordwrap_one_paragraph {  # don't call this directly.
    my $retval = '';
    my $p = shift;
    #print "\n\n\nPARAGRAPH: [$p]\n\n\n";
    if ($p =~ s/\A([\*\-] )//) {  # bullet list, starts with "* " or "- ".
        my $bullet = $1;
        my $item = '';
        my @items = split /\n/, $p;
        foreach (@items) {
            if (s/\A([\*\-] )//) {
                $retval .= wordwrap_with_bullet_indent($bullet, $item);
                $item = '';
            }
            s/\A\s*//;
            $item .= "$_\n";   # accumulate lines until we hit the end or another bullet.
        }
        if ($item ne '') {
            $retval .= wordwrap_with_bullet_indent($bullet, $item);
        }
    } else {
        $retval = wordwrap_atom($p) . "\n";
    }

    return $retval;
}

sub wordwrap_paragraphs {  # don't call this directly.
    my $str = shift;
    my $retval = '';
    my @paragraphs = split /\n\n/, $str;
    foreach (@paragraphs) {
        next if $_ eq '';
        $retval .= wordwrap_one_paragraph($_);
        $retval .= "\n";
    }
    return $retval;
}

my $wordwrap_default_columns = 76;
sub wordwrap {
    my $str = shift;
    my $columns = shift;

    $columns = $wordwrap_default_columns if not defined $columns;
    $columns += $wordwrap_default_columns if $columns < 0;
    $Text::Wrap::columns = $columns;

    my $retval = '';

    #print("\n\nWORDWRAP:\n\n$str\n\n\n");

    $str =~ s/\A\n+//ms;

    while ($str =~ s/(.*?)(\`\`\`.*?\`\`\`|\<syntaxhighlight.*?\<\/syntaxhighlight\>)//ms) {
        #print("\n\nWORDWRAP BLOCK:\n\n$1\n\n ===\n\n$2\n\n\n");
        $retval .= wordwrap_paragraphs($1); # wrap it.
        $retval .= "$2\n\n";  # don't wrap it.
    }

    $retval .= wordwrap_paragraphs($str);  # wrap what's left.
    $retval =~ s/\n+\Z//ms;

    #print("\n\nWORDWRAP DONE:\n\n$retval\n\n\n");
    return $retval;
}

# This assumes you're moving from Markdown (in the Doxygen data) to Wiki, which
#  is why the 'md' section is so sparse.
sub wikify_chunk {
    my $wikitype = shift;
    my $str = shift;
    my $codelang = shift;
    my $code = shift;

    #print("\n\nWIKIFY CHUNK:\n\n$str\n\n\n");

    if ($wikitype eq 'mediawiki') {
        # Convert obvious SDL things to wikilinks.
        $str =~ s/\b(SDL_[a-zA-Z0-9_]+)/[[$1]]/gms;

        # Make some Markdown things into MediaWiki...

        # <code></code> is also popular.  :/
        $str =~ s/\`(.*?)\`/<code>$1<\/code>/gms;

        # bold+italic
        $str =~ s/\*\*\*(.*?)\*\*\*/'''''$1'''''/gms;

        # bold
        $str =~ s/\*\*(.*?)\*\*/'''$1'''/gms;

        # italic
        $str =~ s/\*(.*?)\*/''$1''/gms;

        # bullets
        $str =~ s/^\- /* /gm;

        if (defined $code) {
            $str .= "<syntaxhighlight lang='$codelang'>$code<\/syntaxhighlight>";
        }
    } elsif ($wikitype eq 'md') {
        # Convert obvious SDL things to wikilinks.
        $str =~ s/\b(SDL_[a-zA-Z0-9_]+)/[$1]($1)/gms;
        if (defined $code) {
            $str .= "```$codelang$code```";
        }
    }

    #print("\n\nWIKIFY CHUNK DONE:\n\n$str\n\n\n");

    return $str;
}

sub wikify {
    my $wikitype = shift;
    my $str = shift;
    my $retval = '';

    #print("WIKIFY WHOLE:\n\n$str\n\n\n");

    while ($str =~ s/\A(.*?)\`\`\`(c\+\+|c)(.*?)\`\`\`//ms) {
        $retval .= wikify_chunk($wikitype, $1, $2, $3);
    }
    $retval .= wikify_chunk($wikitype, $str, undef, undef);

    #print("WIKIFY WHOLE DONE:\n\n$retval\n\n\n");

    return $retval;
}


sub dewikify_chunk {
    my $wikitype = shift;
    my $str = shift;
    my $codelang = shift;
    my $code = shift;

    #print("\n\nDEWIKIFY CHUNK:\n\n$str\n\n\n");

    if ($wikitype eq 'mediawiki') {
        # Doxygen supports Markdown (and it just simply looks better than MediaWiki
        # when looking at the raw headers), so do some conversions here as necessary.

        $str =~ s/\[\[(SDL_[a-zA-Z0-9_]+)\]\]/$1/gms;  # Dump obvious wikilinks.

        # <code></code> is also popular.  :/
        $str =~ s/\<code>(.*?)<\/code>/`$1`/gms;

        # bold+italic
        $str =~ s/\'''''(.*?)'''''/***$1***/gms;

        # bold
        $str =~ s/\'''(.*?)'''/**$1**/gms;

        # italic
        $str =~ s/\''(.*?)''/*$1*/gms;

        # bullets
        $str =~ s/^\* /- /gm;
    }

    if (defined $code) {
        $str .= "```$codelang$code```";
    }

    #print("\n\nDEWIKIFY CHUNK DONE:\n\n$str\n\n\n");

    return $str;
}

sub dewikify {
    my $wikitype = shift;
    my $str = shift;
    return '' if not defined $str;

    #print("DEWIKIFY WHOLE:\n\n$str\n\n\n");

    $str =~ s/\A[\s\n]*\= .*? \=\s*?\n+//ms;
    $str =~ s/\A[\s\n]*\=\= .*? \=\=\s*?\n+//ms;

    my $retval = '';
    while ($str =~ s/\A(.*?)<syntaxhighlight lang='?(.*?)'?>(.*?)<\/syntaxhighlight\>//ms) {
        $retval .= dewikify_chunk($wikitype, $1, $2, $3);
    }
    $retval .= dewikify_chunk($wikitype, $str, undef, undef);

    #print("DEWIKIFY WHOLE DONE:\n\n$retval\n\n\n");

    return $retval;
}

sub usage {
    die("USAGE: $0 <source code git clone path> <wiki git clone path> [--copy-to-headers|--copy-to-wiki] [--warn-about-missing]\n\n");
}

usage() if not defined $srcpath;
usage() if not defined $wikipath;
#usage() if $copy_direction == 0;

my @standard_wiki_sections = (
    'Draft',
    '[Brief]',
    'Syntax',
    'Function Parameters',
    'Return Value',
    'Remarks',
    'Version',
    'Code Examples',
    'Related Functions'
);

# Sections that only ever exist in the wiki and shouldn't be deleted when
#  not found in the headers.
my %only_wiki_sections = (  # The ones don't mean anything, I just need to check for key existence.
    'Draft', 1,
    'Code Examples', 1
);


my %headers = ();       # $headers{"SDL_audio.h"} -> reference to an array of all lines of text in SDL_audio.h.
my %headerfuncs = ();   # $headerfuncs{"SDL_OpenAudio"} -> string of header documentation for SDL_OpenAudio, with comment '*' bits stripped from the start. Newlines embedded!
my %headerdecls = ();
my %headerfuncslocation = ();   # $headerfuncslocation{"SDL_OpenAudio"} -> name of header holding SDL_OpenAudio define ("SDL_audio.h" in this case).
my %headerfuncschunk = ();   # $headerfuncschunk{"SDL_OpenAudio"} -> offset in array in %headers that should be replaced for this function.

my $incpath = "$srcpath/include";
opendir(DH, $incpath) or die("Can't opendir '$incpath': $!\n");
while (readdir(DH)) {
    my $dent = $_;
    next if not $dent =~ /\ASDL.*?\.h\Z/;  # just SDL*.h headers.
    open(FH, '<', "$incpath/$dent") or die("Can't open '$incpath/$dent': $!\n");

    my @contents = ();

    while (<FH>) {
        chomp;
        if (not /\A\/\*\*\s*\Z/) {  # not doxygen comment start?
            push @contents, $_;
            next;
        }

        my @templines = ();
        push @templines, $_;
        my $str = '';
        while (<FH>) {
            chomp;
            push @templines, $_;
            last if /\A\s*\*\/\Z/;
            if (s/\A\s*\*\s*\`\`\`/```/) {  # this is a hack, but a lot of other code relies on the whitespace being trimmed, but we can't trim it in code blocks...
                $str .= "$_\n";
                while (<FH>) {
                    chomp;
                    push @templines, $_;
                    s/\A\s*\*\s?//;
                    if (s/\A\s*\`\`\`/```/) {
                        $str .= "$_\n";
                        last;
                    } else {
                        $str .= "$_\n";
                    }
                }
            } else {
                s/\A\s*\*\s*//;
                $str .= "$_\n";
            }
        }

        my $decl = <FH>;
        chomp($decl);
        if (not $decl =~ /\A\s*extern\s+DECLSPEC/) {
            #print "Found doxygen but no function sig:\n$str\n\n";
            foreach (@templines) {
                push @contents, $_;
            }
            push @contents, $decl;
            next;
        }

        my @decllines = ( $decl );

        if (not $decl =~ /\)\s*;/) {
            while (<FH>) {
                chomp;
                push @decllines, $_;
                s/\A\s+//;
                s/\s+\Z//;
                $decl .= " $_";
                last if /\)\s*;/;
            }
        }

        $decl =~ s/\s+\);\Z/);/;
        $decl =~ s/\s+\Z//;
        #print("DECL: [$decl]\n");

        my $fn = '';
        if ($decl =~ /\A\s*extern\s+DECLSPEC\s+(const\s+|)(unsigned\s+|)(.*?)\s*(\*?)\s*SDLCALL\s+(.*?)\s*\((.*?)\);/) {
            $fn = $5;
            #$decl =~ s/\A\s*extern\s+DECLSPEC\s+(.*?)\s+SDLCALL/$1/;
        } else {
            #print "Found doxygen but no function sig:\n$str\n\n";
            foreach (@templines) {
                push @contents, $_;
            }
            foreach (@decllines) {
                push @contents, $_;
            }
            next;
        }

        $decl = '';  # build this with the line breaks, since it looks better for syntax highlighting.
        foreach (@decllines) {
            if ($decl eq '') {
                $decl = $_;
                $decl =~ s/\Aextern\s+DECLSPEC\s+(.*?)\s+(\*?)SDLCALL\s+/$1$2 /;
            } else {
                my $trimmed = $_;
                $trimmed =~ s/\A\s{24}//;  # 24 for shrinking to match the removed "extern DECLSPEC SDLCALL "
                $decl .= $trimmed;
            }
            $decl .= "\n";
        }

        #print("$fn:\n$str\n\n");

        $headerfuncs{$fn} = $str;
        $headerdecls{$fn} = $decl;
        $headerfuncslocation{$fn} = $dent;
        $headerfuncschunk{$fn} = scalar(@contents);

        push @contents, join("\n", @templines);
        push @contents, join("\n", @decllines);
    }
    close(FH);

    $headers{$dent} = \@contents;
}
closedir(DH);


# !!! FIXME: we need to parse enums and typedefs and structs and defines and and and and and...
# !!! FIXME:  (but functions are good enough for now.)

my %wikitypes = ();  # contains string of wiki page extension, like $wikitypes{"SDL_OpenAudio"} == 'mediawiki'
my %wikifuncs = ();  # contains references to hash of strings, each string being the full contents of a section of a wiki page, like $wikifuncs{"SDL_OpenAudio"}{"Remarks"}.
my %wikisectionorder = ();   # contains references to array, each array item being a key to a wikipage section in the correct order, like $wikisectionorder{"SDL_OpenAudio"}[2] == 'Remarks'
opendir(DH, $wikipath) or die("Can't opendir '$wikipath': $!\n");
while (readdir(DH)) {
    my $dent = $_;
    my $type = '';
    if ($dent =~ /\ASDL.*?\.(md|mediawiki)\Z/) {
        $type = $1;
    } else {
        next;  # only dealing with wiki pages.
    }

    open(FH, '<', "$wikipath/$dent") or die("Can't open '$wikipath/$dent': $!\n");

    my $current_section = '[start]';
    my @section_order = ( $current_section );
    my $fn = $dent;
    $fn =~ s/\..*\Z//;
    my %sections = ();
    $sections{$current_section} = '';

    while (<FH>) {
        chomp;
        my $orig = $_;
        s/\A\s*//;
        s/\s*\Z//;

        if ($type eq 'mediawiki') {
            if (/\A\= (.*?) \=\Z/) {
                $current_section = ($1 eq $fn) ? '[Brief]' : $1;
                die("Doubly-defined section '$current_section' in '$dent'!\n") if defined $sections{$current_section};
                push @section_order, $current_section;
                $sections{$current_section} = '';
            } elsif (/\A\=\= (.*?) \=\=\Z/) {
                $current_section = ($1 eq $fn) ? '[Brief]' : $1;
                die("Doubly-defined section '$current_section' in '$dent'!\n") if defined $sections{$current_section};
                push @section_order, $current_section;
                $sections{$current_section} = '';
                next;
            } elsif (/\A\-\-\-\-\Z/) {
                $current_section = '[footer]';
                die("Doubly-defined section '$current_section' in '$dent'!\n") if defined $sections{$current_section};
                push @section_order, $current_section;
                $sections{$current_section} = '';
                next;
            }
        } elsif ($type eq 'md') {
            if (/\A\#+ (.*?)\Z/) {
                $current_section = ($1 eq $fn) ? '[Brief]' : $1;
                die("Doubly-defined section '$current_section' in '$dent'!\n") if defined $sections{$current_section};
                push @section_order, $current_section;
                $sections{$current_section} = '';
                next;
            } elsif (/\A\-\-\-\-\Z/) {
                $current_section = '[footer]';
                die("Doubly-defined section '$current_section' in '$dent'!\n") if defined $sections{$current_section};
                push @section_order, $current_section;
                $sections{$current_section} = '';
                next;
            }
        } else {
            die("Unexpected wiki file type. Fixme!\n");
        }

        $sections{$current_section} .= "$orig\n";
    }
    close(FH);

    foreach (keys %sections) {
        $sections{$_} =~ s/\A\n+//;
        $sections{$_} =~ s/\n+\Z//;
        $sections{$_} .= "\n";
    }

    if (0) {
        foreach (@section_order) {
            print("$fn SECTION '$_':\n");
            print($sections{$_});
            print("\n\n");
        }
    }

    $wikitypes{$fn} = $type;
    $wikifuncs{$fn} = \%sections;
    $wikisectionorder{$fn} = \@section_order;
}
closedir(DH);


if ($warn_about_missing) {
    foreach (keys %wikifuncs) {
        my $fn = $_;
        if (not defined $headerfuncs{$fn}) {
            print("WARNING: $fn defined in the wiki but not the headers!\n");
        }
    }

    foreach (keys %headerfuncs) {
        my $fn = $_;
        if (not defined $wikifuncs{$fn}) {
            print("WARNING: $fn defined in the headers but not the wiki!\n");
        }
    }
}

if ($copy_direction == 1) {  # --copy-to-headers
    my %changed_headers = ();

    $wordwrap_mode = 'md';   # the headers use Markdown format.

    # if it's not in the headers already, we don't add it, so iterate what we know is already there for changes.
    foreach (keys %headerfuncs) {
        my $fn = $_;
        next if not defined $wikifuncs{$fn};  # don't have a page for that function, skip it.
        my $wikitype = $wikitypes{$fn};
        my $sectionsref = $wikifuncs{$fn};
        my $remarks = %$sectionsref{'Remarks'};
        my $params = %$sectionsref{'Function Parameters'};
        my $returns = %$sectionsref{'Return Value'};
        my $version = %$sectionsref{'Version'};
        my $related = %$sectionsref{'Related Functions'};
        my $brief = %$sectionsref{'[Brief]'};
        my $addblank = 0;
        my $str = '';

        $brief = dewikify($wikitype, $brief);
        $brief =~ s/\A(.*?\.) /$1\n/;  # \brief should only be one sentence, delimited by a period+space. Split if necessary.
        my @briefsplit = split /\n/, $brief;
        $brief = shift @briefsplit;

        if (defined $remarks) {
            $remarks = join("\n", @briefsplit) . dewikify($wikitype, $remarks);
        }

        if (defined $brief) {
            $str .= "\n" if $addblank; $addblank = 1;
            $str .= wordwrap($brief) . "\n";
        }

        if (defined $remarks) {
            $str .= "\n" if $addblank; $addblank = 1;
            $str .= wordwrap($remarks) . "\n";
        }

        if (defined $params) {
            $str .= "\n" if $addblank; $addblank = (defined $returns) ? 0 : 1;
            my @lines = split /\n/, dewikify($wikitype, $params);
            if ($wikitype eq 'mediawiki') {
                die("Unexpected data parsing MediaWiki table") if (shift @lines ne '{|');  # Dump the '{|' start
                while (scalar(@lines) >= 3) {
                    my $name = shift @lines;
                    my $desc = shift @lines;
                    my $terminator = shift @lines;  # the '|-' or '|}' line.
                    last if ($terminator ne '|-') and ($terminator ne '|}');  # we seem to have run out of table.
                    $name =~ s/\A\|\s*//;
                    $name =~ s/\A\*\*(.*?)\*\*/$1/;
                    $name =~ s/\A\'\'\'(.*?)\'\'\'/$1/;
                    $desc =~ s/\A\|\s*//;
                    #print STDERR "FN: $fn   NAME: $name   DESC: $desc TERM: $terminator\n";
                    my $whitespacelen = length($name) + 8;
                    my $whitespace = ' ' x $whitespacelen;
                    $desc = wordwrap($desc, -$whitespacelen);
                    my @desclines = split /\n/, $desc;
                    my $firstline = shift @desclines;
                    $str .= "\\param $name $firstline\n";
                    foreach (@desclines) {
                        $str .= "${whitespace}$_\n";
                    }
                }
            } else {
                die("write me");
            }
        }

        if (defined $returns) {
            $str .= "\n" if $addblank; $addblank = 1;
            my $r = dewikify($wikitype, $returns);
            my $retstr = "\\returns";
            if ($r =~ s/\AReturn(s?) //) {
                $retstr = "\\return$1";
            }

            my $whitespacelen = length($retstr) + 1;
            my $whitespace = ' ' x $whitespacelen;
            $r = wordwrap($r, -$whitespacelen);
            my @desclines = split /\n/, $r;
            my $firstline = shift @desclines;
            $str .= "$retstr $firstline\n";
            foreach (@desclines) {
                $str .= "${whitespace}$_\n";
            }
        }

        if (defined $version) {
            # !!! FIXME: lots of code duplication in all of these.
            $str .= "\n" if $addblank; $addblank = 1;
            my $v = dewikify($wikitype, $version);
            my $whitespacelen = length("\\since") + 1;
            my $whitespace = ' ' x $whitespacelen;
            $v = wordwrap($v, -$whitespacelen);
            my @desclines = split /\n/, $v;
            my $firstline = shift @desclines;
            $str .= "\\since $firstline\n";
            foreach (@desclines) {
                $str .= "${whitespace}$_\n";
            }
        }

        if (defined $related) {
            # !!! FIXME: lots of code duplication in all of these.
            $str .= "\n" if $addblank; $addblank = 1;
            my $v = dewikify($wikitype, $related);
            my @desclines = split /\n/, $v;
            foreach (@desclines) {
                s/\A(\:|\* )//;
                s/\(\)\Z//;  # Convert "SDL_Func()" to "SDL_Func"
                $str .= "\\sa $_\n";
            }
        }

        my @lines = split /\n/, $str;
        my $output = "/**\n";
        foreach (@lines) {
            chomp;
            s/\s*\Z//;
            if ($_ eq '') {
                $output .= " *\n";
            } else {
                $output .= " * $_\n";
            }
        }
        $output .= " */";

        #print("$fn:\n$output\n\n");

        my $header = $headerfuncslocation{$fn};
        my $chunk = $headerfuncschunk{$fn};
        my $contentsref = $headers{$header};
        $$contentsref[$chunk] = $output;
        #$$contentsref[$chunk+1] = $headerdecls{$fn};

        $changed_headers{$header} = 1;
    }

    foreach (keys %changed_headers) {
        my $contentsref = $headers{$_};
        my $path = "$incpath/$_.tmp";
        open(FH, '>', $path) or die("Can't open '$path': $!\n");
        foreach (@$contentsref) {
            print FH "$_\n";
        }
        close(FH);
        rename($path, "$incpath/$_") or die("Can't rename '$path' to '$incpath/$_': $!\n");
    }

} elsif ($copy_direction == -1) { # --copy-to-wiki
    foreach (keys %headerfuncs) {
        my $fn = $_;
        my $wikitype = defined $wikitypes{$fn} ? $wikitypes{$fn} : 'mediawiki';  # default to MediaWiki for new stuff FOR NOW.
        die("Unexpected wikitype '$wikitype'\n") if (($wikitype ne 'mediawiki') and ($wikitype ne 'md'));

        #print("$fn\n"); next;

        $wordwrap_mode = $wikitype;

        my $raw = $headerfuncs{$fn};  # raw doxygen text with comment characters stripped from start/end and start of each line.
        $raw =~ s/\A\s*\\brief\s+//;  # Technically we don't need \brief (please turn on JAVADOC_AUTOBRIEF if you use Doxygen), so just in case one is present, strip it.

        my @doxygenlines = split /\n/, $raw;
        my $brief = '';
        while (@doxygenlines) {
            last if $doxygenlines[0] =~ /\A\\/;  # some sort of doxygen command, assume we're past the general remarks.
            last if $doxygenlines[0] =~ /\A\s*\Z/;  # blank line? End of paragraph, done.
            my $l = shift @doxygenlines;
            chomp($l);
            $l =~ s/\A\s*//;
            $l =~ s/\s*\Z//;
            $brief .= "$l ";
        }

        $brief =~ s/\A(.*?\.) /$1\n\n/;  # \brief should only be one sentence, delimited by a period+space. Split if necessary.
        my @briefsplit = split /\n/, $brief;
        $brief = wikify($wikitype, shift @briefsplit) . "\n";
        @doxygenlines = (@briefsplit, @doxygenlines);

        my $remarks = '';
        # !!! FIXME: wordwrap and wikify might handle this, now.
        while (@doxygenlines) {
            last if $doxygenlines[0] =~ /\A\\/;  # some sort of doxygen command, assume we're past the general remarks.
            my $l = shift @doxygenlines;
            if ($l =~ /\A\`\`\`/) {  # syntax highlighting, don't reformat.
                $remarks .= "$l\n";
                while ((@doxygenlines) && (not $l =~ /\`\`\`\Z/)) {
                    $l = shift @doxygenlines;
                    $remarks .= "$l\n";
                }
            } else {
                $l =~ s/\A\s*//;
                $l =~ s/\s*\Z//;
                $remarks .= "$l\n";
            }
        }

        #print("REMARKS:\n\n $remarks\n\n");

        $remarks = wordwrap(wikify($wikitype, $remarks));
        $remarks =~ s/\A\s*//;
        $remarks =~ s/\s*\Z//;

        my $decl = $headerdecls{$fn};
        #$decl =~ s/\*\s+SDLCALL/ *SDLCALL/;  # Try to make "void * Function" become "void *Function"
        #$decl =~ s/\A\s*extern\s+DECLSPEC\s+(.*?)\s+(\*?)SDLCALL/$1$2/;

        my $syntax = '';
        if ($wikitype eq 'mediawiki') {
            $syntax = "<syntaxhighlight lang='c'>\n$decl</syntaxhighlight>\n";
        } elsif ($wikitype eq 'md') {
            $syntax = "```c\n$decl\n```\n";
        } else { die("Expected wikitype '$wikitype'\n"); }

        my %sections = ();
        $sections{'[Brief]'} = $brief;  # include this section even if blank so we get a title line.
        $sections{'Remarks'} = "$remarks\n" if $remarks ne '';
        $sections{'Syntax'} = $syntax;

        my @params = ();  # have to parse these and build up the wiki tables after, since Markdown needs to know the length of the largest string.  :/

        while (@doxygenlines) {
            my $l = shift @doxygenlines;
            if ($l =~ /\A\\param\s+(.*?)\s+(.*)\Z/) {
                my $arg = $1;
                my $desc = $2;
                while (@doxygenlines) {
                    my $subline = $doxygenlines[0];
                    $subline =~ s/\A\s*//;
                    last if $subline =~ /\A\\/;  # some sort of doxygen command, assume we're past this thing.
                    shift @doxygenlines;  # dump this line from the array; we're using it.
                    if ($subline eq '') {  # empty line, make sure it keeps the newline char.
                        $desc .= "\n";
                    } else {
                        $desc .= " $subline";
                    }
                }

                $desc =~ s/[\s\n]+\Z//ms;

                # We need to know the length of the longest string to make Markdown tables, so we just store these off until everything is parsed.
                push @params, $arg;
                push @params, $desc;
            } elsif ($l =~ /\A\\r(eturns?)\s+(.*)\Z/) {
                my $retstr = "R$1";  # "Return" or "Returns"
                my $desc = $2;
                while (@doxygenlines) {
                    my $subline = $doxygenlines[0];
                    $subline =~ s/\A\s*//;
                    last if $subline =~ /\A\\/;  # some sort of doxygen command, assume we're past this thing.
                    shift @doxygenlines;  # dump this line from the array; we're using it.
                    if ($subline eq '') {  # empty line, make sure it keeps the newline char.
                        $desc .= "\n";
                    } else {
                        $desc .= " $subline";
                    }
                }
                $desc =~ s/[\s\n]+\Z//ms;
                $sections{'Return Value'} = wordwrap("$retstr " . wikify($wikitype, $desc)) . "\n";
            } elsif ($l =~ /\A\\since\s+(.*)\Z/) {
                my $desc = $1;
                while (@doxygenlines) {
                    my $subline = $doxygenlines[0];
                    $subline =~ s/\A\s*//;
                    last if $subline =~ /\A\\/;  # some sort of doxygen command, assume we're past this thing.
                    shift @doxygenlines;  # dump this line from the array; we're using it.
                    if ($subline eq '') {  # empty line, make sure it keeps the newline char.
                        $desc .= "\n";
                    } else {
                        $desc .= " $subline";
                    }
                }
                $desc =~ s/[\s\n]+\Z//ms;
                $sections{'Version'} = wordwrap(wikify($wikitype, $desc)) . "\n";
            } elsif ($l =~ /\A\\sa\s+(.*)\Z/) {
                my $sa = $1;
                $sa =~ s/\(\)\Z//;  # Convert "SDL_Func()" to "SDL_Func"
                $sections{'Related Functions'} = '' if not defined $sections{'Related Functions'};
                if ($wikitype eq 'mediawiki') {
                    $sections{'Related Functions'} .= ":[[$sa]]\n";
                } elsif ($wikitype eq 'md') {
                    $sections{'Related Functions'} .= "* [$sa](/$sa)\n";
                } else { die("Expected wikitype '$wikitype'\n"); }
            }
        }

        # Make sure this ends with a double-newline.
        $sections{'Related Functions'} .= "\n" if defined $sections{'Related Functions'};

        # We can build the wiki table now that we have all the data.
        if (scalar(@params) > 0) {
            my $str = '';
            if ($wikitype eq 'mediawiki') {
                while (scalar(@params) > 0) {
                    my $arg = shift @params;
                    my $desc = wikify($wikitype, shift @params);
                    $str .= ($str eq '') ? "{|\n" : "|-\n";
                    $str .= "|'''$arg'''\n";
                    $str .= "|$desc\n";
                }
                $str .= "|}\n";
            } elsif ($wikitype eq 'md') {
                my $longest_arg = 0;
                my $longest_desc = 0;
                my $which = 0;
                foreach (@params) {
                    if ($which == 0) {
                        my $len = length($_) + 4;
                        $longest_arg = $len if ($len > $longest_arg);
                        $which = 1;
                    } else {
                        my $len = length(wikify($wikitype, $_));
                        $longest_desc = $len if ($len > $longest_desc);
                        $which = 0;
                    }
                }

                # Markdown tables are sort of obnoxious.
                $str .= '| ' . (' ' x ($longest_arg+4)) . ' | ' . (' ' x $longest_desc) . " |\n";
                $str .= '| ' . ('-' x ($longest_arg+4)) . ' | ' . ('-' x $longest_desc) . " |\n";

                while (@params) {
                    my $arg = shift @params;
                    my $desc = wikify($wikitype, shift @params);
                    $str .= "| **$arg** " . (' ' x ($longest_arg - length($arg))) . "| $desc" . (' ' x ($longest_desc - length($desc))) . " |\n";
                }
            } else {
                die("Unexpected wikitype!\n");  # should have checked this elsewhere.
            }
            $sections{'Function Parameters'} = $str;
        }

        my $path = "$wikipath/$_.${wikitype}.tmp";
        open(FH, '>', $path) or die("Can't open '$path': $!\n");

        my $sectionsref = $wikifuncs{$fn};

        foreach (@standard_wiki_sections) {
            # drop sections we either replaced or removed from the original wiki's contents.
            if (not defined $only_wiki_sections{$_}) {
                delete($$sectionsref{$_});
            }
        }

        my $wikisectionorderref = $wikisectionorder{$fn};
        my @ordered_sections = (@standard_wiki_sections, defined $wikisectionorderref ? @$wikisectionorderref : ());  # this copies the arrays into one.

        foreach (@ordered_sections) {
            my $sect = $_;
            next if $sect eq '[start]';
            next if (not defined $sections{$sect} and not defined $$sectionsref{$sect});
            my $section = defined $sections{$sect} ? $sections{$sect} : $$sectionsref{$sect};
            if ($sect eq '[footer]') {
                print FH "----\n";   # It's the same in Markdown and MediaWiki.
            } elsif ($sect eq '[Brief]') {
                if ($wikitype eq 'mediawiki') {
                    print FH  "= $fn =\n\n";
                } elsif ($wikitype eq 'md') {
                    print FH "# $fn\n\n";
                } else { die("Expected wikitype '$wikitype'\n"); }
            } else {
                if ($wikitype eq 'mediawiki') {
                    print FH  "\n== $sect ==\n\n";
                } elsif ($wikitype eq 'md') {
                    print FH "\n## $sect\n\n";
                } else { die("Expected wikitype '$wikitype'\n"); }
            }

            print FH defined $sections{$sect} ? $sections{$sect} : $$sectionsref{$sect};

            # make sure these don't show up twice.
            delete($sections{$sect});
            delete($$sectionsref{$sect});
        }

        print FH "\n\n";
        close(FH);
        rename($path, "$wikipath/$_.${wikitype}") or die("Can't rename '$path' to '$wikipath/$_.${wikitype}': $!\n");
    }
}

# end of wikiheaders.pl ...

