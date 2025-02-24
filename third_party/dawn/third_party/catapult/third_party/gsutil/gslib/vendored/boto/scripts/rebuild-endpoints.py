"""Rebuild endpoint config.

This will pull in the latest endpoints, filtering out services that aren't
supported by boto. By default, this will pull in the endpoints from botocore,
but a specific file can also be specified.

Usage
=====

To print the newly gen'd endpoints to stdout::

    python rebuild-endpoints.py

To print the newly gen'd endpoints in the legacy format::

    python rebuild-endpoints.py --legacy-format

To overwrite the existing endpoints.json file in boto:

    python rebuild-endpoints.py --overwrite

If you have a custom upstream endpoints.json file you'd like
to use, you can provide the ``--endpoints-file``:

    python rebuild-endpoints.py --endpoints-json custom-endpoints.json

"""
import sys
import os
import json
import argparse

# Use third party ordereddict for older versions of Python
try:
    from collections import OrderedDict
except ImportError:
    from ordereddict import OrderedDict

from boto.endpoints import BotoEndpointResolver
from boto.endpoints import StaticEndpointBuilder


EXISTING_ENDPOINTS_FILE = os.path.join(
    os.path.dirname(os.path.dirname(os.path.abspath(__file__))),
    'boto', 'endpoints.json')


SUPPORTED_SERVICES = [
    'autoscaling', 'cloudformation', 'cloudfront', 'cloudhsm', 'cloudsearch',
    'cloudsearchdomain', 'cloudtrail', 'codedeploy', 'cognito-identity',
    'cognito-sync', 'config', 'datapipeline', 'directconnect', 'dynamodb',
    'ec2', 'ecs', 'elasticache', 'elasticbeanstalk', 'elasticloadbalancing',
    'elasticmapreduce', 'elastictranscoder', 'email', 'glacier', 'iam',
    'importexport', 'kinesis', 'kms', 'lambda', 'logs', 'machinelearning',
    'monitoring', 'opsworks', 'rds', 'redshift', 'route53', 'route53domains',
    's3', 'sdb', 'sns', 'sqs', 'storagegateway', 'sts', 'support', 'swf'
]


def load_endpoints(endpoints_file):
    if endpoints_file is not None:
        with open(endpoints_file) as f:
            return json.load(f, object_pairs_hook=OrderedDict)

    try:
        import botocore
    except ImportError:
        print("Could not import botocore, make sure it's installed or "
              "provide an endpoints file in order to regen endpoint data.")
        sys.exit(1)

    botocore_dir = os.path.dirname(botocore.__file__)
    endpoints_file = os.path.join(botocore_dir, 'data', 'endpoints.json')
    with open(endpoints_file) as f:
        return json.load(f, object_pairs_hook=OrderedDict)


def filter_services(endpoints):
    for partition in endpoints['partitions']:
        services = list(partition['services'].keys())
        for service in services:
            if service not in SUPPORTED_SERVICES:
                del partition['services'][service]


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument(
        '--endpoints-file',
        help=('Path to endpoints.json.  If this argument is not given, '
              'then the endpoints.json file bundled with botocore will be '
              'used.'))

    # Since the built in endpoints are no longer in the legacy format,
    # overwrite should not be allowed with legacy format output.
    exclusive = parser.add_mutually_exclusive_group()
    exclusive.add_argument(
        '--overwrite', action='store_true',
        help=('Overwrite the endpoints file built into boto2. This is not '
              'compatible with the legacy format.'))
    exclusive.add_argument(
        '--legacy-format', action='store_true',
        help=('Generate the endpoints in the legacy format, suitable for use '
              'as custom endpoints.'))
    args = parser.parse_args()
    endpoints_data = load_endpoints(args.endpoints_file)
    filter_services(endpoints_data)

    if args.legacy_format:
        builder = StaticEndpointBuilder(BotoEndpointResolver(endpoints_data))
        endpoints_data = builder.build_static_endpoints()

    json_data = json.dumps(
        endpoints_data, indent=2, separators=(',', ': '))

    if args.overwrite:
        with open(EXISTING_ENDPOINTS_FILE, 'w') as f:
            f.write(json_data)
            f.write('\n')
    else:
        print(json_data)


if __name__ == '__main__':
    main()
