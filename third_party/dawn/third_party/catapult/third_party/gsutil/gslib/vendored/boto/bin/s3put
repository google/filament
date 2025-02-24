#!/usr/bin/env python
# Copyright (c) 2006,2007,2008 Mitch Garnaat http://garnaat.org/
#
# Permission is hereby granted, free of charge, to any person obtaining a
# copy of this software and associated documentation files (the
# "Software"), to deal in the Software without restriction, including
# without limitation the rights to use, copy, modify, merge, publish, dis-
# tribute, sublicense, and/or sell copies of the Software, and to permit
# persons to whom the Software is furnished to do so, subject to the fol-
# lowing conditions:
#
# The above copyright notice and this permission notice shall be included
# in all copies or substantial portions of the Software.
#
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
# OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABIL-
# ITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT
# SHALL THE AUTHOR BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
# WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
# OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
# IN THE SOFTWARE.
#
import getopt
import sys
import os
import boto

from boto.compat import six

try:
    # multipart portions copyright Fabian Topfstedt
    # https://gist.github.com/924094

    import math
    import mimetypes
    from multiprocessing import Pool
    from boto.s3.connection import S3Connection
    from filechunkio import FileChunkIO
    multipart_capable = True
    usage_flag_multipart_capable = """ [--multipart]"""
    usage_string_multipart_capable = """
        multipart - Upload files as multiple parts. This needs filechunkio.
                    Requires ListBucket, ListMultipartUploadParts,
                    ListBucketMultipartUploads and PutObject permissions."""
except ImportError as err:
    multipart_capable = False
    usage_flag_multipart_capable = ""
    if six.PY2:
        attribute = 'message'
    else:
        attribute = 'msg'
    usage_string_multipart_capable = '\n\n     "' + \
        getattr(err, attribute)[len('No module named '):] + \
        '" is missing for multipart support '


DEFAULT_REGION = 'us-east-1'

usage_string = """
SYNOPSIS
    s3put [-a/--access_key <access_key>] [-s/--secret_key <secret_key>]
          -b/--bucket <bucket_name> [-c/--callback <num_cb>]
          [-d/--debug <debug_level>] [-i/--ignore <ignore_dirs>]
          [-n/--no_op] [-p/--prefix <prefix>] [-k/--key_prefix <key_prefix>]
          [-q/--quiet] [-g/--grant grant] [-w/--no_overwrite] [-r/--reduced]
          [--header] [--region <name>] [--host <s3_host>]""" + \
          usage_flag_multipart_capable + """ path [path...]

    Where
        access_key - Your AWS Access Key ID.  If not supplied, boto will
                     use the value of the environment variable
                     AWS_ACCESS_KEY_ID
        secret_key - Your AWS Secret Access Key.  If not supplied, boto
                     will use the value of the environment variable
                     AWS_SECRET_ACCESS_KEY
        bucket_name - The name of the S3 bucket the file(s) should be
                      copied to.
        path - A path to a directory or file that represents the items
               to be uploaded.  If the path points to an individual file,
               that file will be uploaded to the specified bucket.  If the
               path points to a directory, it will recursively traverse
               the directory and upload all files to the specified bucket.
        debug_level - 0 means no debug output (default), 1 means normal
                      debug output from boto, and 2 means boto debug output
                      plus request/response output from httplib
        ignore_dirs - a comma-separated list of directory names that will
                      be ignored and not uploaded to S3.
        num_cb - The number of progress callbacks to display.  The default
                 is zero which means no callbacks.  If you supplied a value
                 of "-c 10" for example, the progress callback would be
                 called 10 times for each file transferred.
        prefix - A file path prefix that will be stripped from the full
                 path of the file when determining the key name in S3.
                 For example, if the full path of a file is:
                     /home/foo/bar/fie.baz
                 and the prefix is specified as "-p /home/foo/" the
                 resulting key name in S3 will be:
                     /bar/fie.baz
                 The prefix must end in a trailing separator and if it
                 does not then one will be added.
        key_prefix - A prefix to be added to the S3 key name, after any
                     stripping of the file path is done based on the
                     "-p/--prefix" option.
        reduced - Use Reduced Redundancy storage
        grant - A canned ACL policy that will be granted on each file
                transferred to S3.  The value of provided must be one
                of the "canned" ACL policies supported by S3:
                private|public-read|public-read-write|authenticated-read
        no_overwrite - No files will be overwritten on S3, if the file/key
                       exists on s3 it will be kept. This is useful for
                       resuming interrupted transfers. Note this is not a
                       sync, even if the file has been updated locally if
                       the key exists on s3 the file on s3 will not be
                       updated.
        header - key=value pairs of extra header(s) to pass along in the
                 request
        region - Manually set a region for buckets that are not in the US
                 classic region. Normally the region is autodetected, but
                 setting this yourself is more efficient.
        host - Hostname override, for using an endpoint other then AWS S3
""" + usage_string_multipart_capable + """


     If the -n option is provided, no files will be transferred to S3 but
     informational messages will be printed about what would happen.
"""


