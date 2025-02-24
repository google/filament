.. ses_tut:

=============================
Simple Email Service Tutorial
=============================

This tutorial focuses on the boto interface to AWS' `Simple Email Service (SES) <ses>`_.
This tutorial assumes that you have boto already downloaded and installed.

.. _SES: http://aws.amazon.com/ses/

Creating a Connection
---------------------

The first step in accessing SES is to create a connection to the service.
To do so, the most straight forward way is the following::

    >>> import boto.ses
    >>> conn = boto.ses.connect_to_region(
            'us-west-2',
            aws_access_key_id='<YOUR_AWS_KEY_ID>',
            aws_secret_access_key='<YOUR_AWS_SECRET_KEY>')
    >>> conn
    SESConnection:email.us-west-2.amazonaws.com

Bear in mind that if you have your credentials in boto config in your home
directory, the two keyword arguments in the call above are not needed. More
details on configuration can be found in :doc:`boto_config_tut`.

The :py:func:`boto.ses.connect_to_region` functions returns a
:py:class:`boto.ses.connection.SESConnection` instance, which is the boto API
for working with SES.

Notes on Sending
----------------

It is important to keep in mind that while emails appear to come "from" the
address that you specify via Reply-To, the sending is done through Amazon.
Some clients do pick up on this disparity, and leave a note on emails.

Verifying a Sender Email Address
--------------------------------

Before you can send email "from" an address, you must prove that you have
access to the account. When you send a validation request, an email is sent
to the address with a link in it. Clicking on the link validates the address
and adds it to your SES account. Here's how to send the validation email::

    >>> conn.verify_email_address('some@address.com')
    {
        'VerifyEmailAddressResponse': {
            'ResponseMetadata': {
                'RequestId': '4a974fd5-56c2-11e1-ad4c-c1f08c91d554'
            }
        }
    }

After a short amount of time, you'll find an email with the validation
link inside. Click it, and this address may be used to send emails.

Listing Verified Addresses
--------------------------

If you'd like to list the addresses that are currently verified on your
SES account, use
:py:meth:`list_verified_email_addresses <boto.ses.connection.SESConnection.list_verified_email_addresses>`::

    >>> conn.list_verified_email_addresses()
    {
        'ListVerifiedEmailAddressesResponse': {
            'ListVerifiedEmailAddressesResult': {
                'VerifiedEmailAddresses': [
                    'some@address.com',
                    'another@address.com'
                ]
            },
            'ResponseMetadata': {
                'RequestId': '2ab45c18-56c3-11e1-be66-ffd2a4549d70'
            }
        }
    }

Deleting a Verified Address
---------------------------

In the event that you'd like to remove an email address from your account,
use
:py:meth:`delete_verified_email_address <boto.ses.connection.SESConnection.delete_verified_email_address>`::

    >>> conn.delete_verified_email_address('another@address.com')

Sending an Email
----------------

Sending an email is done via
:py:meth:`send_email <boto.ses.connection.SESConnection.send_email>`::

    >>> conn.send_email(
            'some@address.com',
            'Your subject',
            'Body here',
            ['recipient-address-1@gmail.com'])
    {
        'SendEmailResponse': {
            'ResponseMetadata': {
                'RequestId': '4743c2b7-56c3-11e1-bccd-c99bd68002fd'
            },
            'SendEmailResult': {
                'MessageId': '000001357a177192-7b894025-147a-4705-8455-7c880b0c8270-000000'
            }
        }
    }

If you're wanting to send a multipart MIME email, see the reference for
:py:meth:`send_raw_email <boto.ses.connection.SESConnection.send_raw_email>`,
which is a bit more of a low-level alternative.

Checking your Send Quota
------------------------

Staying within your quota is critical, since the upper limit is a hard cap.
Once you have hit your quota, no further email may be sent until enough
time elapses to where your 24 hour email count (rolling continuously) is
within acceptable ranges. Use
:py:meth:`get_send_quota <boto.ses.connection.SESConnection.get_send_quota>`::

    >>> conn.get_send_quota()
    {
        'GetSendQuotaResponse': {
            'GetSendQuotaResult': {
                'Max24HourSend': '100000.0',
                'SentLast24Hours': '181.0',
                'MaxSendRate': '28.0'
            },
            'ResponseMetadata': {
                'RequestId': u'8a629245-56c4-11e1-9c53-9d5f4d2cc8d3'
            }
        }
    }

Checking your Send Statistics
-----------------------------

In order to fight spammers and ensure quality mail is being sent from SES,
Amazon tracks bounces, rejections, and complaints. This is done via
:py:meth:`get_send_statistics <boto.ses.connection.SESConnection.get_send_statistics>`.
Please be warned that the output is extremely verbose, to the point
where we'll just show a short excerpt here::

    >>> conn.get_send_statistics()
    {
        'GetSendStatisticsResponse': {
            'GetSendStatisticsResult': {
                'SendDataPoints': [
                    {
                        'Complaints': '0',
                        'Timestamp': '2012-02-13T05:02:00Z',
                        'DeliveryAttempts': '8',
                        'Bounces': '0',
                        'Rejects': '0'
                    },
                    {
                        'Complaints': '0',
                        'Timestamp': '2012-02-13T05:17:00Z',
                        'DeliveryAttempts': '12',
                        'Bounces': '0',
                        'Rejects': '0'
                    }
                ]
            }
        }
    }
