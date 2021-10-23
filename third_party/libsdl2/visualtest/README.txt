/*!

\mainpage Visual and Interactive Test Automation for SDL 2.0

\section license_sec License
Check the file \c LICENSE.txt for licensing information.

\section intro_sec Introduction
The goal of this GSoC project is to automate the testing of testsprite2.
testsprite2 takes 26 parameters which have thousands of valid combinations and is
used to validate SDL's window, mouse and rendering behaviour. By having a test
harness that runs testsprite2 with various command line argument strings and
validates the output for each run, we can make testing an easier task for
maintainers, contributors and testers. The test harness can be used by a continuous
integration system (like buildbot or jenkins) to validate SDL after checkins.

SDL Homepage: http://libsdl.org/

\section build_sec Building

\subsection build_linux Building on Linux/Cygwin
<tt>./autogen.sh; ./configure; make;</tt>

\subsection build_windows Building on Windows
Use the Visual Studio solution under \c SDL/VisualC/visualtest.

\section docs_sec Documentation
Documentation is available via Doxygen. To build the documentation, cd to the
SDL/visualtest/docs directory and run \c doxygen. A good starting point for
exploring the documentation is \c SDL/visualtest/docs/html/index.html

\section usage_sec Usage
To see all the options supported by the test harness, just run \c testharness
with no arguments.

At the moment the following options are supported:
\li \c sutapp - Path to the system under test (SUT) application
\li \c sutargs - Launch the SUT with the specified arguments string
\li \c timeout - The maximum time after which the SUT process will be killed;
	passed as hh:mm:ss; default 00:01:00
\li \c variator - Which variator to use; see \ref variators_sec
\li \c num-variations - The number of variations to run for; taken to be 
	1 for the random variator and ALL for the exhaustive variator by default
\li \c no-launch - Just print the arguments string for each variation without
	launching the SUT or performing any actions
\li \c parameter-config - A config file that describes the command line parameters
	supported by the SUT; see \ref paramconfig_sec or the sample *.parameters files
	for more details
\li \c action-config - A config file with a list of actions to be performed while
	the SUT is running; see \ref actionconfig_sec or the sample *.actions files
\li \c output-dir - Path to the directory where screenshots should be saved; is
	created if it doesn't exist; taken to be "./output" by default
\li \c verify-dir - Path to the directory with the verification images; taken to 
	be "./verify" by default

Paths can be relative or absolute.

Alternatively, the options can be passed as a config file for convenience:

<tt>testharness \-\-config testsprite2_sample.config</tt>

For a sample, take a look at the *.config files in this repository.

We can also pass a config file and override certain options as necessary:
<tt>testharness \-\-config testsprite2_sample.config \-\-num-variations 10</tt>

Note: You may find it convenient to copy the SUT executable along with any
resources to the test harness directory. Also note that testsprite2 and its
resources (icon.bmp) are automatically copied when using the Visual Studio
solution.

\subsection usageexamples_subsec Usage examples:

Passing a custom arguments string:
<tt>testharness \-\-sutapp testsprite2 \-\-sutargs "\-\-cyclecolor \-\-blend mod
\-\-iterations 2" \-\-action-config xyz.actions</tt>

Using the random variator:
<tt>testharness \-\-sutapp testsprite2 \-\-variator random \-\-num-variations 5
\-\-parameter-config xyz.parameters \-\-action-config xyz.actions</tt>

\subsection config_subsec Config Files
Config files are an alternate way to pass parameters to the test harness. We
describe the paramters in a config file and pass that to the test harness using
the \-\-config option. The config file consists of lines of the form "x=y" where
x is an option and y is it's value. For boolean options, we simply give the name
of the option to indicate that it is to be passed to the testharness.

The hash '#' character can be used to start a comment from that point to the end
of the line.

\section paramconfig_sec The SUT Parameters File
To generate variations we need to describe the parameters the will be passed to
the SUT. This description is given in a parameters file. Each line of the parameters
file (except the blank lines) represents one command line option with five
comma separated fields:
<tt>name, type, values, required, categories</tt>

\li \c name is the name of the option, e.g., \c \-\-cyclecolor.
\li \c type can have one of three values - integer, boolean and enum.
\li \c values - for integer options this is the valid range of values the option
	can take, i.e., [min max]. For enum options this is a list of strings that
	the option can take, e.g., [val1 val2 val3]. For boolean options this field
	is ignored.
\li \c required - true if the option is required, false otherwise.
\li \c categories - a list of categories that the option belongs to. For example,
	[video mouse audio]

Just like with config files, hash characters can be used to start comments.

\subsection additionalnotes_subsec Additional Notes

\li If you want to have an option that always takes a certain value, use an enum
	with only one value.
\li Currently there isn't any way to turn an option off, i.e., all options will
	be included in the command line options string that is generated using the
	config. If you don't want an option to be passed to the SUT, remove it from
	the config file or comment it out.

\section variators_sec Variators
Variators are the mechanism by which we generate strings of command line arguments
to test the SUT with. A variator is quite simply an iterator that iterates through
different variations of command line options. There are two variators supported at
the moment:
\li \b Exhaustive - Generate all possible combinations of command line arguments
	that are valid.
\li \b Random - Generate a random variation each time the variator is called.

As an example, let's try a simple .parameters file:\n
<tt>
\-\-blend, enum, [add mod], false, [] \n
\-\-fullscreen, boolean, [], false, []
</tt>

The exhaustive variator would generate the following four variations:\n
<tt>
\-\-blend add \n
\-\-blend mod \n
\-\-blend add \-\-fullscreen \n
\-\-blend mod \-\-fullscreen \n
</tt>

The random variator would simply generate a random variation like the following:\n
<tt>\-\-blend mod</tt>

\section actionconfig_sec The Actions File
Once the SUT process has been launched, automated testing happens using a mechanism
called actions. A list of actions is read from a file and each action is performed
on the SUT process sequentially. Each line in the actions file describes an action.
The format for an action is <tt>hh:mm:ss ACTION_NAME additional parameters</tt>.
There are five actions supported at the moment:
\li \b SCREENSHOT - Takes a screenshot of each window owned by the SUT process. The
	images are saved as \c [hash]_[i].bmp where \c [hash] is the 32 character long
	hexadecimal MD5 hash of the arguments string that was passed to the SUT while
	launching it and \c i is the window number. i = 1 is an exceptional case
	where the \c _[i] is dropped and the filename is simply \c [hash].bmp\n
	Note: The screenshots are only of the window's client area.
\li \b VERIFY - Verifies the screenshots taken by the last SCREENSHOT action by
	comparing them against a verification image. Each \c [hash]_i.bmp image output
	by the SCREENSHOT action is compared against a \c [hash].bmp image in the
	verify-dir.
\li \b QUIT - Gracefully quits the SUT process. On Windows this means sending a
	WM_CLOSE message to each window owned by the SUT process. On Linux it means
	sending a SIGQUIT signal to the SUT process.
\li \b KILL - Forcefully kills the SUT process. This is useful when the SUT process
	doesn't respond to the QUIT action.
\li <b>LAUNCH [/path/to/executable] [args]</b> - Runs an executable with \c [args]
	as the arguments string.

Just like with config files, hash characters can be used to start comments.

\section contint_sec Continuous Integration (CI)
One of the goals of the project was to create a test harness that integrates
with CI systems to provide automated visual and interactive testing to SDL.

At the moment the test harness can be run in two modes that are useful for CI:
\li Crash testing mode - launch the SUT with every variation and all parameters,
	report to the CI if there's a crash
\li Visual testing mode - launch and visually verify the SUT for a smaller subset
	of the parameters

Look at the launch_harness.sh/launch_harness.cmd for an example scripts that run the
test harness for all variations with all parameters and report an error on a crash.
The script uses the testsprite2_crashtest config, so remember to copy those files
over to the test harness executable directory along with the script.

\section todo_sec TODOs
\li Allow specifying a clipping box along with the VERIFY action, i.e., hh:mm:ss
	VERIFY x, y, w, h
\li Add support for spaces between the equals sign in test harness config files
\li Implement the SCREENSHOT action on Linux
\li Add a pairwise variator
\li Add actions to inject keyboard/mouse events
\li Add actions to manipulate the SUT window, e.g., minimize, restore, resize
\li Add support to load and save screenshots as .pngs instead of .bmps

\section issues_sec Known Issues
\li The QUIT action does not work on a testsprite2 process with multiple windows.
	This appears to be an issue with testsprite2.
\li The SCREENSHOT action doesn't capture the testsprite2 window correctly if the
	--fullscreen option is supplied. It works with --fullscreen-desktop, however.

\section moreinfo_sec More Information

Author Contact Info:\n
Apoorv Upreti \c \<apoorvupreti@gmail.com\>

Other useful links:
- Project Repository: https://bitbucket.org/nerdap/sdlvisualtest
- Project Wiki: https://github.com/nerdap/autotestsprite2/wiki
- Project Blog: http://nerdap.github.io
- Verification images for testsprite2_blendmodes: https://www.dropbox.com/s/nm02aem76m812ng/testsprite2_blendmodes.zip
- Verification images for testsprite2_geometry: https://www.dropbox.com/s/csypwryopaslpaf/testsprite2_geometry.zip
*/