def usage(status=1):
    print(usage_string)
    sys.exit(status)


def submit_cb(bytes_so_far, total_bytes):
    print('%d bytes transferred / %d bytes total' % (bytes_so_far, total_bytes))


def get_key_name(fullpath, prefix, key_prefix):
    if fullpath.startswith(prefix):
        key_name = fullpath[len(prefix):]
    else:
        key_name = fullpath
    l = key_name.split(os.sep)
    return key_prefix + '/'.join(l)


def _upload_part(bucketname, aws_key, aws_secret, multipart_id, part_num,
                 source_path, offset, bytes, debug, cb, num_cb,
                 amount_of_retries=10):
    """
    Uploads a part with retries.
    """
    if debug == 1:
        print("_upload_part(%s, %s, %s)" % (source_path, offset, bytes))

    def _upload(retries_left=amount_of_retries):
        try:
            if debug == 1:
                print('Start uploading part #%d ...' % part_num)
            conn = S3Connection(aws_key, aws_secret)
            conn.debug = debug
            bucket = conn.get_bucket(bucketname)
            for mp in bucket.get_all_multipart_uploads():
                if mp.id == multipart_id:
                    with FileChunkIO(source_path, 'r', offset=offset,
                                     bytes=bytes) as fp:
                        mp.upload_part_from_file(fp=fp, part_num=part_num,
                                                 cb=cb, num_cb=num_cb)
                    break
        except Exception as exc:
            if retries_left:
                _upload(retries_left=retries_left - 1)
            else:
                print('Failed uploading part #%d' % part_num)
                raise exc
        else:
            if debug == 1:
                print('... Uploaded part #%d' % part_num)

    _upload()

def check_valid_region(conn, region):
    if conn is None:
        print('Invalid region (%s)' % region)
        sys.exit(1)

def multipart_upload(bucketname, aws_key, aws_secret, source_path, keyname,
                     reduced, debug, cb, num_cb, acl='private', headers={},
                     guess_mimetype=True, parallel_processes=4,
                     region=DEFAULT_REGION):
    """
    Parallel multipart upload.
    """
    conn = boto.s3.connect_to_region(region, aws_access_key_id=aws_key,
                                     aws_secret_access_key=aws_secret)
    check_valid_region(conn, region)
    conn.debug = debug
    bucket = conn.get_bucket(bucketname)

    if guess_mimetype:
        mtype = mimetypes.guess_type(keyname)[0] or 'application/octet-stream'
        headers.update({'Content-Type': mtype})

    mp = bucket.initiate_multipart_upload(keyname, headers=headers,
                                          reduced_redundancy=reduced)

    source_size = os.stat(source_path).st_size
    bytes_per_chunk = max(int(math.sqrt(5242880) * math.sqrt(source_size)),
                          5242880)
    chunk_amount = int(math.ceil(source_size / float(bytes_per_chunk)))

    pool = Pool(processes=parallel_processes)
    for i in range(chunk_amount):
        offset = i * bytes_per_chunk
        remaining_bytes = source_size - offset
        bytes = min([bytes_per_chunk, remaining_bytes])
        part_num = i + 1
        pool.apply_async(_upload_part, [bucketname, aws_key, aws_secret, mp.id,
                                        part_num, source_path, offset, bytes,
                                        debug, cb, num_cb])
    pool.close()
    pool.join()

    if len(mp.get_all_parts()) == chunk_amount:
        mp.complete_upload()
        key = bucket.get_key(keyname)
        key.set_acl(acl)
    else:
        mp.cancel_upload()


def singlepart_upload(bucket, key_name, fullpath, *kargs, **kwargs):
    """
    Single upload.
    """
    k = bucket.new_key(key_name)
    k.set_contents_from_filename(fullpath, *kargs, **kwargs)


def expand_path(path):
    path = os.path.expanduser(path)
    path = os.path.expandvars(path)
    return os.path.abspath(path)


