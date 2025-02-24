import urllib.request
import os
import ssl
import zipfile

url = "https://github.com/Microsoft/WinObjC/raw/develop/deps/prebuilt/nuget/taef.redist.wlk.1.0.170206001-nativetargets.nupkg"
zipfile_name = os.path.join(
    os.environ["TEMP"], "taef.redist.wlk.1.0.170206001-nativetargets.nupkg.zip"
)
src_dir = os.environ["HLSL_SRC_DIR"]
taef_dir = os.path.join(src_dir, "external", "taef")

os.makedirs(taef_dir, exist_ok=True)

try:
    ctx = ssl.SSLContext(ssl.PROTOCOL_TLSv1_2)
    response = urllib.request.urlopen(url, context=ctx)
    f = open(zipfile_name, "wb")
    f.write(response.read())
    f.close()
except:
    print("Unable to read file with urllib, trying via powershell...")
    from subprocess import check_call

    cmd = ""
    cmd += "[System.Net.ServicePointManager]::SecurityProtocol = [System.Net.SecurityProtocolType]::Tls12;"
    cmd += (
        "(new-object System.Net.WebClient).DownloadFile('"
        + url
        + "', '"
        + zipfile_name
        + "')"
    )
    check_call(["powershell.exe", "-Command", cmd])

z = zipfile.ZipFile(zipfile_name)
z.extractall(taef_dir)
z.close()
