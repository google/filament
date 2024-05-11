HTTP/1.0 200 OK
Content-Type: text/html

<html>
  <p>Prime numbers from 0 to 100, calculated by Lua:</p>
 <?
    function is_prime(n)
      if n <= 0 then return false end
      if n <= 2 then return true end
      if (n % 2 == 0) then return false end
      for i = 3, n / 2, 2 do
        if (n % i == 0) then return false end
      end
      return true
    end

    for i = 1, 100 do
      if is_prime(i) then mg.write('<span>' .. i .. '</span>&nbsp;') end
    end

 ?>

 <p>Reading POST data from Lua (click submit):</p>
 <form method="POST" ><input type="text" name="t1"/><input type="submit"></form>

 <pre>
   POST data: [<?
      local post_data = ''
      if mg.request_info.request_method == 'POST' then
        post_data = mg.read()
      end
      mg.write(post_data)
    ?>]
   request method: [<? mg.write(mg.request_info.request_method) ?>]
   IP/port: [<? mg.write(mg.request_info.remote_ip, ':',
                         mg.request_info.remote_port) ?>]
   URI: [<? mg.write(mg.request_info.uri) ?>]
   HTTP version [<? mg.write(mg.request_info.http_version) ?>]
   HEADERS:
   <?
     for name, value in pairs(mg.request_info.http_headers) do
       mg.write(name, ':', value, '\n')
     end
   ?>
</pre>
</html>
