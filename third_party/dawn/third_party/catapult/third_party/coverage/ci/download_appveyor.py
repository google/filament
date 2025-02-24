# Licensed under the Apache License: http://www.apache.org/licenses/LICENSE-2.0
# For details: https://bitbucket.org/ned/coveragepy/src/default/NOTICE.txt

"""Use the Appveyor API to download Windows artifacts."""

import os
import os.path
import sys
import zipfile

import requests


def make_auth_headers():
    """Make the authentication headers needed to use the Appveyor API."""
    with open("ci/appveyor.token") as f:
        token = f.read().strip()

    headers = {
        'Authorization': 'Bearer {0}'.format(token),
    }
    return headers


def make_url(url, **kwargs):
    """Build an Appveyor API url."""
    return "https://ci.appveyor.com/api" + url.format(**kwargs)


def get_project_build(account_project):
    """Get the details of the latest Appveyor build."""
    url = make_url("/projects/{account_project}", account_project=account_project)
    response = requests.get(url, headers=make_auth_headers())
    return response.json()


def download_latest_artifacts(account_project):
    """Download all the artifacts from the latest build."""
    build = get_project_build(account_project)
    jobs = build['build']['jobs']
    print "Build {0[build][version]}, {1} jobs: {0[build][message]}".format(build, len(jobs))
    for job in jobs:
        name = job['name'].partition(':')[2].split(',')[0].strip()
        print "  {0}: {1[status]}, {1[artifactsCount]} artifacts".format(name, job)

        url = make_url("/buildjobs/{jobid}/artifacts", jobid=job['jobId'])
        response = requests.get(url, headers=make_auth_headers())
        artifacts = response.json()

        for artifact in artifacts:
            is_zip = artifact['type'] == "Zip"
            filename = artifact['fileName']
            print "    {0}, {1} bytes".format(filename, artifact['size'])

            url = make_url(
                "/buildjobs/{jobid}/artifacts/{filename}",
                jobid=job['jobId'],
                filename=filename
            )
            download_url(url, filename, make_auth_headers())

            if is_zip:
                unpack_zipfile(filename)
                os.remove(filename)


def ensure_dirs(filename):
    """Make sure the directories exist for `filename`."""
    dirname, _ = os.path.split(filename)
    if dirname and not os.path.exists(dirname):
        os.makedirs(dirname)


def download_url(url, filename, headers):
    """Download a file from `url` to `filename`."""
    ensure_dirs(filename)
    response = requests.get(url, headers=headers, stream=True)
    if response.status_code == 200:
        with open(filename, 'wb') as f:
            for chunk in response.iter_content(16*1024):
                f.write(chunk)


def unpack_zipfile(filename):
    """Unpack a zipfile, using the names in the zip."""
    with open(filename, 'rb') as fzip:
        z = zipfile.ZipFile(fzip)
        for name in z.namelist():
            print "      extracting {0}".format(name)
            ensure_dirs(name)
            z.extract(name)


if __name__ == "__main__":
    download_latest_artifacts(sys.argv[1])