def main():

    # default values
    aws_access_key_id = None
    aws_secret_access_key = None
    bucket_name = ''
    ignore_dirs = []
    debug = 0
    cb = None
    num_cb = 0
    quiet = False
    no_op = False
    prefix = '/'
    key_prefix = ''
    grant = None
    no_overwrite = False
    reduced = False
    headers = {}
    host = None
    multipart_requested = False
    region = None

    try:
        opts, args = getopt.getopt(
            sys.argv[1:], 'a:b:c::d:g:hi:k:np:qs:wr',
            ['access_key=', 'bucket=', 'callback=', 'debug=', 'help', 'grant=',
             'ignore=', 'key_prefix=', 'no_op', 'prefix=', 'quiet',
             'secret_key=', 'no_overwrite', 'reduced', 'header=', 'multipart',
             'host=', 'region='])
    except:
        usage(1)

    # parse opts
    for o, a in opts:
        if o in ('-h', '--help'):
            usage(0)
        if o in ('-a', '--access_key'):
            aws_access_key_id = a
        if o in ('-b', '--bucket'):
            bucket_name = a
        if o in ('-c', '--callback'):
            num_cb = int(a)
            cb = submit_cb
        if o in ('-d', '--debug'):
            debug = int(a)
        if o in ('-g', '--grant'):
            grant = a
        if o in ('-i', '--ignore'):
            ignore_dirs = a.split(',')
        if o in ('-n', '--no_op'):
            no_op = True
        if o in ('-w', '--no_overwrite'):
            no_overwrite = True
        if o in ('-p', '--prefix'):
            prefix = a
            if prefix[-1] != os.sep:
                prefix = prefix + os.sep
            prefix = expand_path(prefix)
        if o in ('-k', '--key_prefix'):
            key_prefix = a
        if o in ('-q', '--quiet'):
            quiet = True
        if o in ('-s', '--secret_key'):
            aws_secret_access_key = a
        if o in ('-r', '--reduced'):
            reduced = True
        if o == '--header':
            (k, v) = a.split("=", 1)
            headers[k] = v
        if o == '--host':
            host = a
        if o == '--multipart':
            if multipart_capable:
                multipart_requested = True
            else:
                print("multipart upload requested but not capable")
                sys.exit(4)
        if o == '--region':
            regions = boto.s3.regions()
            for region_info in regions:
                if region_info.name == a:
                    region = a
                    break
            else:
                raise ValueError('Invalid region %s specified' % a)

    if len(args) < 1:
        usage(2)

    if not bucket_name:
        print("bucket name is required!")
        usage(3)

    connect_args = {
        'aws_access_key_id': aws_access_key_id,
        'aws_secret_access_key': aws_secret_access_key
    }

    if host:
        connect_args['host'] = host

    c = boto.s3.connect_to_region(region or DEFAULT_REGION, **connect_args)
    check_valid_region(c, region or DEFAULT_REGION)
    c.debug = debug
    b = c.get_bucket(bucket_name, validate=False)

    # Attempt to determine location and warn if no --host or --region
    # arguments were passed. Then try to automagically figure out
    # what should have been passed and fix it.
    if host is None and region is None:
        try:
            location = b.get_location()

            # Classic region will be '', any other will have a name
            if location:
                print('Bucket exists in %s but no host or region given!' % location)

                # Override for EU, which is really Ireland according to the docs
                if location == 'EU':
                    location = 'eu-west-1'

                print('Automatically setting region to %s' % location)

                # Here we create a new connection, and then take the existing
                # bucket and set it to use the new connection
                c = boto.s3.connect_to_region(location, **connect_args)
                c.debug = debug
                b.connection = c
        except Exception as e:
            if debug > 0:
                print(e)
            print('Could not get bucket region info, skipping...')

    existing_keys_to_check_against = []
    files_to_check_for_upload = []

    for path in args:
        path = expand_path(path)
        # upload a directory of files recursively
        if os.path.isdir(path):
            if no_overwrite:
                if not quiet:
                    print('Getting list of existing keys to check against')
                for key in b.list(get_key_name(path, prefix, key_prefix)):
                    existing_keys_to_check_against.append(key.name)
            for root, dirs, files in os.walk(path):
                for ignore in ignore_dirs:
                    if ignore in dirs:
                        dirs.remove(ignore)
                for path in files:
                    if path.startswith("."):
                        continue
                    files_to_check_for_upload.append(os.path.join(root, path))

        # upload a single file
        elif os.path.isfile(path):
            fullpath = os.path.abspath(path)
            key_name = get_key_name(fullpath, prefix, key_prefix)
            files_to_check_for_upload.append(fullpath)
            existing_keys_to_check_against.append(key_name)

        # we are trying to upload something unknown
        else:
            print("I don't know what %s is, so i can't upload it" % path)

    for fullpath in files_to_check_for_upload:
        key_name = get_key_name(fullpath, prefix, key_prefix)

        if no_overwrite and key_name in existing_keys_to_check_against:
            if b.get_key(key_name):
                if not quiet:
                    print('Skipping %s as it exists in s3' % fullpath)
                continue

        if not quiet:
            print('Copying %s to %s/%s' % (fullpath, bucket_name, key_name))

        if not no_op:
            # 0-byte files don't work and also don't need multipart upload
            if os.stat(fullpath).st_size != 0 and multipart_capable and \
                    multipart_requested:
                multipart_upload(bucket_name, aws_access_key_id,
                                 aws_secret_access_key, fullpath, key_name,
                                 reduced, debug, cb, num_cb,
                                 grant or 'private', headers,
                                 region=region or DEFAULT_REGION)
            else:
                singlepart_upload(b, key_name, fullpath, cb=cb, num_cb=num_cb,
                                  policy=grant, reduced_redundancy=reduced,
                                  headers=headers)

if __name__ == "__main__":
    main()
