import sys
import atheris

with atheris.instrument_imports():
    import httplib2


@atheris.instrument_func
def TestOneInput(data):
    fdp = atheris.FuzzedDataProvider(data)
    original = fdp.ConsumeUnicode(sys.maxsize)
    try:
        httplib2.urlnorm(original)
    except httplib2.RelativeURIError:
        return
    return


def main():
    atheris.Setup(sys.argv, TestOneInput, enable_python_coverage=True)
    atheris.Fuzz()


if __name__ == "__main__":
    main()
