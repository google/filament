#!/usr/bin/lua5.2

usedlines = {c={}, n={}}
useddefs = {c={}, n={}}

function AddElem(tab, q)
  if (tab.c[q]) then 
    tab.c[q] = tab.c[q] + 1
  else
    tab.c[q] = 1
    tab.n[#tab.n+1]=q
  end
end

function PrintTab(tab)
  table.sort(tab.n)
  for _,n in ipairs(tab.n) do
    --print(tab.c[n], n)
    print(n)
  end
end


function noifdef(f)
  local out = {}
  local changed = false
  for l in io.lines(f) do
    local n = l:gsub("^#ifdef ([%w_]+)", "#if defined(%1)")
    n = n:gsub("^#ifndef ([%w_]+)", "#if !defined(%1)")
    out[#out+1] = (n)
    if l ~= n then
      --print(l , "-->", n)
      changed = true
    end

    if n:match("^#if") then
      local q = n:gsub("%/%*.+%*%/", "")
      q = q:gsub("%s+$", "")
      q = q:gsub("^%s+", "")
      q = q:gsub("%s+", " ")
      AddElem(usedlines, q)

      for w in q:gmatch("%(%s*([%w_]+)%s*%)") do
        AddElem(useddefs, w)
      end
    end
  end

  if changed then
    local fi = io.open(f, "w")
    for _,l in pairs(out) do
      fi:write(l .. "\n")
    end
    fi:close()   
    print(f .. " rewritten")
  end

  -- print(#out .. " lines processed")
end


path = path or ""
noifdef(path .. "src/civetweb.c")
noifdef(path .. "src/civetweb_private_lua.h")
noifdef(path .. "src/main.c")
noifdef(path .. "src/md5.inl")
noifdef(path .. "src/mod_duktape.inl")
noifdef(path .. "src/mod_lua.inl")
noifdef(path .. "src/mod_zlib.inl")
noifdef(path .. "src/sha1.inl")
noifdef(path .. "src/timer.inl")
noifdef(path .. "src/wolfssl_extras.inl")

--PrintTab(usedlines)

--print("Defines used")
PrintTab(useddefs)




