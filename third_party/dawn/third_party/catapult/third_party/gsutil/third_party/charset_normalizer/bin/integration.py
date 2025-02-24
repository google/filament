from requests import get, __version__
from typing import List
from charset_normalizer import detect, __version__ as __version_cn__

if __name__ == "__main__":

    print(f"requests {__version__}")
    print(f"charset_normalizer {__version_cn__}")

    files: List[str] = get("http://127.0.0.1:8080/").json()

    print("## Testing with actual files")

    for file in files:
        r = get(
            "http://127.0.0.1:8080/" + file
        )

        if r.ok is False:
            print(f"Unable to retrieve '{file}' | HTTP/{r.status_code}")
            exit(1)

        expected_encoding = detect(r.content)["encoding"]

        if expected_encoding != r.apparent_encoding:
            print(f"Integration test failed | File '{file}' | Expected '{expected_encoding}' got '{r.apparent_encoding}'")
            exit(1)

        print(f"✅✅ '{file}' OK")

    print("## Testing with edge cases")

    # Should NOT crash
    get("http://127.0.0.1:8080/edge/empty/json").json()

    print("✅✅ Empty JSON OK")

    if get("http://127.0.0.1:8080/edge/empty/plain").apparent_encoding != "utf-8":
        print("Empty payload SHOULD not return apparent_encoding != UTF-8")
        exit(1)

    print("✅✅ Empty Plain Text OK")

    r = get("http://127.0.0.1:8080/edge/gb18030/json")

    if r.apparent_encoding != "GB18030":
        print("JSON Basic Detection FAILURE (/edge/gb18030/json)")
        exit(1)

    r.json()

    print("✅✅ GB18030 JSON Encoded OK")

    print("Integration tests passed!")
