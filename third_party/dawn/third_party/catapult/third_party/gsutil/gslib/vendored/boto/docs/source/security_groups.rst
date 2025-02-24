.. _security_groups:

===================
EC2 Security Groups
===================

Amazon defines a security group as:

"A security group is a named collection of access rules.  These access rules
 specify which ingress, i.e. incoming, network traffic should be delivered
 to your instance."

To get a listing of all currently defined security groups::

    >>> rs = conn.get_all_security_groups()
    >>> print rs
    [SecurityGroup:appserver, SecurityGroup:default, SecurityGroup:vnc, SecurityGroup:webserver]

Each security group can have an arbitrary number of rules which represent
different network ports which are being enabled.  To find the rules for a
particular security group, use the rules attribute::

    >>> sg = rs[1]
    >>> sg.name
    u'default'
    >>> sg.rules
    [IPPermissions:tcp(0-65535),
     IPPermissions:udp(0-65535),
     IPPermissions:icmp(-1--1),
     IPPermissions:tcp(22-22),
     IPPermissions:tcp(80-80)]

In addition to listing the available security groups you can also create
a new security group.  I'll follow through the "Three Tier Web Service"
example included in the EC2 Developer's Guide for an example of how to
create security groups and add rules to them.

First, let's create a group for our Apache web servers that allows HTTP
access to the world::

    >>> web = conn.create_security_group('apache', 'Our Apache Group')
    >>> web
    SecurityGroup:apache
    >>> web.authorize('tcp', 80, 80, '0.0.0.0/0')
    True

The first argument is the ip protocol which can be one of; tcp, udp or icmp.
The second argument is the FromPort or the beginning port in the range, the
third argument is the ToPort or the ending port in the range and the last
argument is the CIDR IP range to authorize access to.

Next we create another group for the app servers::

    >>> app = conn.create_security_group('appserver', 'The application tier')

We then want to grant access between the web server group and the app
server group.  So, rather than specifying an IP address as we did in the
last example, this time we will specify another SecurityGroup object.:

    >>> app.authorize(src_group=web)
    True

Now, to verify that the web group now has access to the app servers, we want to
temporarily allow SSH access to the web servers from our computer.  Let's
say that our IP address is 192.168.1.130 as it is in the EC2 Developer
Guide.  To enable that access::

    >>> web.authorize(ip_protocol='tcp', from_port=22, to_port=22, cidr_ip='192.168.1.130/32')
    True

Now that this access is authorized, we could ssh into an instance running in
the web group and then try to telnet to specific ports on servers in the
appserver group, as shown in the EC2 Developer's Guide.  When this testing is
complete, we would want to revoke SSH access to the web server group, like this::

    >>> web.rules
    [IPPermissions:tcp(80-80),
     IPPermissions:tcp(22-22)]
    >>> web.revoke('tcp', 22, 22, cidr_ip='192.168.1.130/32')
    True
    >>> web.rules
    [IPPermissions:tcp(80-80)]