#!/usr/bin/lua5.2

-- CivetWeb command line completion for bash

--[[ INSTALLING:

To use auto-completion for bash, you need to define a command for the bash built-in
[*complete*](https://www.gnu.org/software/bash/manual/html_node/Programmable-Completion-Builtins.html).

Create a file called "civetweb" in the completion folder.
Depending on Linux distribution and version, this might be
/usr/share/bash-completion/completions/, /etc/bash_completion or another folder.

The file has to contain just one line:
complete -C /path/to/civetweb/resources/complete.lua civetweb

The complete command script is implemented in this file.
It needs Lua 5.2 to be installed (for Debian based systems: "sudo apt-get install lua5.2").
In case lua5.2 is not located in /usr/bin/lua5.2 (see "which lua5.2"),
the first line (#!) needs to be adapted accordingly.

--]]

---------------------------------------------------------------------------------------------------

-- The bash "complete -C" has an awkward interface:
-- see https://unix.stackexchange.com/questions/250262/how-to-use-bashs-complete-or-compgen-c-command-option
-- Command line arguments
cmd = arg[1] -- typically "./civetweb" or whatever path was used
this = arg[2] -- characters already typed for the next options
last = arg[3] -- option before this one
-- Environment variables
comp_line = os.getenv("COMP_LINE") -- entire command line
comp_point = os.getenv("COMP_POINT") -- position of cursor (index)
comp_type = os.getenv("COMP_TYPE") -- type:
-- 9 for normal completion
-- 33 when listing alternatives on ambiguous completions
-- 37 for menu completion
-- 63 when tabbing between ambiguous completions
-- 64 to list completions after a partial completion


-- Debug-Print function (must use absolute path for log file)
function dp(txt)
  --[[ comment / uncomment to enable debugging
  local f = io.open("/tmp/complete.log", "a");
  f:write(txt .. "\n")
  f:close()
  --]]
end

-- Helper function: Check if files exist
function fileexists(name)
  local f = io.open(name, "r")
  if f then
    f:close()
    return true
  end
  return false
end


-- Debug logging
dp("complete: cmd=" .. cmd .. ", this=" .. this .. ", last=" .. last .. ", type=" .. comp_type)


-- Trim command line (remove spaces)
trim_comp_line = string.match(comp_line, "^%s*(.-)%s*$")
 
if (trim_comp_line == cmd) then
  -- this is the first option
  dp("Suggest --help argument")
  print("--help ")
  os.exit(0)
end

is_h = string.find(comp_line, "^%s*" .. cmd .. "%s+%-h%s")
is_h = is_h or string.find(comp_line, "^%s*" .. cmd .. "%s+%--help%s")
is_h = is_h or string.find(comp_line, "^%s*" .. cmd .. "%s+%-H%s")

if (is_h) then
  dp("If --help is used, no additional argument is allowed")
  os.exit(0)
end

is_a = string.find(comp_line, "^%s*" .. cmd .. "%s+%-A%s")
is_c = string.find(comp_line, "^%s*" .. cmd .. "%s+%-C%s")
is_i = string.find(comp_line, "^%s*" .. cmd .. "%s+%-I%s")
is_r = string.find(comp_line, "^%s*" .. cmd .. "%s+%-R%s")

if (is_i) then
  dp("If --I is used, no additional argument is allowed")
  os.exit(0)
end

-- -A and -R require the password file as second argument
htpasswd_r = ".htpasswd <mydomain.com> <username>"
htpasswd_a = htpasswd_r .. " <password>"
if (last == "-A") and (this == htpasswd_a:sub(1,#this)) then
  dp("Fill with option template for -A")
  print(htpasswd_a)
  os.exit(0)
end
if (last == "-R") and (this == htpasswd_r:sub(1,#this)) then
  dp("Fill with option template for -R")
  print(htpasswd_r)
  os.exit(0)
end
if (is_a or is_r) then
  dp("Options -A and -R have a fixed number of arguments")
  os.exit(0)
end

-- -C requires an URL, usually starting with http:// or https://
http = "http://"
if (last == "-C") and (this == http:sub(1,#this)) then
  print(http)
  print(http.. "localhost/")
  os.exit(0)
end
http = "https://"
if (last == "-C") and (this == http:sub(1,#this)) then
  print(http)
  print(http.. "localhost/")
  os.exit(0)
end
if (is_c) then
  dp("Option -C has just one argument")
  os.exit(0)
end


-- Take options directly from "--help" output of executable
optfile = "/tmp/civetweb.options"
if not fileexists(optfile) then
  dp("options file " .. optfile .. " missing")
  os.execute(cmd .. " --help > " .. optfile .. " 2>&1")
else
  dp("options file " .. optfile .. " found")
end

for l in io.lines(optfile) do
  local lkey, lval = l:match("^%s+(%-[^%s]*)%s*([^%s]*)%s*$")
  if lkey then
    local thislen = string.len(this)
    if (thislen>0) and (this == lkey:sub(1,thislen)) then
      print(lkey)
      dp("match: " .. lkey)
      keymatch = true
    end
    if last == lkey then
      valmatch = lval
    end
  end
end

if keymatch then
  -- at least one options matches
  os.exit(0)
end

if valmatch then
  -- suggest the default value
  print(valmatch)
  os.exit(0)
end

-- No context to determine next argument
dp("no specific option")
os.exit(0)

